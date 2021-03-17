/*
*    http_interace_geo.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the geo part of http interface of the taxi service
*
*/

#include <iostream>
#include <fstream>
#include <algorithm>

#include "httplib.h"

#include "nlohmann/json.hpp"

#include "http_interface.h"
#include "marketplace_api.h"
#include "geo.h"
#include "auth.h"

namespace http_interface
{

using namespace httplib;
using json = nlohmann::json;

// Renders a geo point for the output
json render_point(const geo::compact_addresses &addr2,
                  unsigned long long house_and_street_id,
                  float distance)
{
    const geo::compact_house *ch = addr2.get_house_by_big_id(house_and_street_id);
    const geo::compact_street *cs = addr2.get_street_by_big_id(house_and_street_id);
    const std::string *poi_name = (ch?addr2.get_poi_by_id(ch->poi_id_):NULL);
    const std::string *house_number_name = (ch?addr2.get_house_number_by_house_number_id(ch->number_id_):NULL);
    const std::string *street_name =addr2.get_street_name_by_big_id(house_and_street_id);
    
    json j = json::object();
    
    j["point"] = {
        {"address", (street_name?*street_name:"") + " " + (house_number_name?*house_number_name:"")},
        {"poi_name", poi_name ? *poi_name : ""},
        {"house_latitude", (ch?ch->lat_:0)},
        {"house_longitude", (ch?ch->lon_:0)},
        {"distance", distance},
        
        {"house_and_street_id", house_and_street_id},
        {"street_id", addr2.get_street_id_by_big_id(house_and_street_id)},
        {"house_id", addr2.get_house_id_by_big_id(house_and_street_id)},
        {"house_number_id", (ch?ch->number_id_:-1)},
        {"poi_id", (ch?ch->poi_id_:-1)},
        {"street_name", (cs?cs->name_:"")},
        {"house_number_name", (house_number_name?*house_number_name:"")}
    };
    
    return j;
}

json render_street(const geo::compact_addresses &addr2,
                    unsigned long just_street_id,
                    float distance)
{
    const geo::compact_street *cs = addr2.get_street_by_street_id(just_street_id);

    json j = json::object();
    
    j["street"] = {
        {"street_name", (cs?cs->name_:"")},
        {"distance", distance},
        {"street_id", just_street_id}
    };
    
    return j;
}

void http_server_handle_geo(Server &svr, const geo::compact_addresses &addr2)
{
    // Takes a beginning of a street name and returns suggestions in a form of street names and ids
    // It also supports POI, but without street id (TODO: we still need positions of POI)
    //
    // Request: /geo/find_street?s=улица
    // Response: [[4179979,"улица Печорская"],[4179979,"улица Печорская"],[-1,"12 стульев мебельный салон, , улица Печорская 66"]
    svr.Get("/geo/find_street", [&](const Request& req, Response& res) {
        json result = {{"event" , "find_street"}};
        auto ac = addr2.auto_complete_street(req.get_param_value("s"));
        int i = 0;
        for (const auto &a : ac)
        {
            result["data"][i++] = a;
        }
        if (!i)
            result["data"] = json::array();
        
        // Note: hacks with event and empty data are needed for the react native app to work for some reason
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

    // Takes a beginning a FULL ADDRESS and returns suggestions in a form of full addresses (including house
    // numbers), their coordinates and pickup points if there are any
    //
    // Also the house can be searched by its poi - for this case the flag "poi" should be used
    //
    // Request: /geo/find_house?poi=0&s=улица ленина 2
    // Response: [{"full_address":"улица Красная Гора 2","house_latitude":61.6744231807478,"house_longitude":50.8868152958738,"road_latitude":0.0,"road_longitude":0.0}]
    svr.Get("/geo/find_house", [&](const Request& req, Response& res)
    {
        bool poi = req.get_param_value("poi").empty() ? false : std::stoi(req.get_param_value("poi"));
        
        int i = 0;
        
        
        json result = {{"event" , "find_house"}};
        
        
        // If we search for pois then use a different auto complete and show the poi name in the result
        if (poi)
        {
            auto ac = addr2.auto_complete_poi(req.get_param_value("s"));
            for (const auto &a : ac)
            {
                const std::string *poi_name = addr2.get_poi_by_id(a.first.poi_id_);
                const std::string *house_number_name = addr2.get_house_number_by_house_number_id(a.first.number_id_);
                
                result["data"][i]["poi_name"] = poi_name ? *poi_name : "";
                result["data"][i]["full_address"] = a.second + " " + (house_number_name?*house_number_name:"");
                result["data"][i]["house_latitude"] = a.first.lat_;
                result["data"][i]["house_longitude"] = a.first.lon_;
               // result["data"][i]["road_latitude"] = a.first.nearest_road_position_.latitude;
               // result["data"][i]["road_longitude"] = a.first.nearest_road_position_.longitude;

                ++i;
            }
        }
        else
        {
            std::string street;
            auto ac = addr2.auto_complete_house(req.get_param_value("s"), street);
            for (const auto &a : ac)
            {
                unsigned long poi_id = a.poi_id_;
                const std::string *poi_name = addr2.get_poi_by_id(poi_id);
                const std::string *house_number_name = addr2.get_house_number_by_house_number_id(a.number_id_);
                
                result["data"][i]["full_address"] = street + " " + (house_number_name?*house_number_name:"");
                result["data"][i]["house_latitude"] = a.lat_;//a.house_position_.latitude;
                result["data"][i]["house_longitude"] = a.lon_;//a.house_position_.longitude;
                result["data"][i]["poi_name"] = poi_name ? *poi_name : "";
               // result["data"][i]["road_latitude"] = a.nearest_road_position_.latitude;
               // result["data"][i]["road_longitude"] = a.nearest_road_position_.longitude;

                ++i;
            }
        }
        
        res.set_content(result.dump(1), "application/json; charset=utf-8");
    });
    
    // Returns houses within specified distance to the specified position
    // Request /geo/get_houses_by_location?lat=61.6744231807478&lon=50.8868152958738&resolution=11&ring_size=2
    svr.Get("/geo/get_houses_by_location", [&](const Request& req, Response& res)
    {
        json result;
        geo::point location = {.longitude = std::stof(req.get_param_value("lon")),
            .latitude = std::stof(req.get_param_value("lat"))
        };
        
        geo::compact_house nearest_house;
        std::string nearest_street_name;
        auto houses = addr2.get_houses_near_location(
                                               location,
                                               std::stoi(req.get_param_value("resolution")),
                                               std::stoi(req.get_param_value("ring_size")),
                                                     &nearest_house, &nearest_street_name
                                               );
        
        result["event"] = "get_houses_by_location";
        
        if (!nearest_street_name.empty())
        {
            const std::string *house_number_name = addr2.get_house_number_by_house_number_id(nearest_house.number_id_);
            
            result["data"]["nearest_house"] = nearest_street_name + " " + (house_number_name?*house_number_name:"");
        }
        
        int i = 0;
        for (const auto &a : houses)
        {
            const std::string *house_number_name = addr2.get_house_number_by_house_number_id(a.first.number_id_);
            
            result["data"]["houses"][i]["full_address"] = a.second + " " + (house_number_name?*house_number_name:"");
            result["data"]["houses"][i]["house_latitude"] = a.first.lat_;
            result["data"]["houses"][i]["house_longitude"] = a.first.lon_;
            //result["data"]["houses"][i]["road_latitude"] = a.nearest_road_position_.latitude;
            //result["data"]["houses"][i]["road_longitude"] = a.nearest_road_position_.longitude;
            result["data"]["houses"][i]["shortest_distance"] =
            geo::shortest_distance_km(location, {.longitude = a.first.lon_, .latitude = a.first.lat_});
            result["data"]["houses"][i]["manhattan_distance"] =
                geo::manhattan_distance_km(location, {.longitude = a.first.lon_, .latitude = a.first.lat_});
            ++i;
        }
        res.set_content(result.dump(1), "application/json; charset=utf-8");
    });
    
    // Returns the best street or POI match for this pattern and near this location
    // Request /geo/best_stret_match?s=адм лаз&lat=61.6744231807478&lon=50.8868152958738&pois=3&streets=3&mix=1
    svr.Get("/geo/best_street_match", [&](const Request& req, Response& res)
    {
        json result;
        result["event"] = "best_street_match";
        geo::point location = {.longitude = std::stof(req.get_param_value("lon")),
            .latitude = std::stof(req.get_param_value("lat"))
        };
        
        int poi_num = req.get_param_value("pois").empty() ? 3 : std::stoi(req.get_param_value("pois"));
        int street_num = req.get_param_value("streets").empty() ? 3 : std::stoi(req.get_param_value("streets"));
        int mix = req.get_param_value("mix").empty() ? 0 : std::stoi(req.get_param_value("mix"));
        const std::string &search_string = req.get_param_value("s");

        // If there is no search pattern then it's just reverse geocoding
        if (search_string.empty())
        {
            std::vector<std::pair<unsigned long long, float> > best;
            addr2.best_match(location, best);
            for (auto &b : best)
                result["data"]["mix"].push_back(render_point(addr2, b.first, b.second));
        }
        else
        {
        // Find best POIs and streets to match search words and the location
        std::vector<std::pair<unsigned long long, float> > best_poi(poi_num);
        addr2.best_poi_match(search_string, location, best_poi);
        std::vector<std::pair<unsigned long, float> > best_streets(street_num);
        addr2.best_street_name_match(search_string,location, best_streets);
        
        // Also search for a house
        std::vector<std::pair<unsigned long long, float> > best_houses(0);
        addr2.best_house_match(search_string, location, best_houses);
        
        if (mix)
        {
            // Sort and mix result altogether
            std::vector<std::pair<bool, std::pair<unsigned long long, float>>> mix_result;
            for (auto &b : best_poi)
                mix_result.push_back({true, b});
            for (auto &b : best_streets)
                mix_result.push_back({false, b});
            for (auto &b : best_houses)
                 mix_result.push_back({true, b});

            std::sort(mix_result.begin(), mix_result.end(),
                      [](const typeof(mix_result[0]) &a, const typeof(mix_result[0]) &b)
                      {
                return a.second.second < b.second.second;
            });
            
            // Render it (no more than mix records)
            for (auto &b : mix_result)
            {
                if (b.first)
                    result["data"]["mix"].push_back(render_point(addr2, b.second.first, b.second.second));
                else
                    result["data"]["mix"].push_back(render_street(addr2, b.second.first, b.second.second));
                if (!--mix)
                    break;
            }
        }
        else
        {
            // Render result separately
            for (auto &b : best_poi)
                result["data"]["pois"].push_back(render_point(addr2, b.first, b.second));
            for (auto &b : best_streets)
                result["data"]["streets"].push_back(render_street(addr2, b.first, b.second));
        }
        }
        
        res.set_content(result.dump(1), "application/json; charset=utf-8");
    });
}

}
