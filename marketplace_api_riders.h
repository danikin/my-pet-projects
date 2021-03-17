/*
*       marketplace_api_riders.h
*
*       (C) Denis Anikin 2020
*
*       Headers for api for riders in marketplace
*
*/

#ifndef _marketplace_api_riders_h_
#define _marketplace_api_riders_h_

#include "nlohmann/json.hpp"

#include "fifo_interface.h"
#include "object_track.h"

#include "marketplace.h"
#include "marketplace_assigned_order.h"

namespace marketplace
{

class marketplace_api_riders
{
public:

    struct rider_info
    {
    };
    
    // Registers a rider
    // Returns rider's id on success and -1 otherwise
    long long reg_rider();

    // Dumps driver database to json
    void to_json(nlohmann::json &result);
    
private:
    
    // Driver database
    // Driver's id is the sequential number in this vector
    // Note: don't mind "holes" because drivers are never deleted
    std::vector<rider_info> rider_db_;
};

} // namespace marketplace

#endif
