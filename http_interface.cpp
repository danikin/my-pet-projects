/*
*    http_interace.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the http interface of the taxi service
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

namespace http_interface
{

using namespace httplib;
using json = nlohmann::json;

void http_server::run(int port,
                      auth::authenticator &ath,
                  geo::compact_addresses &addr2,
			    marketplace::marketplace_api &mrktplc,
		marketplace::marketplace_api_riders &riders,
                      marketplace::marketplace_api_drivers &drivers)
{
    // TODO: take care about thread synchronization!!!
    
    Server svr;
    
    // Handle auth requests
    http_server_handle_auth(svr, ath, mrktplc);
    
    // Handle geo requests
    http_server_handle_geo(svr, addr2);
    

    // Sends user's feedback. The response comes through websocket
    //
    // Request: /marketplace/feedback?token=qcGzbiUHb1uN4zlDtpN2&lots_of_info :-)
    // Response:
    svr.Get("/misc/feedback", [&](const Request& req, Response& res) {
        json result;
        
        // TODO: implement
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

    // Turns push notifications on or off
    //
    // Request: /misc/turn_on_off_push?token=qcGzbiUHb1uN4zlDtpN2&on=true :-)
    // Response:
    svr.Get("/misc/turn_on_off_push", [&](const Request& req, Response& res) {
        json result;
        
        // TODO: implement
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

    // Returns info on cars: makers, models, colors
    //
    // Note: it works in form of suggestions
    //
    // Request: /misc/cars?what=maker|model|color&s=т
    // Response: ["Tesla", "Toyota"]
    svr.Get("/misc/cars", [&](const Request& req, Response& res) {
        json result;
        
        // TODO: implement
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

    // Sends a chat message to the other party
    //
    // Request: /misc/chat_other_party?token=blablabla&message=123
    // Response:
    svr.Get("/misc/chat_other_party", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;
        
        json result;
        if (!mrktplc.chat_other_party(auth_result.u_.rider_id_, auth_result.u_.driver_id_,
                                                       req.get_param_value("message")))
            result = {"event", "chat_other_party_error"};
        else
            result = {"event", "chat_other_party_ok"};
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

    // Returns the chat history with other party
    //
    // Note: it works in form of suggestions
    //
    // Request: /misc/chat_history?token=blablabla
    // Response:
    svr.Get("/misc/chat_history", [&](const Request& req, Response& res)
    {
        // This method requires authentication
        auto auth_result = http_interface::check_token(svr, ath, req, res);
        if (!auth_result.is_ok())
            return;
        
        json result;
        std::vector<marketplace::assigned_order::chat_entry> history;
        if (!mrktplc.get_chat_history(auth_result.u_.rider_id_, auth_result.u_.driver_id_, history))
            result = {"event", "chat_history_error"};
        else
        {
            result = {"event", "chat_other_party_ok"};
            for (auto &e : history)
                result["data"].push_back({"chat_entry", {"r_message", e.r_message_}, {"d_message", e.d_message_}});
        }
        
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });
    // Handle rider requests
    http_server_handle_rider(svr, ath, mrktplc, riders);
    
    // Handle driver requests
    http_server_handle_driver(svr, ath, mrktplc, drivers);
    
    // Handle internal requests
    http_server_handle_internal(svr, ath, mrktplc, riders, drivers);

    // This way we will listen any port
    svr.listen(NULL, port, AI_PASSIVE);
}

}

