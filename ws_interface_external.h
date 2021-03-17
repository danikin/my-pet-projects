/*
*    ws_interace_external.h
*
*    (C) Denis Anikin 2020
*
*    Headers for the external ws interface of the taxi service
*
*/

#ifndef _ws_interace_external_included_
#define _ws_interace_external_included_

#include "nlohmann/json.hpp"

#include "marketplace.h"

namespace ws_interface
{

using json = nlohmann::json;

void json_2_marketplace(marketplace::marketplace &impl, json &input);
void marketplace_notifications_2_json(marketplace::marketplace &impl, std::string &json_result);



bool ws_interface_external_handler(std::ostream &output_stream_end_client,
                                   json &j);
}

#endif

