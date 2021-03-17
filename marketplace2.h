/*
*	marketplace2.h
*
*	(C) Denis Anikin 2020
*
*	Headers for marketplace 2n version
*
*/

#ifndef _marketplace_2_h_included_
#define _marketplace_2_h_included_

#include <list>
#include <time.h>
#include <sys/time.h>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <iostream>

#include "nlohmann/json.hpp"

#include "geolocation.h"
#include "driver_billing.h"
#include "object_track.h"

#include "marketplace.h"
#include "cost_db.h"
#include "profiler.h"

namespace marketplace
{

extern profiler g_profiler_;

typedef unsigned long long object_id;

using namespace geo;
using json = nlohmann::json;

struct driver2;

struct rider2
{
    // Params from the user
    object_id rider_id_ = 0;
    point A_ = {.longitude = 1000000, .latitude = 1000000} ,B_ = {.longitude = 1000000, .latitude = 1000000};
    rider_side_filters rsf_;
    
    // Price for the order confirmed with a rider when the order was placed or
    // when the order was repriced
    // Note: if it's zero then that means that this is a price view
    float price_as_placed_ = 0;
    
    // Market price for an order (it does not matter if it's been placed or not yet)
    // Calculated as follows:
    //  1.  Take all unassined drivers that pass filters with this order
    //  2.  Take the top order profit margin for each (or zero if there is no top order for a driver)
    //          Note: profit margin of an order is based on costs incurred by the order and revenue
    //              from its price_as_placed_
    //          Note: only consider orders with price_as_placed_ non zero - aka placed orders
    //          Note: if an order is placed and suggested to be repriced but a rider has not yet confimed
    //              that, then all profit margins for drivers are still based on its current price_as_placed_
    //  3.  Add 1% to that profit margin and determine the price for this order for this driver with with
    //           profit margin
    //  4.  Among all prices from 3 take the least one - that will be the market price for this order
    float market_price_ = 1000000000;
    object_id best_driver_id_ = 0;

    // Assigned part
    object_id assigned_driver_id_ = 0;
};

// Temp and debug data for rebalance
struct rebalance_temp_and_debug_data
{
    std::vector<H3Index> temp_neighbours_buffer_;
    std::vector<H3Index> temp_compacted_buffer_;
    // Beware of nested calls with use of rebalance_temp_and_debug_data::temp_nearby_objects_!
    std::vector<object_id> temp_nearby_objects_;
    
    // This is to debug a problem of considering much too remote drivers
    std::vector<int> debug_clusters_of_objects_;
    float debug_max_distance_considered_driver_to_A_ = 0;
    object_id debug_max_distance_rider_id_ = 0;
    object_id debug_max_distance_driver_id_ = 0;
    int debug_iterations1_ = 0;
    
    void dump(std::ostream &os);
};

struct driver2
{
    // Params - from the user
    object_id driver_id_;
    point where_ = {.longitude = 1000000, .latitude = 1000000};
    driver_side_filters dsf_;
    
    // Best profit magrin in percent (or zero)
    float best_profit_margin_;
    // ID of the best rider in terms of profit margin (or 0)
    object_id best_rider_id_ = 0;
    
    // When the best order is recently formed to this driver
    // If it's zero then it means that there is no best order at the moment
    time_t ts_best_order_appeared_ = 0;

    // Assigned part
    object_id assigned_rider_id_ = 0;
};

// Call the specified function and outputs how long it ran
template <class Func>
void profile(Func f, const char *name, bool debug_print)
{
    
    struct timeval tv;
    unsigned long long sec1, sec2;
    
    if (debug_print)
    {
        gettimeofday(&tv, NULL);
        sec1 = tv.tv_sec * 1000000 + tv.tv_usec;
    }
    
    f();

    if (debug_print)
    {
        gettimeofday(&tv, NULL);
        sec2 = tv.tv_sec * 1000000 + tv.tv_usec;
        std::cerr << name << " has taken " << (sec2 - sec1)/1000 << " miliseconds" << std::endl;
    }
}

template <class T>
void unique(T &t)
{
    std::sort(t.begin(), t.end());
    t.erase(std::unique(t.begin(), t.end()), t.end());
}

class marketplace2
{
public:

    // Make it from 3 because of compaction which can (and will!) result in
    // a bigger resolution
    marketplace2() :
        driver_location_db_(3,7), rider_location_db_(3,7), debug_verbosity_(0) {}
    
    // Rebalances the marketplace
    // The result is:
    //  1.  Each driver has actual best_order_id_ and best_profit_margin_
    //  2.  Each rider has actual best_driver_id_ and market_price_
    // TODO: if or when decide to uncomment it - make sure that you know what to do
    //  with changed_riders_ and changed_drivers_
    //void rebalance(rebalance_temp_and_debug_data &rtdd);

    // Adjusts driver's best order and its profit margin based on this order
    // Note: we need "calc_extra_cost" to adjust the profit margin for THIS order
    //  if it's taken by THIS driver
    // Note: "oc" is the calculated part of "rider"
    // Note: "rttd" is for debuggging and temp storage of data
    void adjust_driver_best_order(rider2 &rider, driver2 &driver, float total_cost);
    
    // Ajusts order's market price based on driver's best profit margin
    // Precondition: driver.best_profit_margin_ is ALREADY filled with actual data based
    //  on placed prices of ALL orders around
    // Note: we need "calc_extra_cost" to calculate the better price for THIS order
    //  to beat the one that's ALREADY on top for THIS driver
    // Note: "oc" is the calculated part of "rider"
    // Note: "calc_extra_cost" is precalculated extra cost for
    void adjust_oder_market_price(rider2 &rider, driver2 &driver, float total_cost);
    
    // Rebalances the marketplace as if a new order appears or changes
    // Note: under the hood it
    //  a) calculates order's market price based on best orders of drivers around
    //  b) adjusts the best order and the best profit margin for ALL drivera around
    void rebalance_for_order(rider2 &rider, rebalance_temp_and_debug_data &rtdd);

    // Rebalances the marketplace as if a new driver appears or changes
    // Note: under the hood it
    //  a) calculates drivers's best order and best profit margin based on placed orders around
    //  b) adjusts the market price for ALL orders around
    void rebalance_for_driver(driver2 &driver, rebalance_temp_and_debug_data &rtdd);
    
    // Rebalances the marketplace as if an order disappears (cancelled or assigned)
    // Precondition: order is ALREADY assigned or unplaced - otherwise it can and WILL
    //  be put on top again
    // Note: under the hodd it
    //  a) iterates all drivers around
    //  b) rebalances for each driver who had this order on top
    void rebalance_for_disappeared_order(rider2 &rider, rebalance_temp_and_debug_data &rtdd);

    // Rebalances the marketplace as if a driver disappears (assigned)
    // Note: under the hodd it
    //  a) iterates all orders around
    //  b) rebalances for each order who had this driver the best for the market price
    void rebalance_for_disappeared_driver(driver2 &driver, rebalance_temp_and_debug_data &rtdd);
    
    // 1. Creates a rider if it does not exist
    // 2. If the order has not been placed yet then updates A and B
    // 3. Additionally if placed_order_price is not zero then place the order with that price
    //  Note: a rider can take the price by one of the previous received updates with a market price
    //          or can set any price they like
    //  Note: if an order has already been placed then the price can be only changed up and no
    //      more than the market place for that order
    // Returns the pointer to the rider if it was found by rider_id
    rider2 *udpate_order(object_id rider_id, point A, point B,
                      float placed_order_price,
                      bool place_order_at_market_price);
    

    // Updates driver's position
    driver2 *update_driver_position(object_id driver_id, point where);
    
    // Just assign an order
    bool assign_order(object_id rider_id, object_id driver_id);
    
    // Rebalances after order assignment
    void rebalance_after_assignment(object_id rider_id, object_id driver_id);

    // Unassignes current order of the driver (and removes it is "remove" is true)
    // Note: if the driver does not have an assigned order then the function does nothing
    // Note: the order can be unassigned in the following moments:
    //  1.  By a driver after toll wait starts
    //          if that happens before the ride began then the order is considered cancelled and
    //              is turned into unassigned stage
    //          if that happens after the ride began then the order is considered finished and
    //              is removed
    //  2.  By a rider before driver arrived
    //          if that happens then the order is removed
    // Returns the driver for this order or NULL if there is an error
    driver2 *unassign_order(object_id rider_id, bool remove);

    // Rebalances after order unassignment
    // Note: rider_id can be 0 - this means that the order has been removed
    void rebalance_after_unassignment(object_id rider_id, object_id driver_id);

    void set_debug_verbosity(int level) { debug_verbosity_ = level; }
    
    driver2 *get_driver_by_id(object_id driver_id);
    rider2 *get_rider_by_id(object_id rider_id);
    rider2 *get_non_assigned_rider_by_id(object_id rider_id);
    driver2 *get_non_assigned_driver_by_id(object_id driver_id);
    rider2 *get_assigned_rider_by_id(object_id rider_id);
    driver2 *get_assigned_driver_by_id(object_id driver_id);

    // Reads a JSON request and write a broadcast response
    bool process_json_request(json &j, std::ostream &os);

private:
    
    // We cap the max distance to the pickup point with 20km
    const int max_distance_toA_ = 20;

    // Global cost info
    cost_db cost_db_;
    
    // For quick search of drivers near a pickup point
    global_position_db driver_location_db_;
    
    // For quick search of riders near a driver
    global_position_db rider_location_db_;
    
    // All riders
    std::unordered_map<object_id, rider2> riders_;
    // All drivers
    std::unordered_map<object_id, driver2> drivers_;
    
    // Store driver tracks to determine real distances of trips
    object_tracks driver_tracks_;
    
    // Lists of ids of changes riders and drivers
    std::vector<object_id> changed_riders_, changed_drivers_, temp_changed_drivers_, temp_changed_riders_;

    int debug_verbosity_;
    
    time_t now_ = 0;
    int now_weekday_;
    int now_minutes_;

}; // class marketplace2


} // namespace marketplace

#endif
