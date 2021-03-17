
/*
*    ws_interace_test.h
*
*    (C) Denis Anikin 2020
*
*    Headers for the test ws interface of the taxi service
*
*/

#ifndef _ws_interace_test_h_included_
#define _ws_interace_test_h_included_

#include "nlohmann/json.hpp"

#include "marketplace.h"

namespace ws_interface
{

using json = nlohmann::json;

bool ws_interface_test_handler(std::ostream &output_stream_end_client,
                               marketplace::marketplace &test_marketplace,
                               json &j);

}

#endif

