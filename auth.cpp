/*
*    auth.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for authentication of the taxi service
*
*/

#include <iostream>

#include "auth.h"

namespace auth
{

std::string generate_random_alpha_numeric_string(int rand_module, int num_chars)
{
    const char *nums = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    std::string result;
    result.resize(num_chars);
    for (int i = 0; i < num_chars; ++i)
        result[i] = nums[rand() % rand_module];
    
    return result;
}

authenticator::authenticator()
{
    // TODO: upload tokens, phones and user db from the database
    // TODO: while uploading user db from database keep in mind that
    // we need to store it in a vector, so for each non existing entry we
    // need to have a "hole" in the vector. Which is OK, because it's very unlikely
    // that a user is deleted. If a user is deleted then the hole will contain a user
    // with valid_ == false
}

bool authenticator::send_smscode(const std::string &phone_number)
{
    // Generate random code of 4 chars
    std::string smscode = generate_random_alpha_numeric_string(10, 4);
    
    // TODO: implement
    // send sms code through API
    
    // TODO: multithreading - sync!!!
    
    // Save smscode & phone_number
    // Note: smscode expires in 180 seconds
    smscodes_.insert({smscode + phone_number, time(NULL) + 180});
    
    std::cerr << "authenticator::send_smscode debug, smscode=" << smscode << ", phone_number=" << phone_number << std::endl;
    
    return true;
}

authenticator::auth_result authenticator::authenticate_by_smscode(const std::string &phone_number, const std::string &smscode)
{
    // TODO: multithreading - sync!!!

    // Check existence of smscode + phone
    auto s_i = smscodes_.find(smscode + phone_number);
    if (s_i == smscodes_.end())
    {
        return {"", -1, user(false)};
    }
    
    // Check the smscode has not expired
    if (s_i->second < time(NULL))
    {
        smscodes_.erase(s_i);
        return {"", -1, user(false)};
    }
    
     // Delete the smscode to avoid the second authentication by the same smscode
     smscodes_.erase(s_i);
    
    // Get user_id by phone_number
    auto i = phones_.find(phone_number);
    user u(true);
    long long user_id;
    
    // Found a phone connected with a user
    if (i != phones_.end())
    {
        user_id = i->second;
        
        // Check if user_id is OK
        // Note: it normally can not be not ok
        if (!(u = get_user_db_entry(user_id).is_valid()).is_valid())
        {
            std::cerr << "authenticator::authenticate_by_smscode: ERROR: data is corrupted, phone_number=" <<
                phone_number << ", user_id=" << user_id << ", user_db size=" << user_db_.size() << std::endl;
            return {"", -1, user(false)};
        }
    }
    else
    {
        // Fresh new phone - create a new valid user entry for it
        u.phone_number_ = phone_number;
        user_db_.push_back(u);
        user_id = user_db_.size()-1;
        
        // Update phone -> user_id index
        phones_.insert({phone_number, user_id});
        
        // TODO: insert a user info database!!!
    }
        
    // Create a unique token
    std::string token;
    while (true)
    {
        srand(time(NULL));
        token = generate_random_alpha_numeric_string(36, 32);

        if (tokens_.find(token) == tokens_.end())
            break;
        
        // Bad random :-)
        std::cerr << "authenticator::authenticate_by_smscode: bad random! Token " << token << " repeated!!!" << std::endl;
    }
    
    // Save token -> user_id
    // Note: tokens never expire because it's very useful for mobile apps
    tokens_[token] = user_id;
    
    // TODO: update database with the phone and with the token
    // it's wise to store datetime of token in a db - just in case

    return {token, user_id, u};
}

authenticator::auth_result authenticator::authenticate_by_token(const std::string &token)
{
    auto it = tokens_.find(token);
    if (it == tokens_.end())
        return {token, -1, user(false)};
    
    user u = get_user_db_entry(it->second);
    if (!u.is_valid())
        return {token, -1, u};
    else
        return {token, it->second, u};
}

}
