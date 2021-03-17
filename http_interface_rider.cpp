/*
*    http_interace_rider.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the rider part of http interface of the taxi service
*
*/

#include <iostream>
#include <fstream>

#include "httplib.h"

#include "nlohmann/json.hpp"

#include "http_interface.h"
#include "marketplace_api.h"
#include "marketplace_api_riders.h"
#include "geo.h"
#include "auth.h"

namespace http_interface
{

using namespace httplib;
using json = nlohmann::json;

void http_server_handle_rider(Server &svr, auth::authenticator &ath,
                              marketplace::marketplace_api &mrktplc,
                              marketplace::marketplace_api_riders &riders)
{
    /*
     *              Rider's part
     */
    
    // Registers a rider
    //
    // Request: /rider/reg?token=qcGzbiUHb1uN4zlDtpN2
    // Response:
    svr.Get("/rider/reg", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;

        json result;
         
        // Check if a rider has already been registered and associated with this user
        if (auth_result.u_.rider_id_ != -1)
            result = {"event", "rider_reg_error_already_registred"};
        else
        {
            // Register a rider
            long long rider_id = riders.reg_rider();
            if (rider_id == -1)
                result = {"event", "rider_reg_error_cant_register"};
            else
            {
                // If something goes wrong here then it's either a bug or corrupted data
                // which is also probably a bug :-)
                if (!ath.set_rider(auth_result.user_id_, rider_id))
                    result = {"event", "rider_reg_error_set_rider_internal"};
                else
                    // Now a rider is succesfully registred
                    result = {"event", "rider_reg_ok"};
            }
        }
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });
    
    
    /*
    ->  /rider/move_A?flow_id=123&lat=11.11&lon=12.12
    <-  {"event"    :   "move_A_ok"}
    <-  {"event"    :   "move_A_error"}
    */

    svr.Get("/rider/move_A", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;
       
	json result = json::object();
 
        if (mrktplc.update_price_view_A(auth_result.u_.rider_id_,
                                                      std::stoi(req.get_param_value("flow_id")),
                              {.longitude = std::stof(req.get_param_value("lon")), .latitude = std::stof(req.get_param_value("lat"))}))
            result = {"event", "move_A_ok"};
        else
            result = {"event", "move_A_error"};

        res.set_content(result.dump(), "application/json; charset=utf-8");
    });
    
    /*
    ->  /rider/move_B?flow_id=123&lat=11.11&lon=12.12
    <-  {"event"    :   "move_B_ok"}
    <-  {"event"    :   "move_B_error"}
    */
    svr.Get("/rider/move_B", [&](const Request& req, Response& res)
       {
           // This method requires authentication
           auto auth_result = http_interface::check_token(svr, ath, req, res);
           if (!auth_result.is_ok())
               return;
          
       json result = json::object();
    
           if (mrktplc.update_price_view_B(auth_result.u_.rider_id_,
                                                         std::stoi(req.get_param_value("flow_id")),
                                 {.longitude = std::stof(req.get_param_value("lon")), .latitude = std::stof(req.get_param_value("lat"))}))
               result = {"event", "move_B_ok"};
           else
               result = {"event", "move_B_error"};

           res.set_content(result.dump(), "application/json; charset=utf-8");
    });
    
    
    // Requests a price from A to B. The response is just a price view id. The price itself
    // does not come immediately rather it comes via websocket in a form of marketplace notifications
    // about price change. For the app it does not matter if a price for view changes or first time forms :-)
    //
    // Note: one rider through one app with one token can requests lots of price views. And she/he will be
    // notified about price change for each of them until they expire. This is useful to remind a rider
    // that the price for some directions either goes down (which stimulates her/him to place an order) or
    // goes up (which also stimulates her/him to place an order until the situation does not become worse :-) )
    //
    // Request: /marketplace/price_view?token=qcGzbiUHb1uN4zlDtpN2&A_lon=&A_lat=&A_addr=&B_lon=&B_lat=&B_addr=
    // Response: {"pv_id":12345}
    svr.Get("/marketplace/price_view", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;
        
        long long pv_id = mrktplc.price_view(auth_result.u_.rider_id_,
                              {.longitude = std::stof(req.get_param_value("A_lon")), .latitude = std::stof(req.get_param_value("A_lat"))},
                              req.get_param_value("A_addr"),
                              {.longitude = std::stof(req.get_param_value("B_lon")), .latitude = std::stof(req.get_param_value("B_lat"))},
                              req.get_param_value("B_addr"));
        
        // Note: we sent only id of a price view to the user because the price check is
        // done asyncronously by marketplace. The user will be notified through web socket
                              
        json result = {"event", "price_view_response", "data", {"pv_id", pv_id}};
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });
    
    // Places an order. That's the big green button "Resuest a ride" in the app. It returns just success because
    // the order id will be the same as price view id. All subsequent changes to the order will be received through
    // the websocket
    //
    // Note: price_view_id is taken by a rider app from its understanding of what exactly price view an end user
    // expected to be placed
    //
    // Request: /marketplace/place_order?token=qcGzbiUHb1uN4zlDtpN2&price_view_id=12345
    // Response:
    svr.Get("/marketplace/place_order", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;

        json result;

        if (!mrktplc.place_order(auth_result.u_.rider_id_, std::stoi(req.get_param_value("price_view_id"))))
            result = {"event", "place_order_error"};
        else
            result = {"event", "place_order_ok"};
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });
  
    // Increases an order price at rider's will. This should be normally based on the
    // message received through the websocket that the order is too cheap
    // It's up to the rider how much to increase the order price because it's only a recomendation
    // But a rider can only increase the order price, not decrease :-)
    //
    // Note: there should be some server side limit to avoid typos like +30% to the original price
    // Note: drivers who have this order on the top will be notified through the websocket
    // Note: this method does not pass order_id to the server-side because it's about the current order
    // as a user may have only single placed order at every given moment
    //
    // Request: /marketplace/increase_order_price?token=qcGzbiUHb1uN4zlDtpN2&id=12345&new_price=250
    // Response:
    svr.Get("/marketplace/increase_order_price", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;
        
        json result;
        if (!mrktplc.increase_unassigned_order_price(auth_result.u_.rider_id_, std::stof(req.get_param_value("new_price"))))
            result = {"event", "increase_order_price_error"};
        else
            result = {"event", "increase_order_price_ok"};
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });
    
    // Cancels an order either by a rider or by a driver
    // The otehr party receives this info through the websocket
    //
    // Note: cancellation can be done by either party only before the ride begins
    //
    // Request: /marketplace/cancel_order?token=qcGzbiUHb1uN4zlDtpN2&id=12345
    // Response:
    svr.Get("/marketplace/cancel_order", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;
        
        json result;
        if (!mrktplc.cancel_order(auth_result.u_.rider_id_, auth_result.u_.driver_id_))
            result = {"event", "cancel_order_error"};
        else
            result = {"event", "cancel_order_ok"};
        
        // TODO: implement
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });
}

}
