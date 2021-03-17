/*
*    http_interace_auth.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the auth part of http interface of the taxi service
*
*/

#include <iostream>
#include <fstream>

#include "httplib.h"

#include "nlohmann/json.hpp"

#include "http_interface.h"
#include "marketplace_api.h"
#include "geo.h"
#include "auth.h"
#include "debug.h"

namespace http_interface
{

using namespace httplib;
using json = nlohmann::json;

void http_server_handle_auth(Server &svr, auth::authenticator &ath, marketplace::marketplace_api &mrktplc)
{
    // TODO: take care about thread synchronization!!!
    
    // Takes user's mobile phone number and send the sms code
    //
    // Request: /auth/sendsms?phone=+71234567890
    // Response:
    svr.Get("/auth/sendsms", [&](const Request& req, Response& res)
    {
        // Send SMS code to the user
        bool is_ok = ath.send_smscode(req.get_param_value("phone"));
        
        json result;
        if (is_ok)
            result = {{"event" , "auth_sms_sent"}};
        else
            result = {{"event" , "auth_sms_not_sent"}};
        ;

        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

    // Takes sms code and converts it into a lifetime token which associated with this phone
    // If this is the first time anybody receives a token for this phone then an entry is create in the user database
    // associated with this phone
    // Returns token and different user ids
    //
    // Note: the server-side stores a type of this token - rider's or driver's - it depends on what kind of
    // an app requested the token. This type is used to restrict rider's app from driver's requests and vice versa
    //
    // Request: /auth/sendsms?phone=+71234567890&smscode=1234
    // Response: {"token":"qcGzbiUHb1uN4zlDtpN2", "user_id":12345, "rider_id":456, "driver_id":-1}
    svr.Get("/auth/by_smscode", [&](const Request& req, Response& res)
    {
        auto auth_result = ath.authenticate_by_smscode(req.get_param_value("phone"), req.get_param_value("smscode"));
 
        long long rider_id, driver_id;
        if (auth_result.is_ok())
        {
            rider_id = auth_result.u_.rider_id_;
            driver_id = auth_result.u_.driver_id_;
        }
        
        // Return rider_id and driver_id to the app
        // Note: the app can use this info to understand where it's the first
        // time driver/app uses the app or not
        json result = json({});
        if (auth_result.is_ok())
        {
            result["event"] = "auth_ok";
            result["data"] =
                    {
                        {"token" , auth_result.token_},
                        {"user_id" , auth_result.user_id_},
                        {"rider_id" , auth_result.u_.rider_id_},
                        {"driver_id" , auth_result.u_.driver_id_},
                        {"phone_number" , auth_result.u_.phone_number_}
                    };
        }
        else
            result = {{"event" , "auth_error"}};

        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

    // Authenticates a user by token
    // Returns token and different user ids
    //
    // Note: the server-side stores a type of this token - rider's or driver's - it depends on what kind of
    // an app requested the token. This type is used to restrict rider's app from driver's requests and vice versa
    //
    // Request: /auth/sendsms?phone=+71234567890&smscode=1234
    // Response: {"token":"qcGzbiUHb1uN4zlDtpN2", "user_id":12345, "rider_id":456, "driver_id":-1}
    svr.Get("/auth/by_token", [&](const Request& req, Response& res)
    {
        auto auth_result = ath.authenticate_by_token(req.get_param_value("token"));
   
        long long rider_id, driver_id;
        if (auth_result.is_ok())
        {
            rider_id = auth_result.u_.rider_id_;
            driver_id = auth_result.u_.driver_id_;
        }
        
        json result;
        if (auth_result.is_ok())
            result = {{"event" , "auth_ok"}, {"data" , {"token" , req.get_param_value("token")}, {"user_id" , auth_result.user_id_}, {"rider_id" , rider_id}, {"driver_id" , driver_id}}};
        else
            result = {{"event", "auth_error"}};
                                  
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });
}

auth::authenticator::auth_result check_token(httplib::Server &srv, auth::authenticator &ath, const Request& req, Response& res)
{
    long long user_id = -1;
    std::string token = req.get_param_value("token");
    auto auth_result = ath.authenticate_by_token(token);
             
    json result;
    if (!auth_result.is_ok())
    {
        result = {{"event" , "auth_error"}};
        res.set_content(result.dump(), "application/json; charset=utf-8");
        std::cerr << debug::debug() << "auth::authenticator::auth_result check_token: failed, token="
            << token << std::endl;
    }

    return auth_result;
}

}
