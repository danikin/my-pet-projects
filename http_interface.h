/*
*    http_interace.h
*
*    (C) Denis Anikin 2020
*
*    Headers for the http interface of the taxi service
*
*/

#ifndef _http_interface_h_included_
#define _http_interface_h_included_

#include "httplib.h"

#include "geo_core.h"
#include "auth.h"
#include "marketplace_api.h"
#include "marketplace_api_riders.h"
#include "marketplace_api_drivers.h"

namespace http_interface
{

class http_server
{
public:

    // Runs the server
    // All the logic is inside
    // All the needed databases are passed through arguments to this function
    void run(int port,
             auth::authenticator &ath,
		geo::compact_addresses &addr2,
             marketplace::marketplace_api &mrktplc,
	marketplace::marketplace_api_riders &riders,
             marketplace::marketplace_api_drivers &drivers);
};

// Handlers
void http_server_handle_auth(httplib::Server &srv, auth::authenticator &ath, marketplace::marketplace_api &mrktplc);
void http_server_handle_geo(httplib::Server &srv, const geo::compact_addresses &addr2);
void http_server_handle_rider(httplib::Server &srv, auth::authenticator &ath, marketplace::marketplace_api &mrktplc, marketplace::marketplace_api_riders &riders);
void http_server_handle_driver(httplib::Server &srv, auth::authenticator &ath,
                               marketplace::marketplace_api &mrktplc,
                               marketplace::marketplace_api_drivers &drivers);
void http_server_handle_internal(httplib::Server &svr, auth::authenticator &ath,
                                 marketplace::marketplace_api &mrktplc,
                                marketplace::marketplace_api_riders &riders,
                                 marketplace::marketplace_api_drivers &drivers);

// Checks token and returns user_id if token is ok otherwise returns -1
// As a side effect it forms the response to the http server
auth::authenticator::auth_result check_token(httplib::Server &srv, auth::authenticator &ath, const httplib::Request& req, httplib::Response& res);

}

#endif


