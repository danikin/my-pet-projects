/*
*       marketplace_api_riders.cpp
*
*       (C) Denis Anikin 2020
*
*       Api impl for riders in marketplace
*
*/

#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

//#include <mysqlx/xdevapi.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

//using namespace ::mysqlx;

#include "fifo_interface.h"
#include "marketplace_api_riders.h"

namespace marketplace
{

long long marketplace_api_riders::reg_rider()
{
    // Save rider data in RAM
    rider_info rinfo;

    rider_db_.push_back(rinfo);
    
    // TODO: save in DB

    return rider_db_.size() - 1;
}

// Dumps driver database to json
void marketplace_api_riders::to_json(nlohmann::json &result)
{
    result["event"] = "dump_riders";
    int i = 0;
    for (auto &d : rider_db_)
    {
        result["data"].push_back({
            {"rider_id", i}
        });
        ++i;
    }
}

} // namespace marketplace
