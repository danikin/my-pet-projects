/*
*       marketplace_api_drivers.cpp
*
*       (C) Denis Anikin 2020
*
*       Api impl for drivers in marketplace
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
#include "marketplace_api_drivers.h"

namespace marketplace
{

long long marketplace_api_drivers::reg_driver(
                    const std::string &first_name,
                    const std::string &last_name,
                    const std::string &car_make,
                    const std::string &car_model,
                    int car_year,
                    const std::string &car_color,
                    const std::string &driver_license,
                    const std::string &photo_url)
{
    // Save driver data in RAM
    driver_info dinfo;
    dinfo.first_name_ = first_name;
    dinfo.last_name_ = last_name;
    dinfo.car_make_ = car_make;
    dinfo.car_model_ = car_model;
    dinfo.car_year_ = car_year;
    dinfo.car_color_ = car_color;
    dinfo.driver_license_ = driver_license;
    dinfo.photo_url_ = photo_url;

    driver_db_.push_back(dinfo);
    
    // TODO: save in DB

    return driver_db_.size() - 1;
}

// Dumps driver database to json
void marketplace_api_drivers::to_json(nlohmann::json &result)
{
    result["event"] = "dump_drivers";
    int i = 0;
    for (auto &d : driver_db_)
    {
        result["data"].push_back({
            {"driver_id", i},
            {"first_name", d.first_name_},
            {"last_name", d.last_name_},
            {"car_make", d.car_make_},
            {"car_model", d.car_model_},
            {"car_year", d.car_year_},
            {"driver_license", d.driver_license_},
            {"photo_url", d.photo_url_}
        });
        ++i;
    }
}

} // namespace marketplace
