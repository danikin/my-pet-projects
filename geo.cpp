/*
*    geo.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation of geo services
*
*/

#include <iostream>
#include <fstream>
#include <unistd.h>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "geo.h"
#include "trie.h"

namespace geo
{
/*
void unescape_html_entities(std::string &str)
{
    size_t in_pos = 0;
    size_t out_pos = 0;
    while (in_pos < str.size())
    {
        size_t new_pos = str.find("&#", in_pos);
        if (new_pos == std::string::npos || new_pos + 3 > str.size())
        {
            out_pos += str.size() - in_pos;
            break;
        }
        else
        {
            if (str[new_pos+1] == '3' && str[new_pos+1] == '4')
        }
    }
    
    str.resize(out_pos);
}*/

void parse_geo_string(const std::string &input_string,
                      std::string &node_id,
                      std::string &poi,
                      std::string &street,
                      std::string &house,
                      std::string &lat,
                      std::string &lon)
{
    const char divider[] = "@@@^$$$";
    const size_t sizeof_divider = sizeof(divider)-1;
    
    size_t pos0 = input_string.find(divider);
    if (pos0 == std::string::npos)
    {
        std::cerr << "BAD INPUT STRING (no node_id): " << input_string << std::endl;
    }
    node_id = std::string(input_string, 0, pos0);
     
    size_t pos1 = input_string.find(divider, pos0 + sizeof_divider);
    if (pos1 == std::string::npos)
    {
        std::cerr << "BAD INPUT STRING (no poi): " << input_string << std::endl;
    }
    poi = std::string(input_string, pos0 + sizeof_divider, pos1 - (pos0+sizeof_divider));
     
    size_t pos2 = input_string.find(divider, pos1 + sizeof_divider);
    if (pos2 == std::string::npos)
    {
        std::cerr << "BAD INPUT STRING (no street): " << input_string << std::endl;
    }
    street = std::string(input_string, pos1 + sizeof_divider, pos2 - (pos1+sizeof_divider));
     
    size_t pos3 = input_string.find(divider, pos2 + sizeof_divider);
    if (pos3 == std::string::npos)
    {
        std::cerr << "BAD INPUT STRING (no house): " << input_string << std::endl;
    }
    house = std::string(input_string, pos2 + sizeof_divider, pos3 - (pos2+sizeof_divider));
     
    size_t pos4 = input_string.find(divider, pos3 + sizeof_divider);
    if (pos4 == std::string::npos)
    {
        std::cerr << "BAD INPUT STRING (no lat): " << input_string << std::endl;
    }
    lat = std::string(input_string, pos3 + sizeof_divider, pos4 - (pos3+sizeof_divider));
    lon = std::string(input_string, pos4 + sizeof_divider);
}

std::string files_with_geo_info[] = {
    
    // All Russia
    "./central-fed-district-latest.osm.parsed6",
    "./north-caucasus-fed-district-latest.osm.parsed6",
    "./northwestern-fed-district-latest.osm.parsed6",
    "./south-fed-district-latest.osm.parsed6",
    "./ural-fed-district-latest.osm.parsed6",
    "./volga-fed-district-latest.osm.parsed6",
    "./siberian-fed-district-latest.osm.parsed6",
    "./crimean-fed-district-latest.osm.parsed6",
    "./kaliningrad-latest.osm.parsed6",
    

    // Target countries
    "./botswana-latest.osm.parsed6",
    "./senegal-and-gambia-latest.osm.parsed6",
    "./belarus-latest.osm.parsed6",
    "./zimbabwe-latest.osm.parsed6",
    "./uganda-latest.osm.parsed6",
    "./papua-new-guinea-latest.osm.parsed6",
    "./seychelles-latest.osm.parsed6",
    "./nigeria-latest.osm.parsed6",
    "./kenya-latest.osm.parsed6",
    "./solomon-islands-latest.osm.parsed6",
    "./south-africa-latest.osm.parsed6",
    "./laos-latest.osm.parsed6"
    
    /*
    "./cyprus-latest.osm.parsed4",
    "./iran-latest.osm.parsed4",
    "./iraq-latest.osm.parsed4",
    "./kosovo-latest.osm.parsed4",
    "./luxembourg-latest.osm.parsed4",
    "./moldova-latest.osm.parsed4",
    "./montenegro-latest.osm.parsed4",
    "./portugal-latest.osm.parsed4",
    "./serbia-latest.osm.parsed4",
    "./slovenia-latest.osm.parsed4",
    "./syria-latest.osm.parsed4",
    "./yemen-latest.osm.parsed4",*/

    /*,
    
    // Just for fun :-)
    "./north-korea-latest.osm.parsed4",
    "./florida-latest.osm.parsed4",
    "./hawaii-latest.osm.parsed4",
    "./new-york-latest.osm.parsed4",
    "./north-carolina-latest.osm.parsed4"*/
};

void upload_everything2(compact_addresses &addr)
{
    std::string input_string, node_id, poi, street, house, lat, lon;
    
    int no_lat_lon_strings = 0;
    
    {
        std::set<std::string> pois;
        
        // 0. Upload all POIs, unique them by EXACT name
        // and form their ids via references to compact_pois_
        std::cerr << "STAGE ZERO: upload poi names" << std::endl << std::endl;
        int total_pois = 0;
        for (auto &file_name : files_with_geo_info)
        {
            std::ifstream is(file_name);
            if (is.fail() || is.bad())
            {
                std::cerr << "CAN'T OPEN FILE " << file_name << std::endl;
                continue;
            }
            std::cerr << "START FILE: " << file_name << std::endl;
            while (getline(is, input_string))
            {
                parse_geo_string(input_string, node_id, poi, street, house, lat, lon);
                if (!poi.empty())
                {
                    pois.insert(poi);
                    ++total_pois;
                }
            }
        }
        
        addr.upload_pois_once(pois);
        
        std::cerr << std::endl << "STAGE ZERO DONE: total poi names: " << total_pois << ", unique poinames: "
            << pois.size() << std::endl;
    }
    
    // From now and on POIs can be referenced via their ids
    
    // 1. Upload all streets and houses
    {
        std::set<std::string> street_names, house_numbers;
        std::cerr << std::endl << "STAGE ONE: upload street names and house names" << std::endl << std::endl;
        for (auto &file_name : files_with_geo_info)
        {
            std::ifstream is(file_name);
            if (is.fail() || is.bad())
            {
                std::cerr << "CAN'T OPEN FILE " << file_name << std::endl;
                continue;
            }
            std::cerr << "START FILE: " << file_name << std::endl;
            while (getline(is, input_string))
            {
                parse_geo_string(input_string, node_id, poi, street, house, lat, lon);
                
                if (street_names.find(street) == street_names.end())
                    street_names.insert(street);
                
                if (house_numbers.find(house) == house_numbers.end())
                    house_numbers.insert(house);
            }
        }

        addr.upload_streets_once(street_names);
        addr.upload_house_numbers_once(house_numbers);
        
        std::cerr  << std::endl << "STAGE ONE DONE: total streets: " << street_names.size() << ", total house numbers: "
            << house_numbers.size() << std::endl;
    }

    // 2. Upload all houses
    std::cerr << std::endl << "STAGE TWO: upload houses" << std::endl << std::endl;
    for (auto &file_name : files_with_geo_info)
    {
        std::ifstream is(file_name);
        if (is.fail() || is.bad())
        {
            std::cerr << "CAN'T OPEN FILE " << file_name << std::endl;
            continue;
        }
        std::cerr << "START FILE: " << file_name << std::endl;
        while (getline(is, input_string))
        {
            parse_geo_string(input_string, node_id, poi, street, house, lat, lon);
            
            double d_lat, d_lon;
            try {
                d_lat = std::stod(lat);
                d_lon = std::stod(lon);
            } catch (std::exception&) {
                ++no_lat_lon_strings;
                //std::cerr << "BAD INPUT STRING: can't convert lat or lon: '" << input_string << "'"
                //    << "lat='" << lat << ", lon='" << lon << "'" << std::endl;
                //throw;
                continue;
            }
            addr.upload_house(poi, street, house, std::stod(lat), std::stod(lon));
        }
    }
    std::cerr << std::endl << "STAGE TWO DONE" << std::endl << std::endl;
    std::cerr << "START STAGE THREE: sort_uploaded_houses(), that's: a)"
        " sort uploade houses, b) fill geo objects db, c) fill temp map for POI search words" << std::endl;
    
    {
        // POI search word -> house_id
        std::map<std::string, std::vector<unsigned long long> > temp_poi_search_word_to_house_id;
    
        // 3. Sort uploaded houses (temp_poi_search_word_to_house_id is filled inside)
        addr.sort_uploaded_houses(temp_poi_search_word_to_house_id);
    
        std::cerr << std::endl << "STAGE THREE DONE" << std::endl << std::endl << "Stat so far:"<< std::endl;
    
        // From now and on houses are sorted, have correct POI ids and totally ready to go
    
        // Print stat so far
        addr.print_stat();
    
    
        std::cerr << std::endl << "START STAGE SIX - sort uploaded pois, no_lat_lon_strings=" <<
        no_lat_lon_strings << std::endl;
    
    //std::cerr << std::endl << "START STAGE SIX - sort uploaded pois, just_all_pois_size=" << just_all_pois_size
      //  << ", unique pois # " << just_all_pois.size() << std::endl;
    
        // 6. Sort uploaded pois (temp_poi_search_word_to_house_id is used inside)
        addr.sort_uploaded_pois(temp_poi_search_word_to_house_id);
    }

    std::cerr << std::endl << "START STAGE SIX DONE" << std::endl;

    // Print stat so far
    addr.print_stat();
    
    /*while (true)
    {
        usleep(1);
    }*/
    
    /*std::cerr << "Dump compacted to disk" << std::endl;
    
    // Dump everything in a compact human readable form for later upload
    std::ofstream dump_here("./geo_compacted_things.txt");
    addr.dump_compacted_things(dump_here);*/
}

} // namespace geo
