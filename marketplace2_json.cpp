/*
*	marketplace2.json
*
*	(C) Denis Anikin 2020
*
*	JSON interface for marketplace2
*
*/

#include <istream>
#include <time.h>
#include <sys/time.h>

#include "marketplace2.h"
#include "geolocation.h"
#include "profiler.h"

#include "nlohmann/json.hpp"

namespace marketplace
{

profiler g_profiler_(3);

using namespace geo;

using json = nlohmann::json;


bool test_simulate_driver_motion(long long driver_id, const geo::point &from, const geo::point &to)
{
    // Note: this command cuts non ascii and spaces: | perl -pe 's/[^[:ascii:]]//g' | perl -pe 's/[ ]//g'
    //  for some reason json parser does not accept it
    std::string command = "curl 'http://router.project-osrm.org/trip/v1/driving/" + std::to_string(from.longitude) + ","+
        std::to_string(from.latitude) + ";" +
        std::to_string(to.longitude)  + "," +
        std::to_string(to.latitude) + "?source=first&destination=last&roundtrip=false' 2>/dev/null | perl -pe 's/[^[:ascii:]]//g' | perl -pe 's/[ ]//g' | ./test_motion_simulator 0 " +
            std::to_string(driver_id) + " 1 >>/tmp/fifos/from_client&";
    std::cerr << "test_simulate_driver_motion: simulator command: " << command << std::endl;
    if (system(command.c_str()) != 0)
    {
        std::cerr << "test_simulate_driver_motion: simulator command failed, errno=" << errno << ", strerror=" << strerror(errno) << std::endl;
        return false;
    }
    else
        return true;
}

template <class T>
T json_get_with_default(json &jdata, const std::string &name, const T &def)
{
    if (jdata[name].empty())
        return def;
    else
        return jdata[name];
}
template <class T>
T json_get(json &jdata, const std::string &name)
{
    return jdata[name];
}
object_id json_get_rider_id(json &jdata) { return jdata["rider_id"]; }
object_id json_get_driver_id(json &jdata) { return jdata["driver_id"]; }
point json_get_point(json &jdata, const std::string &lat_name = "lat", const std::string &lon_name = "lon")
{
    return {.longitude = jdata[lon_name], .latitude = jdata[lat_name]};
}

template <class T>
bool change_var(json &jdata, T &t, const std::string &name)
{
    auto &j = jdata[name];
    if (j.empty())
        return false;
    else
    {
        t = j;
        return true;
    }
}

void set_rsf_helper(json &element, rider2 &d)
{
    if (!d.rsf_.pass_everything_ && d.rsf_.no_smoking_ != 0)
        element["rsf_no_smoking"]=d.rsf_.no_smoking_;
    if (!d.rsf_.pass_everything_ && d.rsf_.need_childseat_ != 0)
        element["rsf_need_childseat"]=d.rsf_.need_childseat_;
    if (!d.rsf_.pass_everything_ && d.rsf_.only_cash_ != 0)
        element["rsf_only_cash"]=d.rsf_.only_cash_;
    if (!d.rsf_.pass_everything_ && d.rsf_.only_sbrebank_online_ != 0)
        element["rsf_only_sbrebank_online"]=d.rsf_.only_sbrebank_online_;
    if (!d.rsf_.pass_everything_ && d.rsf_.with_pet_ != 0)
        element["rsf_with_pet"]=d.rsf_.with_pet_;
}

// Broadcasts changes back to users
void broadcast_back_changes(rider2 &rider, std::ostream &os)
{
    auto ___dont_use_me___ = g_profiler_.profile("broadcast_back_changes/rider");

    json result;
    result["event"] = "marketplace2_update";
    result["data"]["riders"].push_back({
        {"rider_id", rider.rider_id_},
        {"latA", rider.A_.latitude},
        {"lonA", rider.A_.longitude},
        {"latB", rider.B_.latitude},
        {"lonB", rider.B_.longitude},
        {"price_as_placed", rider.price_as_placed_},
        {"market_price", rider.market_price_},
        {"best_driver_id", rider.best_driver_id_},
        {"assigned_driver_id", rider.assigned_driver_id_},
    });
    
    // Rider side filters
    set_rsf_helper(result["data"]["riders"][result["data"]["riders"].size()-1], rider);

    auto ___dont_use_me___2 = g_profiler_.profile("broadcast_back_changes/rider - sending");

    os << result << std::endl;
}
void set_dsf_helper(json &element, driver2 &d)
{
    if (d.dsf_.auto_accept_best_order_) element["dsf_auto_accept_best_order"]=1;
    if (!d.dsf_.pass_everything_ && d.dsf_.max_distance_to_A_km_ != 0) element["dsf_max_distance_to_A_km"]=d.dsf_.max_distance_to_A_km_;
    if (!d.dsf_.pass_everything_ && d.dsf_.max_distance_to_B_km_ != 0) element["dsf_max_distance_to_B_km"]=d.dsf_.max_distance_to_B_km_;
    if (!d.dsf_.pass_everything_ && d.dsf_.no_cash_) element["dsf_no_cash"]=1;
    if (!d.dsf_.pass_everything_ && d.dsf_.no_sbrebank_online_) element["dsf_no_sbrebank_online"]=1;
    if (!d.dsf_.pass_everything_ && d.dsf_.B_only_around_home_)
    {element["dsf_home_lat"]=d.dsf_.home_.latitude;
        element["dsf_home_lon"]=d.dsf_.home_.longitude;
        element["dsf_home_area_radius_km"]=d.dsf_.home_area_radius_km_;}
    
    if (!d.dsf_.pass_everything_ && d.dsf_.bad_districts_on_)
    {
        element["dsf_bad_district_radius_km_"] = d.dsf_.bad_district_radius_km_;
        for (auto &bd : d.dsf_.bad_districts_A_)
            element["dsf_bad_districts_A"].push_back({
                {"latitude",bd.latitude},{"longitude",bd.longitude}});
        for (auto &bd : d.dsf_.bad_districts_B_)
            element["dsf_bad_districts_B"].push_back({
                {"latitude",bd.latitude},{"longitude",bd.longitude}});             }
    
    if (!d.dsf_.pass_everything_ && d.dsf_.smoking_allowed_) element["dsf_smoking_allowed"]=1;
    if (!d.dsf_.pass_everything_ && d.dsf_.chidseat_) element["dsf_chidseat"]=1;
    if (!d.dsf_.pass_everything_ && d.dsf_.no_pets_) element["dsf_no_pets"]=1;
    if (!d.dsf_.pass_everything_ && d.dsf_.no_big_parties_) element["dsf_no_big_parties"]=1;
}

void broadcast_back_changes(driver2 &driver, std::ostream &os, time_t now)
{
    auto ___dont_use_me___ = g_profiler_.profile("broadcast_back_changes/driver");

    json result;
    result["event"] = "marketplace2_update";
    result["data"]["drivers"].push_back({
        {"driver_id", driver.driver_id_},
        {"lat", (double)driver.where_.latitude},
        {"lon", (double)driver.where_.longitude},
        {"best_rider_id", driver.best_rider_id_},
        {"best_profit_margin", driver.best_profit_margin_},
        {"assigned_rider_id", driver.assigned_rider_id_},
        {"seconds_since_best_order_appeared",
            driver.ts_best_order_appeared_ ? now - driver.ts_best_order_appeared_ : 0}
    });
    // Driver side filters
    set_dsf_helper(result["data"]["drivers"][result["data"]["drivers"].size()-1], driver);

    auto ___dont_use_me___2 = g_profiler_.profile("broadcast_back_changes/driver - sending");
    
    os << result << std::endl;
}

void marketplace2::rebalance_after_assignment(object_id rider_id, object_id driver_id)
{
    std::cerr << "marketplace2::rebalance_after_assignment: rider_id=" << rider_id
        << ", driver_id=" << driver_id << std::endl;
    
    changed_riders_.push_back(rider_id);
    changed_drivers_.push_back(driver_id);
    
    auto *rider = get_rider_by_id(rider_id);
    auto *driver = get_driver_by_id(driver_id);
    // If the order is assigned then rebalance for both - the driver and the rider
    // Note: rebalance for the disappeared order first because it will make sure that
    //  all drivers nearby have the best top orders and then later rebalance for
    //  the disappeared driver will be run having all the riders nearby leveaged
    //  aleady best orders for drivers nearby

    rebalance_temp_and_debug_data rtdd;
    profile([this, rider, &rtdd](){rebalance_for_disappeared_order(*rider, rtdd);},
            "marketplace2::rebalance_for_disappeared_order", debug_verbosity_ >= 1);
    
    profile([this, driver, &rtdd](){rebalance_for_disappeared_driver(*driver, rtdd);},
            "marketplace2::rebalance_for_disappeared_driver", debug_verbosity_ >= 1);
}

void marketplace2::rebalance_after_unassignment(object_id rider_id, object_id driver_id)
{
    std::cerr << "marketplace2::rebalance_after_unassignment: rider_id=" << rider_id
        << ", driver_id=" << driver_id << std::endl;
    
    if (rider_id)
        changed_riders_.push_back(rider_id);
    changed_drivers_.push_back(driver_id);
    
    // Rebalance for the driver
    // Note: this is like a new driver appeared
    // If the order in unassigned without its removal then additionally rebalance for the rider
    // Note: this is a like a new order appeared
    // Note: rebalance for the order first because it will make sure that
    //  all drivers nearby have the best top orders and then later rebalance for
    //  the driver will be run having all the riders nearby leveaged
    //  aleady best orders for drivers nearby

    rebalance_temp_and_debug_data rtdd;
    if (rider_id)
    {
        auto *rider = get_rider_by_id(rider_id);
        profile([this, rider, &rtdd](){rebalance_for_order(*rider, rtdd);},
            "marketplace2::rebalance_for_order", debug_verbosity_ >= 1);
    }
    
    auto *driver = get_driver_by_id(driver_id);
    profile([this, driver, &rtdd](){rebalance_for_driver(*driver, rtdd);},
            "marketplace2::rebalance_for_driver", debug_verbosity_ >= 1);
}

bool marketplace2::process_json_request(json &j, std::ostream &os)
{
    const std::string &event = j["event"];
    json &jdata = j["data"];
    object_id rider_id = 0, driver_id = 0;
    
    now_ = time(NULL);
    struct tm tt;
    localtime_r(&now_, &tt);
    now_weekday_ = tt.tm_wday-1;
    if (now_weekday_ == -1)
        now_weekday_ = 6;
    now_minutes_ = tt.tm_hour * 60 + tt.tm_min;
    
    bool debug_print_per_request = (debug_verbosity_ >= 1 || event != "marketplace2_update_driver_position");
    
    if (debug_print_per_request)
        std::cerr << "marketplace2::process_json_request, debug: got a request " << j << std::endl;
    
    g_profiler_.print_stat();
    
    // Print RPS and the running time of this function every 3 seconds
    auto ___dont_use_me___ = g_profiler_.profile("process_json_request");
    
    changed_riders_.clear();
    changed_drivers_.clear();
    
    // Process requests
    if (event == "marketplace2_udpate_order")
    {
        // Print RPS and the running time of this function every 3 seconds
        auto ___dont_use_me___ = g_profiler_.profile("marketplace2_udpate_order");
        
        bool place_order_at_market_price = !jdata["place_order_at_market_price"].empty();
        double price = jdata["placed_order_price"].empty() ? 0.0 : (double)jdata["placed_order_price"];
        
        point point_A = json_get_point(jdata, "latA", "lonA");
        point point_B = json_get_point(jdata, "latB", "lonB");
        
        auto *rider = udpate_order(rider_id = json_get_rider_id(jdata),
                     point_A,
                     point_B,
                     price,
                     place_order_at_market_price);
    
        // Log traffic info just to debug it
        traffic_info_entry *tieA = driver_tracks_.get_traffic_info(point_A);
        if (tieA)
            std::cerr << "GOT TRAFFIC INFO FOR A: " << point_A << ", " << *tieA << std::endl;
        traffic_info_entry *tieB = driver_tracks_.get_traffic_info(point_B);
        if (tieB)
            std::cerr << "GOT TRAFFIC INFO FOR B: " << point_B << ", " << *tieB << std::endl;

        // Rebalance only for this order - optimization
        if (rider)
        {
            changed_riders_.push_back(rider_id);
            rebalance_temp_and_debug_data rtdd;
            
            profile([this, rider, &rtdd](){rebalance_for_order(*rider, rtdd);},
                    "marketplace2::rebalance_for_order", debug_verbosity_ >= 1);
        }
        else
            std::cerr << "marketplace2::process_json_request: ERROR: can't udpate_order " << rider_id << std::endl;
    }
    else
    if (event == "marketplace2_update_driver_position")
    {
        // Print RPS and the running time of this function every 3 seconds
        auto ___dont_use_me___ = g_profiler_.profile("marketplace2_update_driver_position");
        
        auto *driver = update_driver_position(driver_id = json_get_driver_id(jdata), json_get_point(jdata));
        
        // Rebalance only for this driver - optimization
        if (driver)
        {
            // Rebalance only if a driver does not have an assigned rider
            // TODO: in a future it'd be wise to rebalance even for an assigned driver once in a while
            //  because when a ride get to finish a driver has to know about the next potential ride
            if (!driver->assigned_rider_id_)
            {
                // FOR TEST REASONS!!!!!
                driver->dsf_.auto_accept_best_order_ = true;
            
                changed_drivers_.push_back(driver_id);
                rebalance_temp_and_debug_data rtdd;
                profile([this, driver, &rtdd](){rebalance_for_driver(*driver, rtdd);},
                        "marketplace2::rebalance_for_driver", debug_verbosity_ >= 1);
            }
            else
            {
                if (debug_verbosity_ >= 1)
                    std::cerr << "marketplace2::process_json_request: skip rebalance_for_driver because driver " <<
                        driver_id << " is assigned to rider " << driver->assigned_rider_id_ << std::endl;
            }
        }
        else
            std::cerr << "marketplace2::process_json_request: ERROR: can't update_driver_position " << driver_id << std::endl;
    }
    else
    if (event == "marketplace2_assign_order")
    {
        // Print RPS and the running time of this function every 3 seconds
        auto ___dont_use_me___ = g_profiler_.profile("marketplace2_assign_order");
        
        if (assign_order(rider_id = json_get_rider_id(jdata), driver_id = json_get_driver_id(jdata)))
            rebalance_after_assignment(rider_id, driver_id);
        else
            std::cerr << "marketplace2::process_json_request: ERROR: can't assign_order,  rider_id=" << rider_id
                << ", driver_id=" << driver_id << std::endl;
    }
    else
    // Change driver's filters
    if (event == "marketplace2_change_dsf")
    {
        object_id driver_id = json_get_driver_id(jdata);
        auto *driver = get_driver_by_id(driver_id);
        if (!driver)
        {
            std::cerr << "marketplace2::process_json_request: marketplace2_change_dsf: ERROR! BAD DRIVER ID: " << driver_id << std::endl;
        }
        else
        {
            change_var(jdata, driver->dsf_.pass_everything_, "pass_everything");
            change_var(jdata, driver->dsf_.auto_accept_best_order_, "auto_accept_best_order");
            change_var(jdata, driver->dsf_.max_distance_to_A_km_, "max_distance_to_A_km");
            change_var(jdata, driver->dsf_.max_distance_to_B_km_, "max_distance_to_B_km");
            change_var(jdata, driver->dsf_.no_cash_, "no_cash");
            change_var(jdata, driver->dsf_.no_sbrebank_online_, "no_sbrebank_online");
            change_var(jdata, driver->dsf_.B_only_around_home_, "B_only_around_home");
            change_var(jdata, driver->dsf_.home_.latitude, "home_lat");
            change_var(jdata, driver->dsf_.home_.longitude, "home_lon");
            change_var(jdata, driver->dsf_.home_area_radius_km_, "home_area_radius_km");
            if (change_var(jdata, driver->dsf_.bad_districts_on_, "bad_districts_on"))
            {
                json &j_bad_districts_A = jdata["bad_districts_A"];
                driver->dsf_.bad_districts_A_.clear();
                for (auto &j : j_bad_districts_A)
                    driver->dsf_.bad_districts_A_.push_back({.longitude=j["lon"], .latitude=j["lat"]});

                json &j_bad_districts_B = jdata["bad_districts_A"];
                driver->dsf_.bad_districts_B_.clear();
                for (auto &j : j_bad_districts_B)
                    driver->dsf_.bad_districts_B_.push_back({.longitude=j["lon"], .latitude=j["lat"]});
            }
            change_var(jdata, driver->dsf_.bad_district_radius_km_, "bad_district_radius_km");
           /* change_var(jdata, driver->dsf_.own_tarif_minimum_price_, "own_tarif_minimum_price");
            change_var(jdata, driver->dsf_.price_per_km_, "price_per_km");
            change_var(jdata, driver->dsf_.price_per_min_, "price_per_min");*/
            change_var(jdata, driver->dsf_.smoking_allowed_, "smoking_allowed");
            change_var(jdata, driver->dsf_.chidseat_, "chidseat");
            change_var(jdata, driver->dsf_.no_pets_, "no_pets");
            change_var(jdata, driver->dsf_.no_big_parties_, "no_big_parties");
            
            changed_drivers_.push_back(driver_id);
        }
    }
    else
    if (event == "marketplace2_set_debug_verbosity")
        set_debug_verbosity(json_get<int>(jdata, "level"));
    else
    if (event == "marketplace2_unassign_order")
    {
        // Print RPS and the running time of this function every 3 seconds
        auto ___dont_use_me___ = g_profiler_.profile("marketplace2_unassign_order");
        
        bool remove = json_get<int>(jdata, "remove");
        auto *driver = unassign_order(rider_id = json_get_rider_id(jdata), remove);
        if (driver)
            rebalance_after_unassignment(remove ? rider_id : 0, driver->driver_id_);
        else
            std::cerr << "marketplace2::process_json_request: ERROR: can't assign_order,  rider_id=" << rider_id
                << std::endl;
    }
    else
    if (event == "marketplace2_set_local_fare")
    {
        // Change the locl fare per this point and per this resolution
        cost_db_.set_local_fare(json_get_point(jdata),
                                json_get<int>(jdata, "resolution"),
                                json_get<int>(jdata, "fare_id"));
    }
    /*else
    if (event == "marketplace2_start_ride_simulator")
    {
        object_id driver_id = json_get_driver_id(jdata);
        auto *driver = get_assigned_driver_by_id(driver_id);
        if (!driver)
        {
            std::cerr << "marketplace2::process_json_request: marketplace2_start_ride_simulator: ERROR! BAD DRIVER ID: "
                << driver_id << std::endl;
        }
        else
        {
            auto *rider = get_assigned_rider_by_id(driver->assigned_rider_id_);
            if (!rider)
                std::cerr << "marketplace2::process_json_request: marketplace2_start_ride_simulator: ERROR! BAD RIDER ID: "
                    << driver->assigned_rider_id_ << std::endl;
            else
                // Simulate motion from driver's position to the pickup point
                // TODO: check if a driver has already picked a rider up - in that case simulate
                //  the motion to rider's B
                test_simulate_driver_motion(driver_id, driver->where_, rider->A_);
        }
    }*/
    else
        return false;

    {
    auto ___dont_use_me___ = g_profiler_.profile("rider auto placements");
    
    // THIS IS ONLY FOR TEST REASONS!!!!!
    // Search for riders with ready to go orders and make auto placement of orders for them
    // Note: this loop MUST be before the next one (auto accept) because if a rider places an order
    //  then it's better to auto accept it right away
    temp_changed_riders_.resize(changed_riders_.size());
    std::copy(changed_riders_.begin(), changed_riders_.end(), temp_changed_riders_.begin());
    for (auto i : temp_changed_riders_)
    {
        rider_id = i;
        auto *rider = get_rider_by_id(rider_id);
        if (!rider)
        {
            std::cerr << "marketplace2::process_json_request: changes: ERROR! BAD RIDER ID: " << rider_id << std::endl;
            continue;
        }
        // Somebody is ready to take this order - place it
        if (rider->best_driver_id_)
        {
            if (debug_verbosity_ >= 1)
                std::cerr << "marketplace2::process_json_request: try AUTO PLACE the order for rider "
                    << rider_id << std::endl;

            auto ___dont_use_me___3 = g_profiler_.profile("before update_order");
            
            // Place the order at the market price and rebalance
            if (!udpate_order(rider_id, rider->A_, rider->B_, 0, true))
                std::cerr << "marketplace2::process_json_request: COULD NOT PLACE THE ORDER FOR RIDER "
                    << rider_id << std::endl;
            else
            {
                if (debug_verbosity_ >= 1)
                    std::cerr << "marketplace2::process_json_request: ORDER AUTO PLACED FOR RIDER "
                        << rider_id << std::endl;
                
                changed_riders_.push_back(rider_id);
                
                rebalance_temp_and_debug_data rtdd;
                profile([this, rider, &rtdd](){rebalance_for_order(*rider, rtdd);},
                        "marketplace2::rebalance_for_order", debug_verbosity_ >= 1);
            }
        }
    }
    }

    {
    auto ___dont_use_me___3 = g_profiler_.profile("drivers check autoaccepts");
    
    // Check for autoaccept for changed drivers
    // Note: we use a temp vector because under the hood there will be rebalance with new
    //  changes to changed_drivers_
    temp_changed_drivers_.resize(changed_drivers_.size());
    std::copy(changed_drivers_.begin(), changed_drivers_.end(), temp_changed_drivers_.begin());
    for (auto i : temp_changed_drivers_)
    {
        driver_id = i;
        auto *driver = get_driver_by_id(driver_id);
        if (!driver)
        {
            std::cerr << "marketplace2::process_json_request: changes: ERROR! BAD DRIVER ID: " << driver_id << std::endl;
            continue;
        }
        // If this driver auto accepts orders then
        // check if there is a win-win situation with this driver - to accept the order
        if (driver->dsf_.auto_accept_best_order_ && driver->best_rider_id_)
        {
            if (debug_verbosity_ >= 1)
                std::cerr << "marketplace2::process_json_request: try to auto accept the best order for driver "
                    << driver_id << std::endl;
            
            auto *rider = get_rider_by_id(driver->best_rider_id_);
            if (!rider)
            {
                std::cerr << "marketplace2::process_json_request: changes: ERROR! BAD BEST RIDER ID: " << driver->best_rider_id_ << ", for driver " << driver_id << std::endl;
                continue;
            }
            if (rider->best_driver_id_ == driver_id)
            {
                // Win-win
                rider_id = driver->best_rider_id_;
                if (assign_order(rider_id, driver_id))
                {
                    rebalance_after_assignment(rider_id, driver_id);
                    
                    auto ___dont_use_me___2 = g_profiler_.profile("test_simulate_driver_motion");
                    
                    // Start a ride simuation to the pickup point
                    // FOR TEST REASONS ONLY!!!!!
                    test_simulate_driver_motion(driver_id, driver->where_, rider->A_);
                }
            }
            else
            {
                std::cerr << "marketplace2::process_json_request: no auto accept because of no win-win for driver "
                    << driver_id << ", driver->best_rider_id_=" << driver->best_rider_id_ << ", rider->best_driver_id_="
                    << rider->best_driver_id_ << std::endl;
            }
        }
    }
    }

    
    auto ___dont_use_me___4 = g_profiler_.profile("send_changes");
    
    /*struct timeval tv;
    gettimeofday(&tv, NULL);
    long long sec1 = tv.tv_sec * 1000000 + tv.tv_usec;
    
    // Rebalance
    rebalance();

    gettimeofday(&tv, NULL);
    long long sec2 = tv.tv_sec * 1000000 + tv.tv_usec;
        
    std::cerr << "marketplace2::process_json_request: rebalance has taken " << (sec2 - sec1)/1000 << " miliseconds" << std::endl;*/
    
    /*
    rebalance_temp_and_debug_data rtdd;
    profile([this, &rtdd](){rebalance(rtdd);});
    rtdd.dump(std::cerr);
    */
    
    unique(changed_riders_);
    unique(changed_drivers_);
    
    int num_changed = changed_riders_.size() + changed_drivers_.size();
    
    // Check if anything has changed and broadcast changes back to users
    // TODO: in a future change this broadcast script "tail -n 1 -f /tmp/fifos/to_client </dev/null&" with
    //  something that filters by a distance
    for (auto i : changed_riders_)
    {
        auto *rider = get_rider_by_id(i);
        if (!rider)
        {
            std::cerr << "marketplace2::process_json_request: changes: ERROR! BAD RIDER ID: " << i << std::endl;
            continue;
        }

        if (debug_verbosity_ >= 1)
            std::cerr << "marketplace2::process_json_request: changes: rider.rider_id_=" << rider->rider_id_ <<
            ", rider->market_price_=" << rider->market_price_ << ", best_driver_id_=" << rider->best_driver_id_ << std::endl;
        
        broadcast_back_changes(*rider, os);
    }
    for (auto i : changed_drivers_)
    {
        auto *driver = get_driver_by_id(i);
        if (!driver)
        {
            std::cerr << "marketplace2::process_json_request: changes: ERROR! BAD DRIVER ID: " << i << std::endl;
            continue;
        }

        if (debug_verbosity_ >= 1)
            std::cerr << "marketplace2::process_json_request: changes: driver.driver_id_=" << driver->driver_id_ <<
            ", driver.best_rider_id_=" << driver->best_rider_id_ << ", driver.best_profit_margin_=" << driver->best_profit_margin_
            << std::endl;
        broadcast_back_changes(*driver, os, now_);
    }
    
    if (num_changed)
    {
        if (debug_print_per_request)
            std::cerr << "marketplace2::process_json_request: changes sent, num_changed=" << num_changed << std::endl;
    }
    
    return true;
}

} // namespace marketplace
