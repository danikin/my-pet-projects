/*
*       marketplace_assigned_order.h
*
*       (C) Denis Anikin 2020
*
*       Headers for impl for assigned orders
*
*/

#ifndef marketplace_assigned_order_h_
#define marketplace_assigned_order_h_

#include "nlohmann/json.hpp"

#include "marketplace.h"

namespace marketplace
{

// Possible standard messages from a driver to a rider
enum driver_message_type
{
    driver_message_no_message,
    driver_message_will_be_later_before_A,
    driver_message_cancel_before_A,
    driver_message_at_A,
    driver_message_payed_waiting_started_at_A,
    driver_message_free_waiting_ended_at_A,
    driver_message_on_way_to_B,
    driver_message_at_B,
    driver_message_order_closed
};

enum assigned_order_status
{
    // On the way to the pickup point
    assigned_order_status_go_to_A,
    
    // Waiting at the pickup point
    assigned_order_status_wait_at_A,
    
    // Toll waiting at the pickup point
    assigned_order_status_toll_wait_at_A,
    
    // On the way to dropoff point
    assigned_order_status_go_to_B,
    
    // Waiting for the payment at B
    assigned_order_status_payment_at_B
};

class assigned_order
{
public:
    
    assigned_order() : assigned_order(-1, -1) {}
    assigned_order(long long rider_id, long long driver_id) :
        rider_id_(rider_id), driver_id_(driver_id), status_(assigned_order_status_go_to_A) {}
    
    bool is_valid() { return rider_id_ != -1 && driver_id_ != -1; }

    // Who placed the order
    long long rider_id_;
    
    // Who accepted the order
    long long driver_id_;
    
    // Chat between rider and driver
    struct chat_entry {std::string d_message_, r_message_;};
    std::vector<chat_entry> chat_;

    // Current order status
    enum assigned_order_status status_;
};

enum assigned_orders_rider_notification_type
{
    // An order is assigned to a driver
    assigned_orders_rider_notification_order_assigned,
    
    // An order is no assigned to a driver anymore
    assigned_orders_rider_notification_order_unassigned,
    
    // An order has been advanced to the next status
    assigned_orders_rider_notification_order_status_changed,
    
    // A notified party has been messaged from the other party
    assigned_orders_rider_notification_order_message_from_other_party,
};

// Notification about a change in the assigned order
class assigned_order_notification
{
public:

    // Type
    assigned_orders_rider_notification_type type_;
    
    // New or changed order
    assigned_order new_order_;
    
    // Its id
    long long order_id_;
    
    // New message
    std::string new_rider_message_;
    std::string new_driver_message_;
};

// Assigned orders
class assigned_orders
{
public:
    
    assigned_orders();
    
    /*
     *      Operations for assigned orders. Each operation saves all the changes in json
     *      assuming that it is the next array entry
     */
    
    
    // Adds an assigned order
    bool add_assigned_order(long long order_id, long long rider_id, long long driver_id);
    
    // Removes an assigned order
    bool remove_assigned_order(long long driver_id, long long order_id);
    
    // Changes status of an assigned order
    bool change_assigned_order_status(long long order_id, assigned_order_status new_status);
    
    // Sends a message to the other party
    bool chat_other_party(long long order_id, const std::string &message, bool is_originator_rider);
    
    assigned_order rider_id_to_assigned_order(long long rider_id)
    {
        return id_to_assigned_order_helper_(rider_id_to_order_id_, rider_id);
    }

    assigned_order driver_id_to_assigned_order(long long driver_id)
    {
        return id_to_assigned_order_helper_(driver_id_to_order_id_, driver_id);
    }
    
    long long rider_id_to_assigned_order_id(long long rider_id)
    {
        return id_to_assigned_order_id_helper_(rider_id_to_order_id_, rider_id);
    }

    long long driver_id_to_assigned_order_id(long long driver_id)
    {
        return id_to_assigned_order_id_helper_(driver_id_to_order_id_, driver_id);
    }

    // Returns notifications about recent changed
    // It's OK to copy data because a) it's once in a while b) it's like 10-30K for even big towns
    void get_notification_list(std::list<assigned_order_notification> &a_on)
    {
        a_on = notifications_;
    }
    
    bool is_any_notification()
    {
        return !notifications_.empty();
    }

    // Clears all the notifications
    // The caller MUST be sure that everything was sent before calling this method
    void clear_notifications()
    {
        notifications_.clear();
    }
    
    // Dumps to json
    void assigned_orders_to_json(nlohmann::json &result);
    void ro_map_to_json(nlohmann::json &result);
    void do_map_to_json(nlohmann::json &result);
    void notifications_to_json(nlohmann::json &result);

private:

    template <class id_to_id_hash>
    long long id_to_assigned_order_id_helper_(id_to_id_hash &h_, long long id)
    {
        // Find id in a map to order_id
        auto i = h_.find(id);
        if (i == h_.end())
            return -1;
        else
            return i->second;
    }
    
    template <class id_to_id_hash>
    assigned_order id_to_assigned_order_helper_(id_to_id_hash &h_, long long id)
    {
        // Get the order id
        long long order_id = id_to_assigned_order_id_helper_(h_, id);
        
        // Search it in an index
        auto ao = assigned_orders_.find(order_id);
        if (ao == assigned_orders_.end())
            return assigned_order();
        else
            return ao->second;
    }
    
    // order_id -> assigned_order
    std::unordered_map<long long, assigned_order> assigned_orders_;
    
    // rider_id -> assigned order id
    std::unordered_map<long long, long long> rider_id_to_order_id_;
    // driver_id -> assigned order id
    std::unordered_map<long long, long long> driver_id_to_order_id_;
    
    // Notifications for riders and drivers about changes in assigned orders
    // Note: there is no difference between riders and drivers because order statuses and messages
    // are fully transparent for both parties
    std::list<assigned_order_notification> notifications_;
};

}

#endif
