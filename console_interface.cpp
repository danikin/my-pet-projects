/*
*    console_interace.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the console interface of the taxi service
*
*/

#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"

#include "marketplace_api.h"
#include "ws_interface_external.h"

#include "console_interface.h"


namespace console_interface
{

using json = nlohmann::json;

void debug_marketplaces()
{
    // Read the next JSON from input
    std::string str;
    while (getline(std::cin, str))
    {
        json j = json::parse(str);
        marketplace::marketplace m(false, false, false, false, false);
         
        // Upload marketplace from JSON
        ws_interface::json_2_marketplace(m, j);
                
        // Rebalance it
        m.force_rebalance(time(NULL));

        // Get notifications from marketplace
        std::string json_result;
        ws_interface::marketplace_notifications_2_json(m, json_result);
          
        std::cout << json_result;
        std::cout.flush();
                
        //printf("\nmarketplace notification result: %s\n", json_result.c_str());
        //fflush(stdout);
         
        // Dump internals of the marketplace
        std::unordered_map<long long, marketplace::order> price_views;
        std::unordered_map<long long, marketplace::order> unassigned_orders;
        std::unordered_map<long long, marketplace::driver> unassigned_drivers;

        m.get_price_views(price_views);
        m.get_unassigned_orders(unassigned_orders);
        m.get_unassigned_drivers(unassigned_drivers);
                
        std::cout << std::endl << "price_views:(" << price_views.size() << ")" << std::endl;
        for (auto pv_i : price_views)
        {
		auto &pv = pv_i.second;
            json res;
            res["price_view"] = {};
            res["price_view"]["ID"] = pv.id_;
                    res["price_view"]["ClientID"] = pv.rider_id_;
                    res["price_view"]["Latitude"] = pv.A_.latitude;
                    res["price_view"]["Longitude"] = pv.A_.longitude;
                    res["price_view"]["DelLatitude"] = pv.B_.latitude;
                    res["price_view"]["DelLongitude"] = pv.B_.longitude;
                    
                    std::cout << res << std::endl;
               }

                std::cout << std::endl << "unassigned_orders:(" << unassigned_orders.size() << ")" << std::endl;
                for (auto o_i : unassigned_orders)
                {
			auto &o = o_i.second;
                    json res;
                    res["unassigned_order"] = {};
                    res["unassigned_order"]["ID"] = o.id_;
                    res["unassigned_order"]["ClientID"] = o.rider_id_;
                    res["unassigned_order"]["Latitude"] = (float)o.A_.latitude;
                    res["unassigned_order"]["Longitude"] = (float)o.A_.longitude;
                    res["unassigned_order"]["DelLatitude"] = (float)o.B_.latitude;
                    res["unassigned_order"]["DelLongitude"] = (float)o.B_.longitude;
                    res["unassigned_order"]["price"] = (int)o.price_;
                    res["unassigned_order"]["desire"] = o.desirability_;

                    std::cout << res << std::endl;
                }

                std::cout << std::endl << "unassigned_drivers:(" << unassigned_drivers.size() << ")" << std::endl;
                for (auto d_i : unassigned_drivers)
                {
                    auto &d = d_i.second;
                    json res;
                    res["unassigned_driver"] = {};
                    res["unassigned_driver"]["id"] = d.id_;
                    res["unassigned_driver"]["location"]["Lat"] = (float)d.position_where_.latitude;
                    res["unassigned_driver"]["location"]["Lon"] = (float)d.position_where_.longitude;
                    res["unassigned_driver"]["top_order"] = d.top_order_.id_;
                    res["unassigned_driver"]["top_order_desire"] = d.top_order_desirability_;
                    res["unassigned_driver"]["top_order_metric"] = (float)d.top_order_metric_value_;

                    std::cout << res << std::endl;
                }
    }
}


    //marketplace::marketplace_api mkpl("mysqlx://root:De3695511@127.0.0.1");
    
/*    marketplace::marketplace impl;

    impl.turn_rebalance_off();
    marketplace::json_2_marketplace(impl, R"FOOBAR(
    
    {
                                    
    "event":"drivers","data":
                                    
    {
                                    
    "drivers":
                                    
    [
                                    
    {"id":"14521066","location":{"Lon":50.8003203,"Lat":61.6727887},"timestamp":1596029499,"direction":0,"speed":0,"dist":645.62,"info":{"phone":"9042308942"},"car_info":{"id":3110578,"class_id":14},"robot":3},
     
     {"id":"14763938","location":{"Lon":50.812174,"Lat":61.6831056},"timestamp":1596029464,"direction":0,"speed":0.28,"dist":755.32,"info":{"phone":"9121407232"},"car_info":{"id":3245214,"class_id":14},"robot":3},
     
     
     {"id":"14880350","location":{"Lon":50.8258733,"Lat":61.6710007},"timestamp":1596029496,"direction":170,"speed":11.87,"dist":1034.99,"info":{"phone":"9042703010"},"car_info":{"id":3321014,"class_id":2},"robot":3}
                                    
    ],
                                    
     "orders":
        
    [
             
    {"DriverID":15830263,"ID":1099272423,"ClientID":77569791,"Latitude":61.660843,"Longitude":50.847755,"DelLatitude":"61.646601","DelLongitude":"50.799019","CollDate":"2020-07-29 17:28:00","OrderedDate":"2020-07-29 17:21:39","Status":"OW","OrderComment":"","OrderHash":"af9ccb9bd408fa8a6d22ac2fee3d3fe4","Filter":{"NoCashPayment":"N","NoSmoking":"","Baby":"N","Conditioner":0,"OnlinePayment":1,"CardPayment":0},"Tariff":4318},
                  
                  
    {"DriverID":15020778,"ID":1099272703,"ClientID":61422502,"Latitude":61.66103,"Longitude":50.82064,"DelLatitude":"61.64107","DelLongitude":"50.806395","CollDate":"2020-07-29 17:30:00","OrderedDate":"2020-07-29 17:21:43","Status":"OW","OrderComment":"2 подъезд","OrderHash":"6a0f223f87a34c3bab82bc024b9dbc0b","Filter":{"NoCashPayment":"N","NoSmoking":"","Baby":"N","Conditioner":0,"OnlinePayment":1,"CardPayment":0},"Tariff":4318},
             
             
    {"DriverID":17647111,"ID":1099276923,"ClientID":76136891,"Latitude":61.676716,"Longitude":50.813663,"DelLatitude":"61.689628","DelLongitude":"50.81267","CollDate":"2020-07-29 17:28:00","OrderedDate":"2020-07-29 17:22:57","Status":"OW","OrderComment":"","OrderHash":"15e4265453821079efe6827c28f82e57","Filter":{"NoCashPayment":"N","NoSmoking":"","Baby":"N","Conditioner":0,"OnlinePayment":8,"CardPayment":0},"Tariff":4318}
                  
                  
    ]
                                    
    }
                                    
    }
                                    
                                    
                                    
                                    
                                    
                                    
                                    {
                                                                     
                                     "event":"drivers","data":
                                                                     
                                     {
                                                                     
                                     "drivers":
                                                                     
                                     [
                                                                     
                                     {"id":"14521066","location":{"Lon":50.8003203,"Lat":61.6727887},"timestamp":1596029499,"direction":0,"speed":0,"dist":645.62,"info":{"phone":"9042308942"},"car_info":{"id":3110578,"class_id":14},"robot":3},
                                      
                                      {"id":"14763938","location":{"Lon":50.812174,"Lat":61.6831056},"timestamp":1596029464,"direction":0,"speed":0.28,"dist":755.32,"info":{"phone":"9121407232"},"car_info":{"id":3245214,"class_id":14},"robot":3},
                                      
                                      
                                      {"id":"14880350","location":{"Lon":50.8258733,"Lat":61.6710007},"timestamp":1596029496,"direction":170,"speed":11.87,"dist":1034.99,"info":{"phone":"9042703010"},"car_info":{"id":3321014,"class_id":2},"robot":3}
                                                                     
                                     ],
                                                                     
                                      "orders":
                                         
                                     [
                                              
                                     {"DriverID":15830263,"ID":1099272423,"ClientID":77569791,"Latitude":61.660843,"Longitude":50.847755,"DelLatitude":"61.646601","DelLongitude":"50.799019","CollDate":"2020-07-29 17:28:00","OrderedDate":"2020-07-29 17:21:39","Status":"OW","OrderComment":"","OrderHash":"af9ccb9bd408fa8a6d22ac2fee3d3fe4","Filter":{"NoCashPayment":"N","NoSmoking":"","Baby":"N","Conditioner":0,"OnlinePayment":1,"CardPayment":0},"Tariff":4318},
                                                   
                                                   
                                     {"DriverID":15020778,"ID":1099272703,"ClientID":61422502,"Latitude":61.66103,"Longitude":50.82064,"DelLatitude":"61.64107","DelLongitude":"50.806395","CollDate":"2020-07-29 17:30:00","OrderedDate":"2020-07-29 17:21:43","Status":"OW","OrderComment":"2 подъезд","OrderHash":"6a0f223f87a34c3bab82bc024b9dbc0b","Filter":{"NoCashPayment":"N","NoSmoking":"","Baby":"N","Conditioner":0,"OnlinePayment":1,"CardPayment":0},"Tariff":4318},
                                              
                                              
                                     {"DriverID":17647111,"ID":1099276923,"ClientID":76136891,"Latitude":61.676716,"Longitude":50.813663,"DelLatitude":"61.689628","DelLongitude":"50.81267","CollDate":"2020-07-29 17:28:00","OrderedDate":"2020-07-29 17:22:57","Status":"OW","OrderComment":"","OrderHash":"15e4265453821079efe6827c28f82e57","Filter":{"NoCashPayment":"N","NoSmoking":"","Baby":"N","Conditioner":0,"OnlinePayment":8,"CardPayment":0},"Tariff":4318}
                                                   
                                                   
                                     ]
                                                                     
                                     }
                                                                     
                                     }

                                    )FOOBAR");
    impl.rebalance(time(NULL));
 
 
   std::string json_result;
    marketplace_notifications_2_json(impl, json_result);
  
    printf("marketplace notification result: %s\n", json_result.c_str());
    fflush(stdout);
 
    */



}
