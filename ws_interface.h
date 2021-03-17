/*
*    ws_interace.h
*
*    (C) Denis Anikin 2020
*
*    Headers for the websockets interface of the taxi service
*
*/

#ifndef _ws_interface_h_included_
#define _ws_interface_h_included_

#include <map>

#include "auth.h"
#include "marketplace.h"
#include "marketplace2.h"

namespace ws_interface
{

class ws_agent
{
public:
    
    // Note: don't erase riders/drivers with late ccordinates in the test marketplace
    // Note: and no notifications
    ws_agent() : test_marketplace_(true, false, false, false, false) {}

    // Loops to communicate with headquarters and an end client
    // They MUST be run into separate threads
    // They share the state, but the state does not need to be protected because it's filled out once
    // during auth and then is just used to read by agent_vs_client_loop
    void agent_vs_headquarters_loop();
    void agent_vs_client_loop();

private:
    
    // Auth data
    long long user_id_ = -1;
    long long rider_id_ = -1;
    long long driver_id_ = -1;
    std::string token_;
    
    // Test marketplace
    marketplace::marketplace test_marketplace_;

	// New marketplace
	marketplace::marketplace2 mpl2_;
};


void headquarters_loop(auth::authenticator &a,
                       marketplace::marketplace_api &mrktplc,
                       fifo_interface::multicastor &rider_multicastor,
                       fifo_interface::multicastor &driver_multicastor);

}

#endif
