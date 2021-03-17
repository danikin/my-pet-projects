/*
*       marketplace_api_drivers.h
*
*       (C) Denis Anikin 2020
*
*       Headers for api for drivers in marketplace
*
*/

#ifndef _marketplace_api_drivers_h_
#define _marketplace_api_drivers_h_

#include "nlohmann/json.hpp"

#include "fifo_interface.h"
#include "object_track.h"

#include "marketplace.h"
#include "marketplace_assigned_order.h"

namespace marketplace
{

class marketplace_api_drivers
{
public:
    
    // Information about a driver
    struct driver_info
    {
        // Driver's data
        std::string first_name_;
        std::string last_name_;
        std::string car_make_;
        std::string car_model_;
        int car_year_;
        std::string car_color_;
        std::string driver_license_;
        std::string photo_url_;
    };
    
    // Registers a driver
    // Returns driver's id on success and -1 otherwise
    long long reg_driver(const std::string &first_name,
                         const std::string &last_name,
                         const std::string &car_make,
                         const std::string &car_model,
                         int car_year,
                         const std::string &car_color,
                         const std::string &driver_license,
                         const std::string &photo_url);

    // Sets driver's tarif
    bool set_driver_tarif(long long driver_id,
                            double minimal_price,
                            double price_per_km,
                            double price_per_min);
    
    // Set's driver's main settings
    bool set_driver_main_settings(bool notify_new_order,
                                  bool notify_price_increase,
                                  bool notify_district_high_demand,
                                  bool push_on,
                                  bool sound_on,
                                  bool smoking_permitted,
                                  bool child_seat,
                                  time_t no_notify_later,
                                  time_t no_notify_earlier);
    
    // Set's driver's other settings
    bool set_driver_other_sessings(double filter_max_distance_to_A_km,
                                   double filter_max_distance_A_to_B_km,
                                   bool filter_cash_accepted,
                                   bool filter_sberbank_online_accepted,
                                   bool auto_accept,
                                   const std::string &personal_address,
                                   bool filter_only_near_personal_address);
    
    // Set's driver's bad disricts for pickup and dropoff
    bool set_driver_bad_disticts(const std::vector<std::string> &bad_A_districts,
                                 const std::vector<std::string> &bad_B_districts);
    
    // Dumps driver database to json
    void to_json(nlohmann::json &result);
    
private:
    
    // Driver database
    // Driver's id is the sequential number in this vector
    // Note: don't mind "holes" because drivers are never deleted
    std::vector<driver_info> driver_db_;
};

} // namespace marketplace

#endif

