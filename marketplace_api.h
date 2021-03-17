/*
*       marketplace_api.h
*
*       (C) Denis Anikin 2020
*
*       Headers for api impl for marketplace
*
*/

#ifndef marketplace_api_h
#define marketplace_api_h

#include "nlohmann/json.hpp"

#include "fifo_interface.h"
#include "object_track.h"

#include "marketplace.h"
#include "marketplace_assigned_order.h"

namespace marketplace
{

class marketplace_api
{
public:

    // mysql_url    -   points to the main MySQL database
    // rider_multicastor, driver_multicastor  -
    //  references to objects that are responsible for multicasting
    //  messages to end users (mobile apps) of a rider and a driver. It's needed
    //  because marketplace asynchronously notifies end users about changes
	marketplace_api(const char *mysql_url,
                        fifo_interface::multicastor &rider_multicastor,
                        fifo_interface::multicastor &driver_multicastor);
    

    void update_driver_position(long long driver_id, point position);
    
    // Requests a price for a ride from A to B for a specific rider
    // The result is a price view id
    // The actual price will be delivered to the end user through the web socket
    long long price_view(long long rider_id, point A, const std::string &addr_A,
                         point B, const std::string &addr_B);

    // Updates price_view's coordiates
    bool update_price_view_A(long long rider_id, long long pv_id, point A);
    bool update_price_view_B(long long rider_id, long long pv_id, point B);

    // Places an order from price view pv_id
    // Returns true if the order has been placed
    bool place_order(long long rider_id, long long pv_id);

    // Increases current active order's price for rider_id
    // Returns true if price has actually increased
    // Note: it only works for unassigned orders
    bool increase_unassigned_order_price(long long rider_id, double new_price);
    
    // Cancels current active rider_id's or driver's order
    // Returns true if the rider or the rider had an assigned order and it was really cancelled
    bool cancel_order(long long rider_id, long long driver_id);
    
    // Accepts the unassigned order by the driver
    // Returns true if the order has been accepted
    bool accept_order(long long driver_id, long long order_id);

    // Declines the already ASSIGNED order by the driver
    // Returns true if the order has been declined
    bool decline_order(long long driver_id, long long order_id);

    // Send a text message to the other party (driver to rider or rider to driver)
    bool chat_other_party(long long rider_id, long long driver_id, const std::string &message);
    bool chat_from_rider(long long rider_id, const std::string &message);
    bool chat_from_driver(long long driver_id, const std::string &message);
    
    // Returns a chat history with the other party (if there is any)
    bool get_chat_history(long long rider_id, long long driver_id,
                          std::vector<assigned_order::chat_entry> &history);

//    // Sends a message from the driver to the rider that's with the same order
    
    // Changes the order status
    // Note: the status can be changed only for the very next one
    bool change_assigned_order_status(long long driver_id, enum assigned_order_status new_status);
    
 //   enum driver_message_type get_driver_last_message(long long driver_id);
    
    // If driver_id has accepted an order then it returns its rider_id
    // Otherwise returns -1
    long long get_rider_id_by_driver(long long driver_id);
    
    bool is_driver_on_the_way_from_A_to_B(long long driver_id);
    
    // Dump internals
    //void marketplace_to_json(nlohmann::json &result);
    void assigned_orders_to_json(nlohmann::json &result) { assigned_orders_impl_.assigned_orders_to_json(result); }
    void assigned_orders_ro_map_to_json(nlohmann::json &result) { assigned_orders_impl_.ro_map_to_json(result); }
    void assigned_orders_do_map_to_json(nlohmann::json &result) { assigned_orders_impl_.do_map_to_json(result); }
    void assigned_orders_notifications_to_json(nlohmann::json &result)
    {
        assigned_orders_impl_.notifications_to_json(result);
    }
    void driver_tracks_to_json(nlohmann::json &result);
    void rider_multicastor_to_json(nlohmann::json &result);
    void driver_multicastor_to_json(nlohmann::json &result);

 private:

    marketplace_api(const marketplace_api&);
    marketplace_api &operator=(const marketplace_api&);
    
    // Notifies local ws agents about changes passed as arguments
    // Don't worry to call it often - notifications are formed once a second in the impl_ :-)
    void send_notifications(std::list<rider_notification> &rn,
                                std::list<driver_notification> &dn,
                                std::list<assigned_order_notification> &ao_n);

    
    // 1. Sends all pending notifications
    // 2. Commites them in DB
    // 3. Clears them
    void send_notifications_and_commit();
    
	void commit_changes();
    
    // The marketplace itsef - all the algorithms are inside :-)
	marketplace impl_;
    
    // Assigned orders
    assigned_orders assigned_orders_impl_;
    
    // Driver tracks
    object_tracks driver_tracks_;
    
    // For sending async messages to end users
    fifo_interface::multicastor &rider_multicastor_, &driver_multicastor_;
};

// Adds a rider notification to a json_string
void add_rider_notification_to_json(rider_notification &r, std::string &json_result);

// Adds a driver notification to a json_string
void add_driver_notification_to_json(driver_notification &d, std::string &json_result);

}

#endif

