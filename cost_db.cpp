/*
*	cost_db.cpp
*
*	(C) Denis Anikin 2020
*
*	Impl for the cost database - core for tariffs
*
*/

#include <algorithm>

#include "cost_db.h"

namespace marketplace
{

cost_db::cost_db()
{
    // Upload fares from the database
    // TODO
    
    // This is a default fare with id == 0
    fares_.push_back({
        .fare_name_ = "default",
        .cost_per_km_ = 5.0,
        .opportunity_cost_per_minute_ = 5.0,
        .avg_speed_ = 0.5,
        .ratio_manhattan_distance_to_real_distance_ = 1.0,
        .order_cancel_risk_mitigation_ = {{0.0, 1.1}, {2.0, 1.125}, {3.0, 1.15}},
        .distance_to_pay_way_back_ = 100,
        .hotspots_ = {},
        .not_taking_best_order_coefficient_ = 1.0,
        .minimal_cost_ = 0,
        .additional_cost_ = 15, // Just include here 3 free minutes of free waiting
        .free_wait_duration_ = 180,
        .fare_time_frames_ = {{0, 1440}}, // The interval is [x,y)
        .weekdays_ = {true, true, true, true, true, true, true}
    });
    
    
}

bool cost_db::get_cost(const geo::point &where,
                const geo::point &A,
                const geo::point &B,
                int fare_id,
                int seconds_since_best_order_appeared,
                bool is_driver_on_way_home,
                cost_components &cc) const
{
    if (fare_id < 0 || fare_id > fares_.size())
    {
        std::cerr << "cost_db::get_cost: ERROR! bad fare ID: " << fare_id
        << ", fares_.size()=" << fares_.size() << std::endl;
        return false;
    }
    
    const fare *f = &fares_[fare_id];

    
    cc.fare_id_ = fare_id;
    cc.seconds_since_best_order_appeared_ = seconds_since_best_order_appeared;
    cc.is_driver_on_way_home_ = is_driver_on_way_home;
    
    // Determine the distance and the duration to A
    // Note: it is affected by the dynamic situation with average speed and the local specifics for
    //  ration between manhatten distances and real distances
    cc.distance_to_A_ = geo::manhattan_distance_km(where, A) * f->ratio_manhattan_distance_to_real_distance_;
    cc.duration_to_A_ = cc.distance_to_A_ / f->avg_speed_;
    
    // Now take the nominal cost of trip to A
    cc.nominal_cost_to_A_ = cc.distance_to_A_ * f->cost_per_km_ + cc.duration_to_A_ * f->opportunity_cost_per_minute_;

    // Determine the risk of cancellation (depends on the distance to A)
    cc.cancellation_risk_ = 1.0;
    if (!f->order_cancel_risk_mitigation_.empty())
    {
        auto i = std::upper_bound(f->order_cancel_risk_mitigation_.begin(),
                              f->order_cancel_risk_mitigation_.end(),
                                  cc.distance_to_A_,
                              [](float a, const std::pair<float, float> &b){
                                return a < b.first;
        });
        if (i == f->order_cancel_risk_mitigation_.begin())
        {
            // The distance is less than the first element in order_cancel_risk_mitigation_ then
            // the cancellation risk is neglabible
        }
        else
        if (i == f->order_cancel_risk_mitigation_.end())
        {
            // The distance is greater then any distance in order_cancel_risk_mitigation_ then
            //  the cancellation risk is maximal
            cc.cancellation_risk_ = f->order_cancel_risk_mitigation_[f->order_cancel_risk_mitigation_.size()-1].second;
        }
        else
            cc.cancellation_risk_ = (i-1)->second;
    }
    
    if (cc.cancellation_risk_ < 1.0)
    {
        std::cerr << "cost_db::get_cost: ERROR! bad cancellation_risk: " << cc.cancellation_risk_ << std::endl;
    }

    // That's the cost going to A
    cc.cost_to_A_ = cc.nominal_cost_to_A_ * cc.cancellation_risk_;
    
    // Now determine the cost going from A to B
    // Note: we use fares local to driver's position rather than to A or B because driver's
    //  costs are local to the point the she/he operates rather than to points where she/he goes
    cc.distance_A_to_B_ = geo::manhattan_distance_km(A, B) * f->ratio_manhattan_distance_to_real_distance_;
    cc.duration_A_to_B_ = cc.distance_A_to_B_ / f->avg_speed_;
    cc.cost_A_to_B_ = cc.distance_A_to_B_ * f->cost_per_km_ + cc.duration_A_to_B_ * f->opportunity_cost_per_minute_;
    
    // If the driver is not going home and the distance to pay the way back is set - then
    //  the return cost may be applied
    cc.distance_B_to_back_ = 0;
    cc.duration_B_to_back_ = 0;
    cc.is_B_a_hotspot_ = false;
    cc.cost_wayback_ = 0;
    
    if (!is_driver_on_way_home && f->distance_to_pay_way_back_)
    {
        cc.distance_B_to_back_ = geo::manhattan_distance_km(B, where) * f->ratio_manhattan_distance_to_real_distance_;
        if (cc.distance_B_to_back_ > f->distance_to_pay_way_back_)
        {
            // Distance back is too far - the cost MAY apply - it depends if point B is a hotspot within
            //  this fare
            GeoCoord coord_B = point_to_geocoord(B);
            for (int resolution = 6; resolution >= 4; --resolution)
            {
                H3Index h3 = geoToH3(&coord_B, resolution);
                if (std::binary_search(f->hotspots_.begin(), f->hotspots_.end(), h3))
                {
                    cc.is_B_a_hotspot_ = true;
                    break;
                }
            }
            
            // B is way too far and is not a hotspot - consider this in the cost
            if (!cc.is_B_a_hotspot_)
            {
                cc.duration_B_to_back_ = cc.distance_B_to_back_ / f->avg_speed_;
                cc.cost_wayback_ = cc.distance_B_to_back_ * f->cost_per_km_
                    + cc.duration_B_to_back_ * f->opportunity_cost_per_minute_;
            }
        }
    }
    
    // Now consider the non taking the best order part of the cost
    cc.cost_non_taking_order_ = seconds_since_best_order_appeared * f->not_taking_best_order_coefficient_ / 60.0;
    
    // Consider the minial cost and the additional cost
    cc.total_cost_ = cc.cost_to_A_ + cc.cost_A_to_B_ + cc.cost_wayback_ + cc.cost_non_taking_order_ + f->additional_cost_;
    if (cc.total_cost_ < f->minimal_cost_)
        cc.total_cost_ = f->minimal_cost_;
    
    return true;
}

int cost_db::get_local_fare(const geo::point &where,
                            int weekday,
                            int minutes) const
{
    if (weekday < 0 || weekday > 6 || minutes < 0 || minutes >= 1440)
    {
        std::cerr << "cost_db::get_local_fare: bad input params: weekday=" << weekday
            << ", minutes=" << minutes << std::endl;
        return 0;
    }
    
    GeoCoord coord = point_to_geocoord(where);
    
    // Try to a fare group in reasonably big resolutions
    // Note: go from finer to coarser
    for (int resolution = 6; resolution >= 3; --resolution)
    {
        H3Index h3 = geoToH3(&coord, resolution);
        auto i = fare_index_.find(h3);
        if (i != fare_index_.end())
        {
            // Found a fare vector
            auto &fare_ids = i->second;
            
            // Note: this is a fullscan! We expect a few fares per polygon
            for (auto fare_id : fare_ids)
            {
                if (fare_id < 0 || fare_id > fares_.size())
                {
                    std::cerr << "cost_db::get_local_fare: ERROR! bad fare ID: " << fare_id
                    << ", fares_.size()=" << fares_.size() << std::endl;
                    return 0;
                }
            
                const fare *f = &fares_[fare_id];

                // Check if it is a good weekday for this fare
                if (!f->weekdays_[weekday])
                    continue;
            
                // Check if it is a good time for this fare
                // Take the vector of times
                const auto &v = f->fare_time_frames_;
            
                // Found the first interval with ending time (second) >= seconds
                auto f_i = std::lower_bound(v.begin(), v.end(), minutes, fare::comp_fare_time_frames_);
                if (f_i != v.end() && f_i->first <= minutes)
                {
                    // Found a good fare! Return it
                    return fare_id;
                }
            } // for (auto i : fare_ids)
            
            // Not found - try other resolutions and/or fares
        } // if (i != fare_index_.end())
    } // for (int resolution = 6; resolution >= 3; --resolution)
    
    // Not found - return the default fare
    return 0;
}

void cost_db::set_local_fare(const geo::point &where,
                    int resolution,
                    int fare_id)
{
    if (fare_id < 0 || fare_id > fares_.size())
    {
        std::cerr << "cost_db::set_local_fare: ERROR! bad fare ID: " << fare_id
        << ", fares_.size()=" << fares_.size() << std::endl;
        return;
    }
    
    GeoCoord coord = point_to_geocoord(where);
    H3Index h3 = geoToH3(&coord, resolution);
    
    auto i = fare_index_.find(h3);
    if (i != fare_index_.end())
    {
        auto &fare_ids = i->second;
        if (std::find(fare_ids.begin(), fare_ids.end(), fare_id) != fare_ids.end())
        {
            // The fare is alread there - do nothing
            std::cerr << "cost_db::set_local_fare: the fare is already there: fare_id=" << fare_id
                << ", resolution=" << resolution << ", h3=" << h3 << std::endl;
            return;
        }
        else
            fare_ids.push_back(fare_id);
    }
    else
        fare_index_.insert({h3, {fare_id}});
    
    // TODO: save this change in DB!
}


std::ostream &operator<<(std::ostream &os, const cost_components &cc)
{
    return os <<
    "[fare_id_=" << cc.fare_id_ << ", " <<
        "seconds_since_best_order_appeared_=" << cc.seconds_since_best_order_appeared_ << ", " <<
        "is_driver_on_way_home_=" << cc.is_driver_on_way_home_ << ", " <<
        "distance_to_A_=" << cc.distance_to_A_ << ", " <<
        "duration_to_A_=" << cc.duration_to_A_ << ", " <<
        "nominal_cost_to_A_=" << cc.nominal_cost_to_A_ << ", " <<
        "cancellation_risk_=" << cc.cancellation_risk_ << ", " <<
        "cost_to_A_=" << cc.cost_to_A_ << ", " <<
        "distance_A_to_B_=" << cc.distance_A_to_B_ << ", " <<
        "duration_A_to_B_=" << cc.duration_A_to_B_ << ", " <<
        "cost_A_to_B_=" << cc.cost_A_to_B_ << ", " <<
        "distance_B_to_back_=" << cc.distance_B_to_back_ << ", " <<
        "duration_B_to_back_=" << cc.duration_B_to_back_ << ", " <<
        "is_B_a_hotspot_=" << cc.is_B_a_hotspot_ << ", " <<
        "cost_wayback_=" << cc.cost_wayback_ << ", " <<
        "cost_non_taking_order_=" << cc.cost_non_taking_order_ << ", " <<
        "total_cost_=" << cc.total_cost_ <<
        "]";
}

} // namespace marketplace

