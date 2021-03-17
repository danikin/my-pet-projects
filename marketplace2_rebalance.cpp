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

// Conforms distance in km to relosution and ring size
// Note: it's guaranteed that a circle of <=X km will be within
//  a polygon returned by distance_to_res_and_ring[X]
const int distance_to_res_and_ring[][2] = {
    {7, 1},     //  <= 1km (radius at least 1.22km and at worst 1.22x2 = 2.44km)
    {7, 2},     //  <= 2km (radius at least 1.22*x = 2.44km and at worst 1.22x3 = 3.66km)
    {6, 1},     //  <= 3km (radius at least 3.23km and at worst 6.46km)
    {6, 2},     //  <= 4km (radius at least 6.46km and at worst 9.69km)
    {6, 2},     //  <= 5km (radius at least 6.46km and at worst 9.69km)
    {6, 2},     //  <= 6km (radius at least 6.46km and at worst 9.69km)
    {6, 3},     //  <= 7km (radius at least 9.69km and at worst 12.92km)
    {5, 1},     //  <= 8km (radius at least 8.54km and at worst 17.08km)
    {6, 3},     //  <= 9km (radius at least 9.69km and at worst 12.92km)
    {5, 2},     //  <= 10km (radius at least 17.05km and at worst 25.59km)
    {5, 2},     //  <= 11km (radius at least 17.05km and at worst 25.59km)
    {5, 2},     //  <= 12km (radius at least 17.05km and at worst 25.59km)
    {5, 2},     //  <= 13km (radius at least 17.05km and at worst 25.59km)
    {5, 2},     //  <= 14km (radius at least 17.05km and at worst 25.59km)
    {5, 2},     //  <= 15km (radius at least 17.05km and at worst 25.59km)
    {5, 2},     //  <= 16km (radius at least 17.05km and at worst 25.59km)
    {5, 2},     //  <= 17km (radius at least 17.05km and at worst 25.59km)
    {4, 1},     //  <= 18km (radius at least 22.6km and at worst 45.2km)
    {4, 1},     //  <= 19km (radius at least 22.6km and at worst 45.2km)
    {4, 1},     //  <= 20km (radius at least 22.6km and at worst 45.2km)
    {4, 1},     //  <= 21km (radius at least 22.6km and at worst 45.2km)
    {4, 1},     //  <= 22km (radius at least 22.6km and at worst 45.2km)
    {4, 2},     //  <= 23km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 24km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 25km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 26km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 27km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 28km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 29km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 30km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 31km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 32km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 33km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 34km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 35km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 36km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 37km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 38km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 39km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 40km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 41km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 42km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 43km (radius at least 45.2km and at worst 67.8km)
    {4, 2},     //  <= 44km (radius at least 45.2km and at worst 67.8km)
    {4, 2}      //  <= 45km (radius at least 45.2km and at worst 67.8km)
};

void marketplace2::adjust_driver_best_order(rider2 &rider,
                                            driver2 &driver,
                                            float total_cost)
{
    auto ___dont_use_me___ = g_profiler_.profile("adjust_driver_best_order");
    
    // If this order is with ZERO revenue (e.g. a price view)
    // then just do nothing
    if (!rider.price_as_placed_)
    {
        if (debug_verbosity_ >= 2)
            std::cerr << "marketplace2::adjust_driver_best_order: debug - no revenue - quit: rider_id=" << rider.rider_id_ <<
            ", driver_id=" << driver.driver_id_ << std::endl;
        return;
    }

    // This is the adjusted profit margin given that the driver takes sum_extra_distance to
    // take this order
    float adjusted_profit_margin = (rider.price_as_placed_ - total_cost) / rider.price_as_placed_;

    if (debug_verbosity_ >= 2)
        std::cerr << "marketplace2::adjust_driver_best_order: debug: rider_id=" << rider.rider_id_
            << ", driver_id=" << driver.driver_id_ << ", adjusted_profit_margin=" <<
            adjusted_profit_margin << ", driver.best_profit_margin_=" << driver.best_profit_margin_ <<
            ", driver.best_rider_id_=" << driver.best_rider_id_ << std::endl;
    
    // Is this profit margin better than with another order?
    if (adjusted_profit_margin > driver.best_profit_margin_)
    {
        driver.best_rider_id_ = rider.rider_id_;
        driver.best_profit_margin_ = adjusted_profit_margin;
        
        // Remember the change to send it to users back
        changed_drivers_.push_back(driver.driver_id_);
    }
}

void marketplace2::adjust_oder_market_price(rider2 &rider,
                                            driver2 &driver,
                                            float total_cost)
{
    auto ___dont_use_me___ = g_profiler_.profile("adjust_oder_market_price");
    
    // This is the profit margin that "order" HAS to beat to be on top for this driver if placed
    float profit_margin_to_beat = driver.best_profit_margin_;
    
    // This is the price for the order to be on top for this driver
    // Now do the math
    // profit_margin_to_beat = (price - cost)/price = (price - costtoA - costAB - costBnext)/price =>
    // price * profit_margin_to_beat = price - costtoA - costAB - costBnext  =>
    // price * (1 - profit_margin_to_beat) = costtoA + costAB + costBnext =>
    // price = (costtoA + costAB + costBnext) / (1 - profit_margin_to_beat) =>
    // price = (distanceA * cost_per_km_at_A + costAB + costBnext) / (1 - profit_margin_to_beat)
    //
    // Try to be at least one percentage point better than
    // current profit margin. e.g. if it's 20% then we need 21%
    
    // Also don't beat the margin for the driver who already have this rider on top
    float margin_markup;
    if (driver.best_rider_id_ == rider.rider_id_)
        margin_markup = 0;
    else
    {
        // If profit margin is already too big then stop moving by percentage
        // points and swutch to percents. Why?
        // Because otherwise we can (and WILL) make the profit margin more than 100%
        // which will break everything :-)
        if (profit_margin_to_beat > 0.5)
        {
            margin_markup = (1-profit_margin_to_beat) * 0.01;
            // 1 - profit_margin_to_beat - margin_markup =
            // 1 - profit_margin_to_beat - 0.01 + profit_margin_to_beat * 0.01 =
            // 0.99 * (1 - profit_margin_to_beat) -> which is still less than 1
        }
        else
            margin_markup = 0.01;
    }

    float price_to_be_on_top = total_cost / (1 - profit_margin_to_beat - margin_markup);

    if (debug_verbosity_ >= 2)
        std::cerr << "marketplace2::adjust_oder_market_price: debug: rider_id=" << rider.rider_id_
            << ", driver_id=" << driver.driver_id_ << ", profit_margin_to_beat=" << profit_margin_to_beat <<
            ", price_to_be_on_top=" << price_to_be_on_top << ", margin_markup=" << margin_markup <<
            ", rider.market_price_=" << rider.market_price_ << ", rider.best_driver_id_=" << rider.best_driver_id_ << std::endl;
    
    // If this price is better than another one for the order then keep it
    if (!rider.best_driver_id_ || price_to_be_on_top < rider.market_price_)
    {
        rider.best_driver_id_ = driver.driver_id_;
        rider.market_price_ = price_to_be_on_top;
        
        // Remember the change to send it to users back
        changed_riders_.push_back(rider.rider_id_);
    }
}

void marketplace2::rebalance_for_order(rider2 &rider, rebalance_temp_and_debug_data &rtdd)
{
    auto ___dont_use_me___ = g_profiler_.profile("rebalance_for_order");
    
    if (debug_verbosity_ >= 1)
        std::cerr << "marketplace2::rebalance_for_order: rider_id=" << rider.rider_id_
    << ", rider.market_price_=" << rider.market_price_
    << ", rider.best_driver_id_=" << rider.best_driver_id_
    << ", rider.assigned_driver_id_=" << rider.assigned_driver_id_
    << ", riders_.size()=" << riders_.size() << ", drivers_.size()=" << drivers_.size() << std::endl;
    
    // Unset the best driver and the market price because if this function is called then
    //  rider's A and/or B can be changed and that can influent the best driver e.g. if
    //  a rider is not too far from the previous best driver
    rider.best_driver_id_ = 0;
    rider.market_price_ = 1000000000;
 
    // Note: those are the drivers who will be interested in this order - others
    //  will not be because it's negative in profit margin, so it's better to do
    //  nothing than taking an order like this
    driver_location_db_.get_objects_in_radius(rider.A_,
                                        distance_to_res_and_ring[(int)ceil(max_distance_toA_)][0],
                                        distance_to_res_and_ring[(int)ceil(max_distance_toA_)][1],
                                        rtdd.temp_neighbours_buffer_,
                                        rtdd.temp_compacted_buffer_,
                                        rtdd.temp_nearby_objects_,
                                        debug_verbosity_ >= 1);
    
    rtdd.debug_clusters_of_objects_.push_back(rtdd.temp_nearby_objects_.size());
    
    if (debug_verbosity_ >= 1)
    std::cerr << "marketplace2::rebalance_for_order: (drivers)temp_nearby_objects_.size()="
        << rtdd.temp_nearby_objects_.size() <<
        ", A.lat=" << rider.A_.latitude << ", A.lon=" << rider.A_.longitude << std::endl;
    
    if (rtdd.temp_nearby_objects_.empty())
    {
        std::cerr << "marketplace2::rebalance_for_order: NO DRIVERS AROUND for rider_id=" << rider.rider_id_ <<
        ", A.lat=" << rider.A_.latitude << ", A.lon=" << rider.A_.longitude << std::endl;
    }
    
    for (auto driver_id : rtdd.temp_nearby_objects_)
    {
        ++rtdd.debug_iterations1_;
        
        auto p_driver = get_non_assigned_driver_by_id(driver_id);
        if (!p_driver)
            continue;
        auto &driver = *p_driver;
        
        // Check on filters
        // TODO: if a driver does not want to take orders at all then they HAVE to be
        //  temporarily removed from unassigned_driver_location_db - it's more efficient
        //  then to filter them
        if (!passed_rider_filters(rider.rsf_, driver.dsf_, driver.where_, rider.A_, rider.B_) ||
            !passed_driver_filters(rider.rsf_, driver.dsf_, driver.where_, rider.A_, rider.B_))
        {
            std::cerr << "marketplace2::calc_oder_market_price: driver_id=" << driver.driver_id_ << " and rider_id=" <<
                rider.rider_id_ << " are incompatible with each other by filters" << std::endl;
            continue;
        }
        
        // Get the fare local to where the driver is
        int fare_id = cost_db_.get_local_fare(driver.where_,
                                     now_weekday_,
                                     now_minutes_);

        // Determine the cost for the using that fare
        cost_components cc;
        cost_db_.get_cost(driver.where_,
                               rider.A_,
                               rider.B_,
                               fare_id,
                               driver.ts_best_order_appeared_ ? now_ - driver.ts_best_order_appeared_ : 0,
                               !(driver.dsf_.pass_everything_ || !driver.dsf_.B_only_around_home_),
                               cc);
        
        // Save the maximum considered distance from a driver to A
        if (cc.distance_to_A_ > rtdd.debug_max_distance_considered_driver_to_A_)
        {
            rtdd.debug_max_distance_considered_driver_to_A_ = cc.distance_to_A_;
            rtdd.debug_max_distance_rider_id_ = rider.rider_id_;
            rtdd.debug_max_distance_driver_id_ = driver.driver_id_;
        }

        // Based on the total cost adjust the best order and the best profit margin for this driver
        adjust_driver_best_order(rider, driver, cc.total_cost_);
        
        // Based on the total cost and best profit adjust the market price for this order
        adjust_oder_market_price(rider, driver, cc.total_cost_);
        
        // Debug all cost components
        if (debug_verbosity_ >= 2)
            std::cerr << "marketplace2::rebalance_for_order: rider_id=" << rider.rider_id_ << ", driver_id=" <<
                driver.driver_id_ << ", cc=" << cc << std::endl;
    }

    if (debug_verbosity_ >= 1)
        rtdd.dump(std::cerr);
}

void marketplace2::rebalance_for_driver(driver2 &driver, rebalance_temp_and_debug_data &rtdd)
{
    auto ___dont_use_me___ = g_profiler_.profile("rebalance_for_driver");
    
    if (debug_verbosity_ >= 1)
        std::cerr << "marketplace2::rebalance_for_driver: driver_id=" << driver.driver_id_
    << ", driver.best_rider_id_=" << driver.best_rider_id_
    << ", driver.best_profit_margin_=" << driver.best_profit_margin_
    << ", driver.assigned_rider_id_=" << driver.assigned_rider_id_
    << ", riders_.size()=" << riders_.size() << ", drivers_.size()=" << drivers_.size() << std::endl;
    
    bool did_driver_have_best_rider = (driver.best_rider_id_ != 0);

    // Unset the best rider and the best profit margin because if this function is called then
    //  driver's position can be changed and that can influent the best rider e.g. if
    //  a driver is not too far from the previous best rider
    driver.best_rider_id_ = 0;
    driver.best_profit_margin_ = 0;
    
    // Note: those are the riders who the driver will be interested in - others
    //  will be not be because it's negative in profit margin, so it's better to do
    //  nothing than taking an order like this
    rider_location_db_.get_objects_in_radius(driver.where_,
                                        distance_to_res_and_ring[(int)ceil(max_distance_toA_)][0],
                                        distance_to_res_and_ring[(int)ceil(max_distance_toA_)][1],
                                        rtdd.temp_neighbours_buffer_,
                                        rtdd.temp_compacted_buffer_,
                                        rtdd.temp_nearby_objects_,
                                        debug_verbosity_ >= 1);
    rtdd.debug_clusters_of_objects_.push_back(rtdd.temp_nearby_objects_.size());

    if (debug_verbosity_ >= 1)
        std::cerr << "marketplace2::rebalance_for_driver: (riders)temp_nearby_objects_.size()="
            << rtdd.temp_nearby_objects_.size() <<
            ", where_.lat=" << driver.where_.latitude << ", where_.lon=" << driver.where_.longitude << std::endl;
    
    for (auto rider_id : rtdd.temp_nearby_objects_)
    {
        ++rtdd.debug_iterations1_;
        
        auto p_rider = get_non_assigned_rider_by_id(rider_id);
        if (!p_rider)
            continue;
        auto &rider = *p_rider;
       
        if (!passed_rider_filters(rider.rsf_, driver.dsf_, driver.where_, rider.A_, rider.B_) ||
            !passed_driver_filters(rider.rsf_, driver.dsf_, driver.where_, rider.A_, rider.B_))
            continue;

        // Get the fare local to where the driver is
        int fare_id = cost_db_.get_local_fare(driver.where_,
                                     now_weekday_,
                                     now_minutes_);

        // Determine the cost for the using that fare
        cost_components cc;
        cost_db_.get_cost(driver.where_,
                               rider.A_,
                               rider.B_,
                               fare_id,
                               driver.ts_best_order_appeared_ ? now_ - driver.ts_best_order_appeared_ : 0,
                               !(driver.dsf_.pass_everything_ || !driver.dsf_.B_only_around_home_),
                               cc);
        
        // Save the maximum considered distance from a driver to A
        if (cc.distance_to_A_ > rtdd.debug_max_distance_considered_driver_to_A_)
        {
            rtdd.debug_max_distance_considered_driver_to_A_ = cc.distance_to_A_;
            rtdd.debug_max_distance_rider_id_ = rider.rider_id_;
            rtdd.debug_max_distance_driver_id_ = driver.driver_id_;
        }
        
        // Readjust driver's best order and best profit margin
        adjust_driver_best_order(rider, driver, cc.total_cost_);

        // Debug all cost components
        if (debug_verbosity_ >= 2)
            std::cerr << "marketplace2::rebalance_for_driver: rider_id=" << rider.rider_id_ << ", driver_id=" <<
                driver.driver_id_ << ", cc=" << cc << std::endl;
        
    } // for (auto rider_id : riders_nearby_)
    
    // If the driver didn't have the best order when this function started (before we unset the best order of course :-) )
    //  and has it now then this means that the best order has just appeared - so save the time of it appearance
    if (driver.best_rider_id_ && !did_driver_have_best_rider)
        driver.ts_best_order_appeared_ = now_;

    if (debug_verbosity_ >= 1)
        rtdd.dump(std::cerr);
}

void marketplace2::rebalance_for_disappeared_order(rider2 &rider, rebalance_temp_and_debug_data &rtdd)
{
    auto ___dont_use_me___ = g_profiler_.profile("rebalance_for_disappeared_order");
    
    if (debug_verbosity_ >= 1)
    std::cerr << "marketplace2::rebalance_for_disappeared_order: rider_id=" << rider.rider_id_
    << ", riders_.size()=" << riders_.size() << ", drivers_.size()=" << drivers_.size()
    << ", rider.assigned_driver_id_=" << rider.assigned_driver_id_
    << ", rider.price_as_placed_=" << rider.price_as_placed_ << std::endl;
    
    // Price view (rider.price_as_placed_ == 0) -> it's disappeared -> OK
    // Assigned (rider.assigned_driver_id_ != 0) -> it's disappeared -> OK
    // Placed (rider.assigned_driver_id_ == 0  && rider.price_as_placed_ != 0) -> it's NOT disappeared -> NOT OK
    if (!rider.assigned_driver_id_ && rider.price_as_placed_)
    {
        std::cerr << "marketplace2::rebalance_for_disappeared_order: the order is still placed or unassigned, "
            "rider.price_as_placed_=" << rider.price_as_placed_ << ", rider.assigned_driver_id_=" <<
            rider.assigned_driver_id_ << std::endl;
        return;
    }
    
    // Iterate all drivers around this order and select the ones that leverage this order as the best
    // Note: use temp_nearby_drivers rather than rtdd.temp_nearby_objects_ because the
    //  nested call to rebalance_for_driver will use this field again
    std::vector<object_id> temp_nearby_drivers;
    driver_location_db_.get_objects_in_radius(rider.A_,
                                        distance_to_res_and_ring[(int)ceil(max_distance_toA_)][0],
                                        distance_to_res_and_ring[(int)ceil(max_distance_toA_)][1],
                                        rtdd.temp_neighbours_buffer_,
                                        rtdd.temp_compacted_buffer_,
                                        temp_nearby_drivers,
                                        debug_verbosity_ >= 1);
    rtdd.debug_clusters_of_objects_.push_back(temp_nearby_drivers.size());
    for (auto driver_id : temp_nearby_drivers)
    {
        // Consider only unassigned drivers
        auto p_driver = get_non_assigned_driver_by_id(driver_id);
        if (!p_driver)
            continue;
        auto &driver = *p_driver;
        
        // Rebalance for the driver
        // If this driver has this rider on top then rebalance for the driver
        if (p_driver->best_rider_id_ == rider.rider_id_)
            rebalance_for_driver(driver, rtdd);
    }

    if (debug_verbosity_ >= 1)
        rtdd.dump(std::cerr);
}

void marketplace2::rebalance_for_disappeared_driver(driver2 &driver, rebalance_temp_and_debug_data &rtdd)
{
    auto ___dont_use_me___ = g_profiler_.profile("rebalance_for_disappeared_driver");
    
    if (debug_verbosity_ >= 1)
        std::cerr << "marketplace2::rebalance_for_disappeared_driver: driver_id=" << driver.driver_id_ <<
        ", riders_.size()=" << riders_.size() << ", drivers_.size()=" << drivers_.size() <<
        ", driver.assigned_rider_id_=" << driver.assigned_rider_id_ << std::endl;
    
    if (!driver.assigned_rider_id_)
    {
        std::cerr << "marketplace2::rebalance_for_disappeared_driver: the driver is NOT assigned, driver_id="
            << driver.driver_id_ << std::endl;
        return;
    }
    
    // Iterate all riders around this driver and select the ones that leverage this driver as
    // the best for the market price
    // Note: use temp_nearby_riders rather than rtdd.temp_nearby_objects_ because the
    //  nested call to rebalance_for_order will use this field again
    std::vector<object_id> temp_nearby_riders;
    rider_location_db_.get_objects_in_radius(driver.where_,
                                        distance_to_res_and_ring[(int)ceil(max_distance_toA_)][0],
                                        distance_to_res_and_ring[(int)ceil(max_distance_toA_)][1],
                                        rtdd.temp_neighbours_buffer_,
                                        rtdd.temp_compacted_buffer_,
                                        temp_nearby_riders,
                                        debug_verbosity_ >= 1);
    rtdd.debug_clusters_of_objects_.push_back(temp_nearby_riders.size());
    for (auto rider_id : temp_nearby_riders)
    {
        // Consider only unassigned riders
        auto p_rider = get_non_assigned_rider_by_id(rider_id);
        if (!p_rider)
            continue;
        auto &rider = *p_rider;
        
        // Rebalance for the rider
        // If this rider has this driver as the best for the market price then rebalance for the rider
        //  because it will be a new driver
        if (p_rider->best_driver_id_ == driver.driver_id_)
            rebalance_for_order(rider, rtdd);
    }
    
    if (debug_verbosity_ >= 1)
        rtdd.dump(std::cerr);
}

void rebalance_temp_and_debug_data::dump(std::ostream &os)
{
    // `Sort and unique temp_nearby_objects_ stat
    unique(temp_nearby_objects_);
    
    os << "rebalance_temp_and_debug_data::dump, iterations1=" << debug_iterations1_ <<
        " , debug_max_distance_considered_driver_to_A=" << debug_max_distance_considered_driver_to_A_ <<
        " , debug_max_distance_rider_id=" << debug_max_distance_rider_id_ <<
        " , debug_max_distance_driver_id=" << debug_max_distance_driver_id_ <<
        " , debug_clusters_of_drivers.size()=" << debug_clusters_of_objects_.size() << ":";
    for (auto i : debug_clusters_of_objects_)
        os << " " << i;
    os << std::endl;
}

} // namespace marketplace
