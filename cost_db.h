/*
*	cost_db.h
*
*	(C) Denis Anikin 2020
*
*	Headers for the cost database - core for tariffs
*
*/

#ifndef _cost_db_h_included_
#define _cost_db_h_included_

#include <string>
#include <vector>
#include <unordered_map>
#include <ostream>

#include <h3/h3api.h>

#include "geolocation.h"

namespace marketplace
{

// Local cost structure (basically a fare)
struct fare
{
    std::string fare_name_;
    
    // Costs directly contributing to motion
    
    //  The per km part can be incurred by:
    //  - gas price
    //  - gas consumption
    //  - car maintenence (oil service, any other other per km service, expendable materials and supplies)
    //  - per km part of car price amortization (or per km part of car rental or lease)
    //  - car wash
    //  - washer liquid
    // Note: It is NOT REASONABLE AT ALL for a driver to accept an order for the price that doesn't cover AT LEAST
    //  variable cost - that is cost_per_km_
    float cost_per_km_;

    // The per minute part is normally an opportunity costs that's basically a
    //  profit margin of a ride on top of per km cost
    // Example: cost per km if 5 RUB, expected contribution margin is 67%,
    //  average speed if 30km/h (0.5 km/minute)
    // With that a driver expectes that she/he receives 15 RUB per km revenue -> 10 RUB per km contribution margin
    //  For getting this profit a driver will drive 1km or 2 minutes, so that will result in 5 RUB per minute
    //  Which means that if a driver stands still for 1 minutes then she/he loses 5 RUB
    // Note: It can BE REASONABLE for a driver to accept an order with the contribution margin less than
    //  the one that will cover opportunity_cost_per_minute_ - this is when a driver DOES NOT HAVE an
    //  opportunity to sell her/his minutes better (no orders around, no other job right now)
    float opportunity_cost_per_minute_;
    
    // Average speed in this location in km/min
    // How is that related to the cost? This is how: distance / speed is time spent on trip, which
    //  bear the opportunity cost
    float avg_speed_;
    
    // This is on average real distance divided by manhattan distance - helps to better predict cost per km
    float ratio_manhattan_distance_to_real_distance_;

    // Vector of pair - distance and coefficient that reflects the risk of order cancellation
    //   by a rider if the pickup distance is more than that
    // Example vector: [{1.0, 1.25}, {2.0, 1.5}, {3.0, 2.0}]
    //  In that example if a distance is less than 1km then the cancellation risk is neglegible
    //  If a distance is 1-2km then the cancellation risk entails +25% to the cost of a pickup
    //  If a distance is 2-3km then the cancellation risk entails +50% to the cost of a pickup
    //  If a distance is over 3km then the cancellation risk entails +100% to the cost of a pickup
    // Example vector: [{0.0, 1.1}, {3.0, 2.0}]
    //  In that example if a distance is less than 3km then the cancellation risk entails +10% to the cost of a pickup
    //  If a distance is over 3km then the cancellation risk entails +100% to the cost of a pickup
    std::vector<std::pair<float, float> > order_cancel_risk_mitigation_;

    // A real distance since which a driver has to return back to the position she/he
    //  starts - which entails the same cost as going to A or from A to B per km and per minute
    // It's normally 50-100km - because it's defenitely a trip of of the current location that
    //  a driver got used to work in
    float distance_to_pay_way_back_;
    
    // Hotspots (like city centers etc) that are exception from the rule
    //  for cost on the way back
    // Note: if a driver finishes a trip in the city center then they can continue working there
    //  if a driver finishes a trip in a suburb then they have to return back to city - which
    //  entails a cost
    // Note: this vector is sorted for a quick search
    // Note: we use relolutions 4,5,6 here (22km, 8km , 3km)
    std::vector<H3Index> hotspots_;
    
    // If a driver has not been taking the best order for a while then we
    //  include that waiting time as an opportunity cost into the order's cost
    // That time is multiplied by this coefficient to take the local specifics into account
    // Note: this coefficient can be less than 1 or even zero
    float not_taking_best_order_coefficient_;
    
    // The minimal_cost_ cost for the whole trip - includes first km/minutes
    float minimal_cost_;
    
    // Additional cost - it's added to every trip - does not include first km/minutes
    float additional_cost_;

    // Maximum duration of a free waiting at A
    // Note: waiting above a free duration will be paid at opportunity cost
    int free_wait_duration_;
    
    // Fare time frames - interval of times within a day when a fare works
    //  expressed in minutes from 00:00
    // Note: the vector is sorted by second - that's the number of minutes since the beginning
    //  of the day when the fare ends
    std::vector<std::pair<int, int> > fare_time_frames_;

    // Which weekdays this fare works
    bool weekdays_[7];

    inline static bool comp_fare_time_frames_(const std::pair<int,int> &a, int b)
    {
        return a.second < b;
    }
};

// All components of a cost for debug reasons
struct cost_components
{
    int fare_id_;
    int seconds_since_best_order_appeared_;
    bool is_driver_on_way_home_;
    
    float distance_to_A_;
    float duration_to_A_;
    float nominal_cost_to_A_;
    float cancellation_risk_;
    float cost_to_A_;
    
    float distance_A_to_B_;
    float duration_A_to_B_;
    float cost_A_to_B_;
    
    float distance_B_to_back_;
    float duration_B_to_back_;
    float is_B_a_hotspot_;
    float cost_wayback_;
  
    float cost_non_taking_order_;
    float total_cost_;
};

std::ostream &operator<<(std::ostream &os, const cost_components &cc);
    
// Database of costs
class cost_db
{
public:

    // Initializes the cost databases uploading it from the DB
    cost_db();
    
    // Determines a cost of a trip: where -> A -> B
    // Returns true if everything is OK
    // Total cost and its components are in "cc"
    bool get_cost(const geo::point &where,
                   const geo::point &A,
                   const geo::point &B,
                   int fare_id,
                   int seconds_since_best_order_appeared,
                   bool is_driver_on_way_home,
                   cost_components &cc) const;
    
    // Returns the id of a local fare or defaults to the fare with the 0 id if nothing
    //  is found
    // Note: all costs are expressed in the local currency
    //  where   -   the point for fare to return
    //  weekday -   current weekday: 0 - Monday , ..., 6 - Sunday
    //  minutes -   minutes since 00:00
    int get_local_fare(const geo::point &where,
                                int weekday,
                                int minutes) const;
    
    // Sets the fare for the polygon
    void set_local_fare(const geo::point &where,
                        int resolution,
                        int fare_id);

    
private:

    // H3 indexes -> fare indexes
    // Note: that is the index directly into fares_ vector
    // Note: there can be many fares per polygon - each one within its schedule
    // Note: all farec per one polygon are full scanned! So it's a bad practice to have
    //   more than 1-5 fares per polygon
    std::unordered_map<H3Index, std::vector<int> > fare_index_;
    
    // All fares
    std::vector<fare> fares_;
};

} // namespace marketplace

#endif
