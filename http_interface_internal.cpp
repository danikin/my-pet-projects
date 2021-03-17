/*
*    http_interace_internal.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the internal http interface of the taxi service
*
*/

#include <iostream>
#include <fstream>

#include "httplib.h"

#include "nlohmann/json.hpp"

#include "http_interface.h"
#include "marketplace_api.h"
#include "marketplace_api_riders.h"
#include "marketplace_api_drivers.h"
#include "marketplace_dumper.h"
#include "auth_dumper.h"
#include "geo.h"
#include "auth.h"

namespace http_interface
{

using namespace httplib;
using json = nlohmann::json;

void http_server_handle_internal(httplib::Server &svr, auth::authenticator &ath,
                                 marketplace::marketplace_api &mrktplc,
                                 marketplace::marketplace_api_riders &riders,
				marketplace::marketplace_api_drivers &drivers)
{
    /*
     *      Methods for internal statistics and control
     *      TODO: authentication!!!!!
     */
    
    // Dumps content of all internal databases:
    //  1.  Marketplace
    //  2.  Assigned orders
    //  3.  Users
    //  4.  Drivers
    //  5.  Riders
    //  6.  Driver tracks
    svr.Get("/internal/dump_internals", [&](const Request& req, Response& res)
    {
        std::string content;
        
        try
        {
            int indent = req.get_param_value("is_human_readable").empty() ? -1 : 1;

            json ra1; ath.user_db_to_json(ra1); content += ra1.dump(indent) + "\n";
            json ra2; ath.tokens_to_json(ra2); content += ra2.dump(indent) + "\n";
            json ra3; ath.phones_to_json(ra3); content += ra3.dump(indent) + "\n";
            json ra4; ath.smscodes_to_json(ra4); content += ra4.dump(indent) + "\n";

		// We need to do something with it - because it only works on a message per
		// message basis with streams        
            //json rm1; mrktplc.marketplace_to_json(rm1); content += rm1.dump(indent) + "\n";
 

           json rm2; mrktplc.assigned_orders_to_json(rm2);  content += rm2.dump(indent) + "\n";
            json rm3; mrktplc.assigned_orders_ro_map_to_json(rm3); content += rm3.dump(indent) + "\n";
            json rm4; mrktplc.assigned_orders_do_map_to_json(rm4); content += rm4.dump(indent) + "\n";
            json rm5; mrktplc.assigned_orders_notifications_to_json(rm5); content += rm5.dump(indent) + "\n";
            json rm6; mrktplc.driver_tracks_to_json(rm6); content += rm6.dump(indent) + "\n";
            json rm7; mrktplc.rider_multicastor_to_json(rm7); content += rm7.dump(indent) + "\n";
            json rm8; mrktplc.driver_multicastor_to_json(rm8); content += rm8.dump(indent) + "\n";
        
            json rd1; drivers.to_json(rd1); content += rd1.dump(indent) + "\n";
            json rr1; riders.to_json(rr1); content += rr1.dump(indent) + "\n";
        }
        catch (std::exception &e)
        {
            content += "\nERROR: ";
            content += e.what();
        }
        
        res.set_content(content, "application/json; charset=utf-8");
    });
}

} // namespace http_interface
