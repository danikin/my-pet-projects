#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>

#include "marketplace2.h"

/*
*	marketplace2.cpp
*
*	(C) Denis Anikin 2020
*
*	Impl for marketplace 2nd version
*
*/

namespace marketplace
{

rider2 *marketplace2::get_non_assigned_rider_by_id(object_id rider_id)
{
    auto *p_rider = get_rider_by_id(rider_id);
    if (!p_rider)
    {
        std::cerr << "marketplace2::get_non_assigned_rider_by_id: ERROR: bad rider_id! " << rider_id << std::endl;
        return NULL;
    }
    if (p_rider->assigned_driver_id_)
    {
        std::cerr << "marketplace2::get_non_assigned_rider_by_id: ERROR: rider " << rider_id
            << " is assigned to " << p_rider->assigned_driver_id_ << std::endl;
        return NULL;
    }
    if (rider_id != p_rider->rider_id_)
    {
        std::cerr << "marketplace2::get_non_assigned_rider_by_id: ERROR! RIDER IDS MISMATCH: rider_id=" << rider_id << ", rider.rider_id_=" << p_rider->rider_id_ << std::endl;
        return NULL;
    }
    return p_rider;
}

driver2 *marketplace2::get_non_assigned_driver_by_id(object_id driver_id)
{
    auto *p_driver = get_driver_by_id(driver_id);
    if (!p_driver)
    {
        std::cerr << "marketplace2::get_non_assigned_driver_by_id: ERROR: bad driver_id! " << driver_id << std::endl;
        return NULL;
    }
    if (p_driver->assigned_rider_id_)
    {
        std::cerr << "marketplace2::get_non_assigned_driver_by_id: ERROR: driver " << driver_id
            << " is assigned to " << p_driver->assigned_rider_id_ << std::endl;
        return NULL;
    }
    if (driver_id != p_driver->driver_id_)
    {
        std::cerr << "marketplace2::get_non_assigned_driver_by_id : ERROR! DRIVER IDS MISMATCH: driver_id=" << driver_id << ", driver.driver_id_=" << p_driver->driver_id_ << std::endl;
        return NULL;
    }
    return p_driver;
}

rider2 *marketplace2::get_assigned_rider_by_id(object_id rider_id)
{
    auto *p_rider = get_rider_by_id(rider_id);
    if (!p_rider)
    {
        std::cerr << "marketplace2::get_assigned_rider_by_id: ERROR: bad rider_id! " << rider_id << std::endl;
        return NULL;
    }
    if (!p_rider->assigned_driver_id_)
    {
        std::cerr << "marketplace2::get_assigned_rider_by_id: ERROR: rider " << rider_id
            << " is NOT assigned" << std::endl;
        return NULL;
    }
    if (rider_id != p_rider->rider_id_)
    {
        std::cerr << "marketplace2::get_assigned_rider_by_id: ERROR! RIDER IDS MISMATCH: rider_id=" << rider_id << ", rider.rider_id_=" << p_rider->rider_id_ << std::endl;
        return NULL;
    }
    return p_rider;
}

driver2 *marketplace2::get_assigned_driver_by_id(object_id driver_id)
{
    auto *p_driver = get_driver_by_id(driver_id);
    if (!p_driver)
    {
        std::cerr << "marketplace2::get_assigned_driver_by_id: ERROR: bad driver_id! " << driver_id << std::endl;
        return NULL;
    }
    if (!p_driver->assigned_rider_id_)
    {
        std::cerr << "marketplace2::get_assigned_driver_by_id: ERROR: driver " << driver_id
            << " is NOT assigned" << std::endl;
        return NULL;
    }
    if (driver_id != p_driver->driver_id_)
    {
        std::cerr << "marketplace2::get_assigned_driver_by_id : ERROR! DRIVER IDS MISMATCH: driver_id=" << driver_id << ", driver.driver_id_=" << p_driver->driver_id_ << std::endl;
        return NULL;
    }
    return p_driver;
}

driver2 *marketplace2::get_driver_by_id(object_id driver_id)
{
    auto i = drivers_.find(driver_id);
    if (i == drivers_.end())
        return NULL;
    else
        return &i->second;
}

rider2 *marketplace2::get_rider_by_id(object_id rider_id)
{
    auto i = riders_.find(rider_id);
    if (i == riders_.end())
        return NULL;
    else
        return &i->second;
}

rider2 *marketplace2::udpate_order(object_id rider_id, point A, point B,
                                    float placed_order_price,
                                    bool place_order_at_market_price)
{
    if (!rider_id)
        return NULL;
    
    auto &rider = riders_[rider_id];
    rider.rider_id_ = rider_id;

    // Update rider's pickup point in a location DB - for quick search of riders nearby
    rider_location_db_.update_object_position(rider_id,
                                               (rider.A_.latitude==1000000)?NULL:&rider.A_,
                                               &A,
                                               debug_verbosity_ >= 1);
    rider.A_ = A;
    rider.B_ = B;
    

    // If the order is being placed and no best driver then don't place it
    // Note: this is when no drivers near a rider compatible with filters
    // Note: this also can (and WILL) happen if a rider appears out of the blue
    //  with a new ID with placed order (no price view - straight to the order)
    if ((placed_order_price || place_order_at_market_price)
        && !rider.best_driver_id_)
    {
        std::cerr << "marketplace2::udpate_order: CAN'T PLACE ORDER - NO DRIVERS AROUND, rider_id="
            << rider_id << ", rider.market_price_=" << rider.market_price_ << std::endl;
        return &rider;
    }
    
    if (place_order_at_market_price)
        rider.price_as_placed_ = rider.market_price_;
    else
    // Note: I removed the check on placed price not to be more than market price because
    // the market price can be changed since then and it's OK that a rider placed an order for
    // a bigger price - if a rider is OK with that price then let's take it :-)
    if (placed_order_price > rider.price_as_placed_/* && placed_order_price <= rider.market_price_*/)
        rider.price_as_placed_ = placed_order_price;
    
    return &rider;
}

driver2 *marketplace2::update_driver_position(object_id driver_id, point where)
{
    if (!driver_id)
        NULL;
    
    auto &driver = drivers_[driver_id];
    driver.driver_id_ = driver_id;

    // Update driver's current position in a location DB - for quick search of drivers nearby
    driver_location_db_.update_object_position(driver_id,
                                               (driver.where_.latitude==1000000)?NULL:&driver.where_,
                                               &where,
                                               debug_verbosity_ >= 1);

    driver.where_ = where;
    
    // Update driver's current position in the track
    driver_tracks_.update_object_position(driver_id, driver.where_, now_);
    
    return &driver;
}

bool marketplace2::assign_order(object_id rider_id, object_id driver_id)
{
    if (!rider_id || !driver_id)
        return false;
    
    // Check that both rider and driver exist - we don't want to create them
    //  in this request
    auto *driver = get_driver_by_id(driver_id);
    if (!driver)
        return false;
    auto *rider = get_rider_by_id(rider_id);
    if (!rider)
        return false;
    
    // Assign a rider and a driver to each other is they're not assigned yet
    if (driver->assigned_rider_id_)
        return false;
    if (rider->assigned_driver_id_)
        return false;
    driver->assigned_rider_id_ = rider_id;
    rider->assigned_driver_id_ = driver_id;

    // Unset this ts to set it back when the driver has the best order
    driver->ts_best_order_appeared_ = 0;
    
    // Remove the driver from geolocation db to optimize loops for nearby drivers
    driver_location_db_.update_object_position(driver_id,
                                               &driver->where_,
                                               NULL,
                                               debug_verbosity_ >= 1);

    // Remove the rider from geolocation db to optimize loops for nearby riders
    rider_location_db_.update_object_position(rider_id,
                                               &rider->A_,
                                               NULL,
                                               debug_verbosity_ >= 1);
    
    return true;
}

driver2 *marketplace2::unassign_order(object_id rider_id, bool remove)
{
    auto *rider = get_assigned_rider_by_id(rider_id);
    if (!rider)
        return NULL;
    auto *driver = get_assigned_driver_by_id(rider->assigned_driver_id_);
    if (!driver)
        return NULL;
    
    // Reset everything for a driver just like she/he is ready for the next order
    driver->assigned_rider_id_ = 0;
    driver->best_rider_id_ = 0;
    driver->ts_best_order_appeared_ = now_;
    
    if (remove)
        riders_.erase(rider_id);
    else
    {
        // Reset everything for a rider just like it's another unassigned order
        rider->assigned_driver_id_ = 0;
        rider->best_driver_id_ = 0;
    }
    
    return driver;
}

} // namespace marketplace
