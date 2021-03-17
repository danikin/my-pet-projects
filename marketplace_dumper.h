/*
*    marketplace_dumper.h
*
*    (C) Denis Anikin 2020
*
*    Dumper for marketplace headers
*
*/

#ifndef _marketplace_dumper_h_included_
#define _marketplace_dumper_h_included_

#include <iostream>

#include "nlohmann/json.hpp"

#include "marketplace.h"

namespace dumper
{

using json = nlohmann::json;

// Dumps marketplace to json
//void to_json(marketplace::marketplace &m, json &result);

// Sends a marketplace to the output stream as one json message
// per item
// One per item - because react native apps have limitation on one json message
// from a websocket (I've not found this in docs but in fact if a message is longer than
// 15800 bytes then it's cut and JSON gets broken)
void send_marketplace_message_by_message(marketplace::marketplace &m, std::ostream &os);

}

#endif
