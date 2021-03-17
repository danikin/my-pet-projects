/*
*       marketplace_api.cpp
*
*       (C) Denis Anikin 2020
*
*       Api impl for marketplace
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
#include "marketplace_dumper.h"

namespace marketplace
{

void marketplace_api::update_driver_position(long long driver_id, point position)
{
    // This method is called VERY FREQUENTLY, so we need to store EVERYTHING it touches
    // in RAM and all O(N)+ algorighms MUST be amortised to O(1) (like marketplace rebalance and
    // notification sending - happen no frequently than once a second)
    
    time_t now = time(NULL);
    
    // We only update driver position in the marketplace if she/he is NOT WITH THE RIDER
    // Why? Because this way a driver will be removed from the marketplace which is good
    // because we don't want the marketplace rely on this driver until she/he finished a ride
    if (!is_driver_on_the_way_from_A_to_B(driver_id))
        impl_.update_driver_position(driver_id, position, now);
    
    // Update tracks (it's totally in RAM)
    driver_tracks_.update_object_position(driver_id, position, now);

    // TODO: update_driver_position is called often and notifications can really create
    // a broadcast storm. We need to think how to fix it
    send_notifications_and_commit();
}

long long marketplace_api::price_view(long long rider_id, point A, const std::string &addr_A,
                       point B, const std::string &addr_B)
{
    // TODO: implement INSERT price view into DB and then use its id
    
    long long pv_id = rand();//save_price_view_in_db(...);
    if (pv_id == -1)
        return -1;
    
    // Add a price view to the marketplace
    if (!impl_.add_price_view(order(pv_id, rider_id, A, B), time(NULL)))
        return -1;
    
    // Send notifications in case if they're already there
    send_notifications_and_commit();

    return pv_id;
}

bool marketplace_api::update_price_view_A(long long rider_id, long long pv_id, point A)
{
    // TODO: implement UPDATE of DB

    // Add a price view to the marketplace
    if (!impl_.update_price_view_A(rider_id, pv_id, A, time(NULL)))
        return false;
    
    // Send notifications in case if they're already there
    send_notifications_and_commit();

    return true;
}

bool marketplace_api::update_price_view_B(long long rider_id, long long pv_id, point B)
{
    // TODO: implement UPDATE of DB

    // Add a price view to the marketplace
    if (!impl_.update_price_view_B(rider_id, pv_id, B, time(NULL)))
        return false;
    
    // Send notifications in case if they're already there
    send_notifications_and_commit();

    return true;
}
  
bool marketplace_api::place_order(long long rider_id, long long pv_id)
{
    // Place the order
    bool is_ok = impl_.convert_price_view_to_order(rider_id, pv_id, time(NULL));
    if (!is_ok)
        return false;
    
    send_notifications_and_commit();
    
    return is_ok;
}
    
bool marketplace_api::increase_unassigned_order_price(long long rider_id, double new_price)
{
    // TODO: implement
    // TODO: check if the price actually INCREASES :-) Also check that it increases for not too much

    send_notifications_and_commit();
    
    return false;
}

bool marketplace_api::cancel_order(long long rider_id, long long driver_id)
{
    // TODO: update DB
    // TODO: implement:
    //  1.  Find the order in unassigned orders (by rider_id) and remove it
    //  2.  Find the order in assigned orders (by rider_id or by rider_id) and remove it
    
    //bool is_ok = impl_.remove_unassigned_order(order_id, time(NULL));

    send_notifications_and_commit();
    
    return false;
}
  
bool marketplace_api::accept_order(long long driver_id, long long order_id)
{
    // The order must be unassigned at the moment - so find the entry in the marketplace and get it
    order ord;
    time_t now = time(NULL);
    if (!impl_.remove_unassigned_order(order_id, now, &ord))
        return false;
    
    // Add the unassigned order to the list of assigned
    assigned_orders_impl_.add_assigned_order(order_id, ord.id_, driver_id);
    
    // Now that the driver has accepted the order it's a good time to start
    // measuring RTA to A
    // The call to this function will reset the route length and the time in the tracker
    // So it will start a new measuring of route length and time
    driver_tracks_.get_recent_track_info(driver_id);
    
    send_notifications_and_commit();
    
    return true;
}
  
bool marketplace_api::decline_order(long long driver_id, long long order_id)
{
    if (!assigned_orders_impl_.remove_assigned_order(driver_id, order_id))
        return false;
    
    send_notifications_and_commit();
    
    return false;
}
  
bool marketplace_api::change_assigned_order_status(long long driver_id, enum assigned_order_status new_status)
{
    // Search for an assigned order for this driver
    long long order_id = assigned_orders_impl_.driver_id_to_assigned_order_id(driver_id);
    if (order_id == -1)
        return false;
    
    // Change the status
    if (!assigned_orders_impl_.change_assigned_order_status(order_id, new_status))
        return false;
    
    // Now that the driver started going to B - reset track info
    if (new_status == assigned_order_status_go_to_B)
        driver_tracks_.get_recent_track_info(driver_id);
    else
    // Now that the driver just got A or just got B we can update RTA
    if (new_status == assigned_order_status_wait_at_A || new_status == assigned_order_status_payment_at_B)
    {
        // lt will have started time and position of the recent piece of the track + length of
        // that piece in km
        /*object_tracks::length_time lt = */driver_tracks_.get_recent_track_info(driver_id);
        
        // TODO: save to DB this:
        // This will help determine ETA in a future using existing RTAs
        // Especially it could be useful that we differentiate RTAs in toA and AtoB
        // {driver_id, "toA"/"AtoB", dt <- now, when_started <- lt.when_started_, where_started <- lt.where_started}
    }
    
    send_notifications_and_commit();
    
    return true;
}

bool marketplace_api::chat_other_party(long long rider_id, long long driver_id, const std::string &message)
{
    if (rider_id != -1)
        return chat_from_rider(rider_id, message);
    else
    if (driver_id != -1)
        return chat_from_driver(driver_id, message);
    else
        return false;
}

bool marketplace_api::get_chat_history(long long rider_id, long long driver_id,
                                       std::vector<assigned_order::chat_entry> &history)
{
    assigned_order ao = assigned_orders_impl_.rider_id_to_assigned_order(rider_id);
    if (!ao.is_valid())
    {
        ao = assigned_orders_impl_.driver_id_to_assigned_order(driver_id);
        if (!ao.is_valid())
            return false;
    }
    
    history = ao.chat_;
    
    return true;
}

bool marketplace_api::chat_from_rider(long long rider_id, const std::string &message)
{
    // Note: we don't need to update DB
    
    long long order_id = assigned_orders_impl_.rider_id_to_assigned_order_id(rider_id);
    if (order_id == -1)
        return false;
    
    return assigned_orders_impl_.chat_other_party(order_id, message, true);
}

bool marketplace_api::chat_from_driver(long long driver_id, const std::string &message)
{
    long long order_id = assigned_orders_impl_.driver_id_to_assigned_order_id(driver_id);
    if (order_id == -1)
        return false;

    return assigned_orders_impl_.chat_other_party(order_id, message, false);
}

/*bool marketplace_api::driver_message(long long driver_id, enum driver_message_type message)
{
    // TODO: implement
    
    send_notifications();
    
    return false;
}
  
enum driver_message_type marketplace_api::get_driver_last_message(long long driver_id)
{
    // TODO: implement
    
    return driver_message_no_message;
}*/
  
long long marketplace_api::get_rider_id_by_driver(long long driver_id)
{
    // Note if there is no rider for a driver then the impl will return -1
    return assigned_orders_impl_.driver_id_to_assigned_order(driver_id).rider_id_;
}

bool marketplace_api::is_driver_on_the_way_from_A_to_B(long long driver_id)
{
    assigned_order ao = assigned_orders_impl_.driver_id_to_assigned_order(driver_id);
    if (!ao.is_valid())
        return false;

    return ao.status_ == assigned_order_status_go_to_B;
}

marketplace_api::marketplace_api(const char *mysql_url,
                                 fifo_interface::multicastor &rider_multicastor,
                                 fifo_interface::multicastor &driver_multicastor) :
    impl_(true, true, true, true, true),
    rider_multicastor_(rider_multicastor),
    driver_multicastor_(driver_multicastor)
{
    std::cout << "marketplace_api::marketplace_api: uploading marketplace data" << std::endl;
    // TODO : upload data from DB
    // Data to upload:
    //  1.  user_db_
}

void marketplace_api::commit_changes()
{
    // TODO: implement
}

void add_rider_notification_to_json(rider_notification &r, std::string &json_result)
{
    json_result += "{\"rider_id\":" + std::to_string(r.rider_id_) + "\",";
    switch (r.object_type_)
    {
        case rider_notification::rider_notification_view_price_change:
            json_result += "\"type\":\"view_price_change\",\"view_id\":\"" + std::to_string(r.id_);
            break;
        case rider_notification::rider_notification_order_price_change_desire:
            json_result += "\"type\":\"order_price_change_des\",\"order_id\":\"" + std::to_string(r.id_);
            break;
        case rider_notification::rider_notification_order_price_change_positive:
            json_result += "\"type\":\"order_price_change_pstv\",\"order_id\":\"" + std::to_string(r.id_);
            break;
        default:
            break;
    }

    // Change the price for a price view
    // Or ask the rider if we could change the price for a placed order
    json_result
    += "\",\"old_p\":\"" + std::to_string(r.old_price_) + "\""
    +  "\",\"new_p\":\"" + std::to_string(r.new_price_) + "\""
    + "\",\"old_m\":\"" + std::to_string(r.old_metric_value_) + "\""
    +  "\",\"new_m\":\"" + std::to_string(r.new_metric_value_) + "\""
    +  "\",\"new_drv\":\"" + std::to_string(r.new_driver_id_) + "\"";

    json_result += "}";
}

void add_driver_notification_to_json(driver_notification &d, std::string &json_result)
{
    json_result += "{\"type\":\"new_top_order\",\"driver_id\":" + std::to_string(d.driver_id_)
        + "\",\"order_id\":\"" + std::to_string(d.id_)
        + "\",\"order_desire\":\"" + std::to_string(d.top_order_desirability_)
        + "\",\"order_metric\":\"" + std::to_string(d.top_order_metric_value_);

    json_result += "\"}";
}

void add_ao_notification_to_json(assigned_order_notification &a_on, std::string &json_result)
{
    json_result += "{\"type\":\"" + std::to_string((int)a_on.type_)
        + "\",\"rider_id\":" + std::to_string(a_on.new_order_.rider_id_)
        + "\",\"driver_id\":" + std::to_string(a_on.new_order_.driver_id_)
        + "\",\"new_rider_message\":\"" + a_on.new_rider_message_
        + "\",\"new_driver_message\":\"" + a_on.new_driver_message_
    ;

    json_result += "\"}";
}

void marketplace_api::send_notifications_and_commit()
{
    // Optimization for a popular case :-)
    if (!impl_.is_any_notification() && !assigned_orders_impl_.is_any_notification())
        return;
    
    // Notify end users about all pending changes
    std::list<rider_notification> rn;
    std::list<driver_notification> dn;
    std::list<assigned_order_notification> ao_n;
    
    impl_.get_notification_lists(rn, dn);
    assigned_orders_impl_.get_notification_list(ao_n);
    
    send_notifications(rn, dn, ao_n);
    
    // Save all pending changes in DB
    // TODO: ^^^
    
    // Clear pending changes
    impl_.clear_notifications();
    assigned_orders_impl_.clear_notifications();
}

void marketplace_api::send_notifications(std::list<rider_notification> &rn,
                                         std::list<driver_notification> &dn,
                                         std::list<assigned_order_notification> &ao_n)
{
    // Fast access to all notifications per one rider and per one driver
    std::unordered_map<long long, std::string> notifications_by_rider;
    std::unordered_map<long long, std::string> notifications_by_driver;

    // UNASSIGNED ORDERS:
    // Iterate all rider notifications and assemble a single json for each rider in a hash map
    // Iterate all driver notifications and assemble a single json for each driver in a hash map
    for (auto &r : rn)
    {
        std::string &j = notifications_by_rider[r.rider_id_];
        add_rider_notification_to_json(r, j);
        j += ",";
    }
    for (auto &d : dn)
    {
        std::string &j = notifications_by_driver[d.driver_id_];
        add_driver_notification_to_json(d, j);
        j += ",";
    }
    
    // Note: for assigned orders we send absolutely the same notifications for drivers and for riders
    // ASSIGNED ORDERS:
    for (auto &ao : ao_n)
    {
        std::string &j = notifications_by_rider[ao.new_order_.rider_id_];
        add_ao_notification_to_json(ao, j);
        j += ",";
    }
    for (auto &ao : ao_n)
    {
        std::string &j = notifications_by_driver[ao.new_order_.driver_id_];
        add_ao_notification_to_json(ao, j);
        j += ",";
    }

    // Now send each rider a single bunch
    // It's O(N * Log M), N - number of riders to notify, M - number of agents
    // In a future it would be wise to change rider_id_to_pid_ from multimap to unordered_map with vector inside
    // That would give just O(N)
    //

    // TODO: multicastor can block on hanging agents! Consider separate thread for this. But without race conditions :-)

    // Send this bunch to each rider
    for (auto &r : notifications_by_rider)
        rider_multicastor_.multicast(r.first, "{\"event\":\"rider_notifications'\",\"data\":[" + r.second + "]}");
    
    // Send this bunch to each driver
    for (auto &d : notifications_by_driver)
        driver_multicastor_.multicast(d.first, "{\"event\":\"driver_notifications'\",\"data\":[" + d.second + "]}");
    
    // TODO: important! If some notifications MUST BE DELIVERED even if a user is not in the app through push
    // notifications then we MUST filter them and sent them
    // So we will need a flag for each notification - is it ok to send it for an offline user
}

/*
void marketplace_api::marketplace_to_json(json &result)
{
    dumper::to_json(impl_, result);
}*/

void marketplace_api::driver_tracks_to_json(nlohmann::json &result)
{
    driver_tracks_.to_json(result);
}

void marketplace_api::rider_multicastor_to_json(nlohmann::json &result)
{
    rider_multicastor_.to_json(result);
}

void marketplace_api::driver_multicastor_to_json(nlohmann::json &result)
{
    driver_multicastor_.to_json(result);
}


} // namespace marketplace
