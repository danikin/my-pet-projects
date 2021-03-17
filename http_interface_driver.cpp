/*
*    http_interace_driver.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the driver part of http interface of the taxi service
*
*/

#include <iostream>
#include <fstream>

#include "httplib.h"

#include "nlohmann/json.hpp"

#include "http_interface.h"
#include "marketplace_api.h"
#include "marketplace_api_drivers.h"
#include "geo.h"
#include "auth.h"

namespace http_interface
{

using namespace httplib;
using json = nlohmann::json;

void http_server_handle_driver(Server &svr, auth::authenticator &ath,
                                marketplace::marketplace_api &mrktplc,
                                marketplace::marketplace_api_drivers &drivers)
{
    /*
     *              Driver's part
     */
    
    // Registers a driver. We need from a driver the following info:
    //  1.  Name (to show it a rider)
    //  2.  Photograph (to show it a rider)
    //  3.  Car make, model and color (to show it a rider)
    //  4.  Lisence plate (to show it a rider)
    //
    //  Note: we don't check the info above. Just trust the driver. Why? Well, we want she or he to start working
    //  right away :-)
    //
    // Request: /driver/reg?token=qcGzbiUHb1uN4zlDtpN2&lots_of_data :-)
    // Response:
    svr.Get("/driver/reg", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;

        json result;
         
        // Check if a driver has already been registered and associated with this user
        if (auth_result.u_.driver_id_ != -1)
            result = {"event", "driver_reg_error_already_registred"};
        else
        {
            // Register a driver
            long long driver_id = drivers.reg_driver(
            req.get_param_value("first_name"),
            req.get_param_value("last_name"),
            req.get_param_value("car_make"),
            req.get_param_value("car_model"),
            std::stoi(req.get_param_value("car_year")),
            req.get_param_value("car_color"),
            req.get_param_value("driver_license"),
            req.get_param_value("photo_url")
            );
            if (driver_id == -1)
                result = {"event", "driver_reg_error_cant_register"};
            else
            {
                // If something goes wrong here then it's either a bug or corrupted data
                // which is also probably a bug :-)
                if (!ath.set_driver(auth_result.user_id_, driver_id))
                    result = {"event", "driver_reg_error_set_driver_internal"};
                else
                    // Now a driver is succesfully registred
                    result = {"event", "driver_reg_ok"};
            }
        }
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });
    
    // A driver sees the top order along with all its data that the app receives through websocket
    // She/he can do two actions with the top order: accept and decline
    // Accept means that this order stops being included in the marketplace algos and
    // a system expects the driver to start going to the pickup point imediately
    //
    // Request: /marketplace/accept_order?token=qcGzbiUHb1uN4zlDtpN2&order_id=12345
    // Response:
    svr.Get("/marketplace/accept_order", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;

        // TODO: there is only ONE driver per assigned order. Avoid race conditions!!!
        
        json result;

        if (!mrktplc.accept_order(auth_result.u_.driver_id_, std::stoi(req.get_param_value("order_id"))))
            result = {"event", "accept_order_error"};
        else
            result = {"event", "accept_order_ok"};
         
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

    // A driver sees the top order along with all its data that the app receives through websocket
    // She/he can do two actions with the top order: accept and decline
    // Decline means that THIS driver will not see this order. It works like a blacklist, so at the
    // next rebalance of the marketplace the driver will receive the next best order (will will likely to
    // happen in 1 second :-) )
    //
    // Request: /marketplace/decline_order?token=qcGzbiUHb1uN4zlDtpN2&order_id=12345
    // Response:
    svr.Get("/marketplace/decline_order", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;
        
        json result;
         
        if (!mrktplc.accept_order(auth_result.u_.driver_id_, std::stoi(req.get_param_value("order_id"))))
            result = {"event", "decline_order_error"};
        else
            result = {"event", "decline_order_ok"};
         
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

     // Changes an order status by a driver initiative:
     //
     //  Note: a rider receives each message through the websocket
     //  Note: a message is a number, not text
     //  Note: a driver will receive the notification back which should be ignored for the instance
    // of the app that generated it
     //
     // Request: /driver/order_action?token=qcGzbiUHb1uN4zlDtpN2&status=
     // Response:
     svr.Get("/driver/change_assigned_order_status", [&](const Request& req, Response& res)
     {
         // This method requires authentication
         auto auth_result = http_interface::check_token(svr, ath, req, res);
         if (!auth_result.is_ok())
             return;
    
         json result;
         if (!mrktplc.change_assigned_order_status(auth_result.u_.driver_id_,
                                     (marketplace::assigned_order_status)
                                                   std::stoi(req.get_param_value("status"))))
             result = {"event", "change_assigned_order_status_error"};
         else
             result = {"event", "change_assigned_order_status_ok"};

         res.set_content(result.dump(), "application/json; charset=utf-8");
     });
    
/*    // A message from a driver to a rider after order is accepted. Messages could be from the following list:
    //
    //  1.  I will be there later, please wait a bit more, don't cancel, I'm on the way :-)
    //  2.  Sorry man I canceled a ride
    //  3.  Got to the pickup point
    //  4.  Free waiting is over
    //  5.  On the way to the drop off point
    //  6.  Got to the drop off point, wait for the payment
    //  7.  Payment is done, order is closed
    //
    //  Note: a rider receives each message through the websocket
    //  Note: a message is a number, not text
    //
    // Request: /driver/order_action?token=qcGzbiUHb1uN4zlDtpN2&message=
    // Response:
    svr.Get("/driver/message", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;
   
        json result;
        if (!mrktplc.driver_message(auth_result.u_.driver_id_,
                                    (marketplace::driver_message_type)
                                    std::stoi(req.get_param_value("message"))))
            result = {"event", "decline_order_error"};
        else
            result = {"event", "decline_order_ok"};

        res.set_content(result.dump(), "application/json; charset=utf-8");
    });*/
}

}
