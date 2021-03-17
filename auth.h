/*
*    auth.h
*
*    (C) Denis Anikin 2020
*
*    Headers for authentication of the taxi service
*
*/

#ifndef _auth_h_included_
#define _auth_h_included_

#include <string>
#include <set>
#include <tuple>
#include <vector>
#include <map>
#include <unordered_map>
#include <time.h>

#include "nlohmann/json.hpp"

namespace auth
{

struct user
{
    user(bool valid) : rider_id_(-1), driver_id_(-1), valid_(valid) {}
    user(long long rider_id, long long driver_id) : rider_id_(rider_id), driver_id_(driver_id), valid_(true) {}
       
    long long rider_id_;
    long long driver_id_;
    
    bool valid_;
    
    std::string phone_number_;

    // Note: a user can be easily valid with rider_id == -1 && driver_id == -1
    // This is when it's a freshly created user entry without attaching a rider or a driver to it
    bool is_valid() { return valid_; }
};

class authenticator
{
public:
    
    authenticator();
    
    // Send smscode for the specified phone number
    bool send_smscode(const std::string &phone_number);
    
    // Authenticate by smscode
    // On error a returned user object will be not valid
    struct auth_result {std::string token_; long long user_id_; user u_; bool is_ok() { return u_.is_valid();}};
    auth_result authenticate_by_smscode(const std::string &phone_number, const std::string &smscode);
    
    // Authenticate by token
    // On error a returned user object will be not valid
    auth_result authenticate_by_token(const std::string &token);

    // Returns a user by user_id
    // On error a user object will be not valid
    user get_user_db_entry(long long user_id)
    {
        if (user_id < 0 || user_id >= user_db_.size() || !user_db_[user_id].valid_)
            return user(false);
        else
            return user_db_[user_id];
    }
    
    // Sets driver_id for a user
    // Note: once the driver is set IT CAN'T BE CHANGED
    bool set_driver(long long user_id, long long driver_id)
    {
        if (user_id < 0 || user_id >= user_db_.size() || !user_db_[user_id].valid_)
            return false;
        
        // Is a driver set?
        if (user_db_[user_id].driver_id_ != -1)
            return false;
        
        // Set the driver
        user_db_[user_id].driver_id_ = driver_id;
        
        // TODO: update DB!
        
        return true;
    }

    // Sets rider_id for a user
    // Note: once the driver is set IT CAN'T BE CHANGED
    bool set_rider(long long user_id, long long rider_id)
    {
        if (user_id < 0 || user_id >= user_db_.size() || !user_db_[user_id].valid_)
            return false;
        
        // Is a driver set?
        if (user_db_[user_id].rider_id_ != -1)
            return false;
        
        // Set the rider
        user_db_[user_id].rider_id_ = rider_id;
  
        // TODO: update DB!

        return true;
    }
    
    
    // Dumps user db to json
    void user_db_to_json(nlohmann::json &result);

    // Dumps tokens to json
    void tokens_to_json(nlohmann::json &result);

    // Dump phones to json
    void phones_to_json(nlohmann::json &result);

    // Dump smscodes to json
    void smscodes_to_json(nlohmann::json &result);


private:
    
    // User database
    // user_id is an index to the vector
    std::vector<user> user_db_;
    
    // token -> user_id
    std::unordered_map<std::string, long long> tokens_;
    
    // phone -> user_id
    std::unordered_map<std::string, long long> phones_;

    // smscode + phone -> expiration time
    std::unordered_map<std::string, time_t> smscodes_;
};

}

#endif

