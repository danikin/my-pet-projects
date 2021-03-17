/*
*       marketplace_assigned_order.cpp
*
*       (C) Denis Anikin 2020
*
*       Api impl for assigned orders
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
#include "marketplace_api.h"

namespace marketplace
{

assigned_orders::assigned_orders()
{
    // TODO: upload assigned orders from DB
    // TODO: notifications MUST BE TURN OFF during upload
}

bool assigned_orders::add_assigned_order(long long order_id, long long rider_id, long long driver_id)
{
     // Check if the order has been already added
     if (assigned_orders_.find(order_id) != assigned_orders_.end())
         return false;
     
     // Check if the rider already has a placed and assigned order
     if (rider_id_to_order_id_.find(rider_id) != rider_id_to_order_id_.end())
         return false;

     // Check if the driver already has an assigned order
     if (driver_id_to_order_id_.find(driver_id) != driver_id_to_order_id_.end())
         return false;

     // Create an entry in active order database
     assigned_order &new_order = assigned_orders_[order_id] = assigned_order(rider_id, driver_id);
     
     // Create entries in indexes
     rider_id_to_order_id_[rider_id] = order_id;
     driver_id_to_order_id_[driver_id] = order_id;
    
    // Notify end users about the order being accepted
    notifications_.push_back({
        .type_ = assigned_orders_rider_notification_order_assigned,
        .new_order_ = new_order,
        .order_id_ = order_id,
        .new_rider_message_ = "",
        .new_driver_message_ = ""
    });

     return true;
}

bool assigned_orders::remove_assigned_order(long long driver_id, long long order_id)
{
    if (driver_id == -1 || order_id == -1)
        return false;
    
    // Check if the driver already has an assigned order
    if (driver_id_to_order_id_.find(driver_id) != driver_id_to_order_id_.end())
        return false;
    
    // Check if this order is actually assigned to the driver
    if (driver_id_to_order_id_[driver_id] != order_id)
        return false;
    
    // Check if this order exists in the order db
    auto i = assigned_orders_.find(order_id);
    if (i == assigned_orders_.end())
        return false;
        
    // Take the order as object
    assigned_order old_order = i->second;
     
    // Remove an order from the database
    assigned_orders_.erase(i);
     
     // Note: we don't do expensive cleanups of indexes - dangling entries
     // will be recreated at the assignment of the next order to the rider or the driver
    
    // Notify end users about a previously assigned/accepted order being declined by a driver
    notifications_.push_back({
        .type_ = assigned_orders_rider_notification_order_unassigned,
        .new_order_ = old_order,
        .order_id_ = order_id,
        .new_rider_message_ = "",
        .new_driver_message_ = ""
    });
     
     return true;
 }
 
bool assigned_orders::change_assigned_order_status(long long order_id, assigned_order_status new_status)
{
     auto i = assigned_orders_.find(order_id);
     if (i == assigned_orders_.end())
         return false;
     
     // Status can only be changed to the next one
    if ((int)new_status != (int)i->second.status_ + 1)
         return false;
    i->second.status_ = new_status;
     
    // Notify end users about changed status
    notifications_.push_back({
        .type_ = assigned_orders_rider_notification_order_status_changed,
        .new_order_ = i->second,
        .order_id_ = order_id,
        .new_rider_message_ = "",
        .new_driver_message_ = ""
    });
    
     return true;
}

// Sends a message to the other party
bool assigned_orders::chat_other_party(long long order_id, const std::string &message, bool is_originator_rider)
{
    auto i = assigned_orders_.find(order_id);
    if (i == assigned_orders_.end())
        return false;
    
    // Save the message in the order
    if (is_originator_rider)
        i->second.chat_.push_back({.d_message_ = "", .r_message_ = message});
    else
        i->second.chat_.push_back({.d_message_ = message, .r_message_ = ""});
    
    // Notify end users about a new message in a chat
    notifications_.push_back({
        .type_ = assigned_orders_rider_notification_order_status_changed,
        .new_order_ = i->second,
        .order_id_ = order_id,
        .new_rider_message_ = is_originator_rider ? message : "",
        .new_driver_message_ = is_originator_rider ? "" : message
    });
    
    return true;
}

void assigned_orders::assigned_orders_to_json(nlohmann::json &result)
{
    // Assigned orders
    result["event"] = "dump_assigned_orders";
    for (auto &u : assigned_orders_)
    {
        result["data"].push_back({
            {"order_id", u.first},
            {"rider_id", u.second.rider_id_},
            {"driver_id", u.second.driver_id_},
            {"status", (int)u.second.status_}
        });
        for (auto &c : u.second.chat_)
            result["data"][result["data"].size()-1]["chat"].push_back({
                {"r_message",c.r_message_},
                {"d_message",c.d_message_},
            });
    }

    /*
    for (auto &r : rider_id_to_order_id_)
    // rider_id -> assigned order id
    std::unordered_map<long long, long long> rider_id_to_order_id_;
    // driver_id -> assigned order id
    std::unordered_map<long long, long long> driver_id_to_order_id_;
    
    // Notifications for riders and drivers about changes in assigned orders
    // Note: there is no difference between riders and drivers because order statuses and messages
    // are fully transparent for both parties
    std::list<assigned_order_notification> notifications_;*/
}

void assigned_orders::ro_map_to_json(nlohmann::json &result)
{
    result["event"] = "dump_assigned_orders_ro_map";
    for (auto &r : rider_id_to_order_id_)
    {
        result["data"].push_back({
            {"rider_id", r.first},
            {"order_id", r.second}
        });
    }
}

void assigned_orders::do_map_to_json(nlohmann::json &result)
{
    result["event"] = "dump_assigned_orders_do_map";
    for (auto &d : driver_id_to_order_id_)
    {
        result["data"].push_back({
            {"driver_id", d.first},
            {"order_id", d.second}
        });
    }
}

void assigned_orders::notifications_to_json(nlohmann::json &result)
{
    result["event"] = "dump_assigned_orders_notifications";
    for (auto &n : notifications_)
    {
        result["data"].push_back({
            {"type", (int)n.type_},
            {"new_order_status", (int)n.new_order_.status_},
            {"order_id", n.order_id_},
            {"rider_id", n.new_order_.rider_id_},
            {"driver_id", n.new_order_.driver_id_},
            {"new_rider_message_", n.new_rider_message_},
            {"new_driver_message_", n.new_driver_message_}
        });
    }
}

} // namespace marketplace
