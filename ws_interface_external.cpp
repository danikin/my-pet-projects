/*
*    ws_interace_external.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the external ws interface of the taxi service
*
*/

#include <iostream>

#include "nlohmann/json.hpp"

#include "marketplace.h"
#include "marketplace_api.h"

namespace ws_interface
{

using json = nlohmann::json;

void marketplace_notifications_2_json(marketplace::marketplace &impl, std::string &json_result)
{
    std::list<marketplace::rider_notification> rn;
    std::list<marketplace::driver_notification> dn;
    
    impl.get_notification_lists(rn, dn);

    json_result = "\n\n{\"event\":\"rider_notifications\",  \"data\":[\n";
    
    for (auto r : rn)
    {
        marketplace::add_rider_notification_to_json(r, json_result);
        json_result += ",\n";
    }
    
    json_result += "]}\n\n{\"event\":\"driver_notifications\", \"data\":[\n";
    
    //printf("dn.size()=%d\n", (int)dn.size());
    
    for (auto &d : dn)
    {
        marketplace::add_driver_notification_to_json(d, json_result);
        json_result += ",\n";
    }
    
    json_result += "]}\n";
}

void json_2_marketplace(marketplace::marketplace &impl, json &input)
{
    
    /*
     
    Format:
     
    {"event":"drivers","data":{"drivers":[{"id":"14521066","location":{"Lon":50.8003203,"Lat":61.6727887},"timestamp":1596029499,"direction":0,"speed":0,"dist":645.62,"info":{"phone":"9042308942"},"car_info":{"id":3110578,"class_id":14},"robot":3},
     
     {"id":"14763938","location":{"Lon":50.812174,"Lat":61.6831056},"timestamp":1596029464,"direction":0,"speed":0.28,"dist":755.32,"info":{"phone":"9121407232"},"car_info":{"id":3245214,"class_id":14},"robot":3},
     
     
     {"id":"14880350","location":{"Lon":50.8258733,"Lat":61.6710007},"timestamp":1596029496,"direction":170,"speed":11.87,"dist":1034.99,"info":{"phone":"9042703010"},"car_info":{"id":3321014,"class_id":2},"robot":3},
        ]
    }

    */
  
    // Upload everything from JSON string
    if (input["event"] == "drivers")
    {
        for (auto drv : input["data"]["drivers"])
        {
            // TODO: take only drivers with "robot" == "3"
            
            /*driver d;
            d.id_ = atoll(drv["id"].get<std::string>().c_str());
            d.erased_ = false;
            d.position_where_.latitude = drv["location"]["Lat"];
            d.position_where_.longitude = drv["location"]["Lon"];
            d.cost_per_km_ = 14.0;
            d.metric_type_ = driver::driver_metric_profit_per_second;*/
            
            // It will implicitly create a driver if does not exist
            impl.update_driver_position(atoll(drv["id"].get<std::string>().c_str()),
                                        {.longitude = drv["location"]["Lon"],
                                            .latitude = drv["location"]["Lat"]}, time(NULL));
        }
        
        for (auto ord : input["data"]["orders"])
        {
            // TODO: take only riders with "Status" == "R"
            marketplace::order o;
            o.id_ = ord["ID"];//atoll(ord["ID"].get<std::string>().c_str());
            o.rider_id_ = ord["ClientID"];//atoll(ord["ClientID"].get<std::string>().c_str());
            o.A_.latitude = ord["Latitude"];
            o.A_.longitude = ord["Longitude"];
            o.B_.latitude = atof(ord["DelLatitude"].get<std::string>().c_str());
            o.B_.longitude = atof(ord["DelLongitude"].get<std::string>().c_str());
            o.ETA_ = 0;// TODO
            o.distance_km_ = 0; // TODO - use manhattan distance
            o.price_ = 0;
            o.erased_ = false;
            
            impl.add_unassigned_order(o, time(NULL));
        }
    }
}


bool ws_interface_external_handler(std::ostream &output_stream_end_client,
                                    json &j)
{
     /*
     *      Interface for the quick marketplace test: create from scratch, rebalance, see result
     */

     // This is a test/debug event with external data
     if (j["event"] == "drivers")
     {
         // Upload marketplace from JSON
         marketplace::marketplace m(false, false, false, false, false);
         json_2_marketplace(m, j);
          
         // Rebalance it
         m.force_rebalance(time(NULL));

         // Get notifications from marketplace
         std::string json_result;
         marketplace_notifications_2_json(m, json_result);
    
         output_stream_end_client << json_result << std::endl;
     }
     else
         return false;
    
    return true;
}

}

