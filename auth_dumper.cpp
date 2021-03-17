/*
*    auth_dumper.cpp
*
*    (C) Denis Anikin 2020
*
*    Dumper for all auth data
*
*/

#include "auth.h"
#include "auth_dumper.h"

#include "nlohmann/json.hpp"

namespace auth
{

void authenticator::user_db_to_json(nlohmann::json &result)
{
    result["event"] = "dump_user_db";
    int i = 0;
    for (const auto &u : user_db_)
    {
        result["data"].push_back({
                                 {"user_id", i},
                                 {"rider_id", u.rider_id_},
                                 {"driver_id", u.driver_id_},
                                 {"phone_number", u.phone_number_}
        });
        
        ++i;
    }
}

void authenticator::tokens_to_json(nlohmann::json &result)
{
    result["event"] = "dump_token_db";
    for (const auto &u : tokens_)
        result["data"].push_back({{"token", u.first}, {"user_id", u.second}});
}

void authenticator::phones_to_json(nlohmann::json &result)
{
    result["event"] = "dump_phones_db";
    for (const auto &u : phones_)
        result["data"].push_back({{"phone", u.first}, {"user_id", u.second}});
}

void authenticator::smscodes_to_json(nlohmann::json &result)
{
    result["event"] = "dump_smscodes";
    for (const auto &u : smscodes_)
        result["data"].push_back({{"smscode", u.first}, {"expiration_time", u.second}});
}

} // namespace auth
