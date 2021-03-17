/*
*    marketplace_dumper.cpp
*
*    (C) Denis Anikin 2020
*
*    Dumper for marketplace impl
*
*/

#include "marketplace_dumper.h"
//#include "md5.h"

namespace dumper
{

void set_dsf_helper(json &element,marketplace::driver &d)
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

void send_marketplace_message_by_message(marketplace::marketplace &m, std::ostream &os)
{
    std::unordered_map<long long, marketplace::order> price_views;
    std::unordered_map<long long, marketplace::order> unassigned_orders;
    std::unordered_map<long long, marketplace::driver> unassigned_drivers;
    std::unordered_map<long long, marketplace::order> assigned_orders;
    std::unordered_map<long long, marketplace::driver> assigned_drivers;

    m.get_price_views(price_views);
    m.get_unassigned_orders(unassigned_orders);
    m.get_unassigned_drivers(unassigned_drivers);
    m.get_assigned_orders(assigned_orders);
    m.get_assigned_drivers(assigned_drivers);

    // Price views
    for (auto &pv_i : price_views)
    {
        json result;
        result["event"] = "test_everything";
        
        auto &pv = pv_i.second;
        result["data"]["riders"].push_back({
                     {"rider_id", pv.rider_id_},
                     {"price_view_id", pv.id_},
                     {"latA", pv.A_.latitude},
                     {"lonA", pv.A_.longitude},
                     {"latB", pv.B_.latitude},
                     {"lonB", pv.B_.longitude},
                     {"dt_added", pv.dt_added_},
                     {"expires_in_seconds", (pv.dt_added_ + marketplace::params::price_view_expiration_time()) - time(NULL)},
                     {"status", "price_view"},
                     {"price", pv.price_},
                     {"best_pickup_ETA", pv.best_ETA_min_},
                     {"best_pickup_distance", pv.best_pickup_route_length_km_},
                    {"best_driver_id", pv.best_driver_id_}
        });
        
        os << result << std::endl;
    }
 
    // Unassigned orers
    for (auto &o_i : unassigned_orders)
    {
        json result;
        result["event"] = "test_everything";
        
        auto &o = o_i.second;
        result["data"]["riders"].push_back({
                      {"rider_id", o.rider_id_},
                    {"unassigned_order_id", o.id_},
                      {"latA", o.A_.latitude},
                      {"lonA", o.A_.longitude},
                      {"latB", o.B_.latitude},
                      {"lonB", o.B_.longitude},
                      {"dt_added", o.dt_added_},
                      {"expires_in_seconds", (o.dt_added_ + marketplace::params::unassigned_order_expiration_time()) - time(NULL)},
                      {"status", "order"},
                      {"price", o.price_},
                      {"suggested_price", o.suggested_price_},
                     {"best_pickup_ETA", o.best_ETA_min_},
                     {"best_pickup_distance", o.best_pickup_route_length_km_},
                     {"best_driver_id", o.best_driver_id_}
                  });
        
        os << result << std::endl;
    }
    
    // Unassigned drivers
    for (auto &d_i : unassigned_drivers)
    {
        json result;
        result["event"] = "test_everything";
        
        auto &d = d_i.second;
        result["data"]["drivers"].push_back({
                       {"status", "unassigned_driver"},
                        {"driver_id", d.id_},
                        {"lat", (double)d.position_where_.latitude},
                        {"lon", (double)d.position_where_.longitude},
                        {"dt_coords_updated", d.position_when_},
                        {"expires_in_seconds", (d.position_when_ + marketplace::params::position_freshness_time()) - time(NULL)},
                        {"best_rider_id", d.top_order_.rider_id_},
                        {"best_order_desirability", d.top_order_.desirability_},
                        {"best_order_metric", d.top_order_metric_value_}
                    });

        // Driver side filters
        set_dsf_helper(result["data"]["drivers"][result["data"]["drivers"].size()-1], d);
        
        os << result << std::endl;
        
    } // for (auto &d_i : unassigned_drivers)
    
    // Assigned orders
    for (auto &o_i : assigned_orders)
    {
        json result;
        result["event"] = "test_everything";
        
        auto &o = o_i.second;
        result["data"]["riders"].push_back({
                 {"rider_id", o.rider_id_},
                {"assigned_order_id", o.id_},
                {"assigned_driver_id", o.assigned_driver_id_},
             {"ts_driver_at_A", o.ts_driver_at_A_},
             {"ts_ride_started", o.ts_ride_started_},
             {"price_for_toll_wait", o.price_for_toll_wait_},
                {"latA", o.A_.latitude},
                 {"lonA", o.A_.longitude},
                 {"latB", o.B_.latitude},
                 {"lonB", o.B_.longitude},
                 {"dt_added", o.dt_added_},
                 {"expires_in_seconds", (o.dt_added_ + marketplace::params::unassigned_order_expiration_time()) - time(NULL)},
                 {"status", "assigned_order"},
                 {"price", o.price_},
                 {"suggested_price", o.suggested_price_},
                {"best_pickup_ETA", o.best_ETA_min_},
                {"best_pickup_distance", o.best_pickup_route_length_km_},
                {"best_driver_id", o.best_driver_id_}
             });

        // Order options (rider side filters)
        json &element = result["data"]["riders"][result["data"]["riders"].size()-1];
        if (!o.rsf_.pass_everything_ && o.rsf_.no_smoking_) element["rsf_no_smoking"]=1;
        if (!o.rsf_.pass_everything_ && o.rsf_.need_childseat_) element["rsf_need_childseat"]=1;
        if (!o.rsf_.pass_everything_ && o.rsf_.only_cash_) element["rsf_only_cash"]=1;
        if (!o.rsf_.pass_everything_ && o.rsf_.only_sbrebank_online_) element["rsf_only_sbrebank_online"]=1;
        if (!o.rsf_.pass_everything_ && o.rsf_.with_pet_) element["rsf_with_pet"]=1;
        
        os << result << std::endl;
    } // for (auto &o_i : assigned_orders)
    
    // Assigned drivers
    for (auto &d_i : assigned_drivers)
    {
        json result;
        result["event"] = "test_everything";
        
        auto &d = d_i.second;
        result["data"]["drivers"].push_back({
                   {"status", "assigned_driver"},
                    {"driver_id", d.id_},
                {"assigned_order_id", d.assigned_order_id_},
                    {"lat", (double)d.position_where_.latitude},
                    {"lon", (double)d.position_where_.longitude},
                    {"dt_coords_updated", d.position_when_},
                    {"expires_in_seconds", (d.position_when_ + marketplace::params::position_freshness_time()) - time(NULL)},
                    {"best_rider_id", d.top_order_.rider_id_},
                    {"best_order_desirability", d.top_order_.desirability_},
                    {"best_order_metric", d.top_order_metric_value_}
                });
            
        // Driver side filters
        set_dsf_helper(result["data"]["drivers"][result["data"]["drivers"].size()-1], d);
        
        os << result << std::endl;
        
    } // for (auto &d_i : assigned_drivers)
}

/*
void to_json(marketplace::marketplace &m, json &result)
{
    time_t now = time(NULL);
        
    // Get all marketplace :-)
    std::unordered_map<long long, marketplace::order> price_views;
	std::unordered_map<long long, marketplace::order> unassigned_orders;
    std::unordered_map<long long, marketplace::driver> unassigned_drivers;
    std::unordered_map<long long, marketplace::order> assigned_orders;
    std::unordered_map<long long, marketplace::driver> assigned_drivers;

    m.get_price_views(price_views);
    m.get_unassigned_orders(unassigned_orders);
    m.get_unassigned_drivers(unassigned_drivers);
    m.get_assigned_orders(assigned_orders);
    m.get_assigned_drivers(assigned_drivers);

         result["event"] = "test_everything";
         
         for (auto &pv_i : price_views)
         {
             auto &pv = pv_i.second;
             result["data"]["riders"].push_back(
                 {
                     {"rider_id", pv.rider_id_},
                     {"price_view_id", pv.id_},
                     {"latA", pv.A_.latitude},
                     {"lonA", pv.A_.longitude},
                     {"latB", pv.B_.latitude},
                     {"lonB", pv.B_.longitude},
                     {"dt_added", pv.dt_added_},
                     {"expires_in_seconds", (pv.dt_added_ + marketplace::params::price_view_expiration_time()) - time(NULL)},
                     {"status", "price_view"},
                     {"price", pv.price_},
                     {"best_pickup_ETA", pv.best_ETA_min_},
                     {"best_pickup_distance", pv.best_pickup_route_length_km_},
                    {"best_driver_id", pv.best_driver_id_}
                }
             );
         }
    
 
    int best_driver_id_;

        for (auto &o_i : unassigned_orders)
         {
             auto &o = o_i.second;
             result["data"]["riders"].push_back(
                 {
                     {"rider_id", o.rider_id_},
			{"unassigned_order_id", o.id_},
                     {"latA", o.A_.latitude},
                     {"lonA", o.A_.longitude},
                     {"latB", o.B_.latitude},
                     {"lonB", o.B_.longitude},
                     {"dt_added", o.dt_added_},
                     {"expires_in_seconds", (o.dt_added_ + marketplace::params::unassigned_order_expiration_time()) - time(NULL)},
                     {"status", "order"},
                     {"price", o.price_},
                     {"suggested_price", o.suggested_price_},
                    {"best_pickup_ETA", o.best_ETA_min_},
                    {"best_pickup_distance", o.best_pickup_route_length_km_},
                    {"best_driver_id", o.best_driver_id_}
                 }
             );
         }
         for (auto &d_i : unassigned_drivers)
         {
		auto &d = d_i.second;
             result["data"]["drivers"].push_back(
                 {
                    {"status", "unassigned_driver"},
                     {"driver_id", d.id_},
                     {"lat", (double)d.position_where_.latitude},
                     {"lon", (double)d.position_where_.longitude},
                     {"dt_coords_updated", d.position_when_},
                     {"expires_in_seconds", (d.position_when_ + marketplace::params::position_freshness_time()) - time(NULL)},
                     {"best_rider_id", d.top_order_.rider_id_},
                     {"best_order_desirability", d.top_order_.desirability_},
                     {"best_order_metric", d.top_order_metric_value_}
                 }
             );

             // Driver side filters
             set_dsf_helper(result["data"]["drivers"][result["data"]["drivers"].size()-1], d);
             
         } // for (auto &d_i : unassigned_drivers)

    for (auto &o_i : assigned_orders)
    {
         auto &o = o_i.second;
         result["data"]["riders"].push_back(
             {
             
                 {"rider_id", o.rider_id_},
             
                {"assigned_order_id", o.id_},
                {"assigned_driver_id", o.assigned_driver_id_},
             {"ts_driver_at_A", o.ts_driver_at_A_},
             {"ts_ride_started", o.ts_ride_started_},
             {"price_for_toll_wait", o.price_for_toll_wait_},

                {"latA", o.A_.latitude},
                 {"lonA", o.A_.longitude},
                 {"latB", o.B_.latitude},
                 {"lonB", o.B_.longitude},
                 {"dt_added", o.dt_added_},
                 {"expires_in_seconds", (o.dt_added_ + marketplace::params::unassigned_order_expiration_time()) - time(NULL)},
                 {"status", "assigned_order"},
                 {"price", o.price_},
                 {"suggested_price", o.suggested_price_},
                {"best_pickup_ETA", o.best_ETA_min_},
                {"best_pickup_distance", o.best_pickup_route_length_km_},
                {"best_driver_id", o.best_driver_id_}
             }
         );

        // Order options (rider side filters)
        json &element = result["data"]["riders"][result["data"]["riders"].size()-1];
        if (!o.rsf_.pass_everything_ && o.rsf_.no_smoking_) element["rsf_no_smoking"]=1;
        if (!o.rsf_.pass_everything_ && o.rsf_.need_childseat_) element["rsf_need_childseat"]=1;
        if (!o.rsf_.pass_everything_ && o.rsf_.only_cash_) element["rsf_only_cash"]=1;
        if (!o.rsf_.pass_everything_ && o.rsf_.only_sbrebank_online_) element["rsf_only_sbrebank_online"]=1;
        if (!o.rsf_.pass_everything_ && o.rsf_.with_pet_) element["rsf_with_pet"]=1;
     }
    
     for (auto &d_i : assigned_drivers)
     {
    auto &d = d_i.second;
         result["data"]["drivers"].push_back(
             {
                {"status", "assigned_driver"},
                 {"driver_id", d.id_},
             {"assigned_order_id", d.assigned_order_id_},
                 {"lat", (double)d.position_where_.latitude},
                 {"lon", (double)d.position_where_.longitude},
                 {"dt_coords_updated", d.position_when_},
                 {"expires_in_seconds", (d.position_when_ + marketplace::params::position_freshness_time()) - time(NULL)},
                 {"best_rider_id", d.top_order_.rider_id_},
                 {"best_order_desirability", d.top_order_.desirability_},
                 {"best_order_metric", d.top_order_metric_value_}
             }
         );
         
         // Driver side filters
        set_dsf_helper(result["data"]["drivers"][result["data"]["drivers"].size()-1], d);
     }

        // Get notifications
        std::list<marketplace::rider_notification> rn;
        std::list<marketplace::driver_notification> dn;
        m.get_notification_lists(rn, dn);

        for (auto &r : rn)
        {
            result["data"]["rider_notify"].push_back({
                {"seconds_ago", now - r.when_},
                {"rider_id", r.rider_id_},
                {"type", r.object_type_},
                {"old_price", r.old_price_},
                {"new_price", r.new_price_},
                {"old_metric", r.old_metric_value_},
                {"new_metric", r.new_metric_value_},
                {"new_best_driver", r.new_driver_id_},
                {"new_best_pickup_ETA", r.best_ETA_min_},
                {"new_best_pickup_distance", r.best_pickup_route_length_km_}
            });
        }
        
        for (auto &d : dn)
        {
            result["data"]["driver_notify"].push_back({
                {"seconds_ago", now - d.when_},
                {"order_id", d.id_},
                {"type", "new_top_order"},
                {"driver_id", d.driver_id_},
                {"order_desire", d.top_order_desirability_},
                {"order_metric", d.top_order_metric_value_}
            });
        }
        
}
*/

} // namespace dumper
