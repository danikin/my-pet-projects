/*
*    ws_interace_test.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the test ws interface of the taxi service
*
*/

#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"

#include "marketplace.h"
#include "marketplace_dumper.h"

namespace ws_interface
{

using json = nlohmann::json;

/*
std::string marketplace_to_json(marketplace::marketplace &m, bool is_human_readable)
{
    json result;

    dumper::to_json(m, result);
    
    std::string s = result.dump(is_human_readable ? 1 : -1);
    
    std::cerr << "std::string marketplace_to_json s.size() " << s.size() << std::endl;

    return s;
}*/

// Run the process of simulation of driver motion
// TODO: driver goes strickly by a route, but it's better to implement some bias from the route
//  to check how it's handled by the driver app
bool test_simulate_driver_motion(long long driver_id, const geo::point &from, const geo::point &to)
{
    // Note: this command cuts non ascii and spaces: | perl -pe 's/[^[:ascii:]]//g' | perl -pe 's/[ ]//g'
    //  for some reason json parser does not accept it
    std::string command = "curl 'http://router.project-osrm.org/trip/v1/driving/" + std::to_string(from.longitude) + ","+
        std::to_string(from.latitude) + ";" +
        std::to_string(to.longitude)  + "," +
        std::to_string(to.latitude) + "?source=first&destination=last&roundtrip=false' 2>/dev/null | perl -pe 's/[^[:ascii:]]//g' | perl -pe 's/[ ]//g' | ./test_motion_simulator 1 " +
            std::to_string(driver_id) + " 1 >>/tmp/fifos/auth_strings/driver-" +
    std::to_string(driver_id) + "&";
    std::cerr << "test_simulate_driver_motion: simulator command: " << command << std::endl;
    if (system(command.c_str()) != 0)
    {
        std::cerr << "test_simulate_driver_motion: simulator command failed, errno=" << errno << ", strerror=" << strerror(errno) << std::endl;
        return false;
    }
    else
        return true;
}

// Sends a personal message to a driver
bool send_personal_message_to_driver(long long driver_id, json &j)
{
    std::ofstream ofs("/tmp/fifos/auth_strings/driver-" + std::to_string(driver_id),
                        std::ios::binary | std::ios::app | std::ios::ate | std::ios::out);
    ofs << j << std::endl;
    
    return !ofs.bad() && !ofs.fail();
}

bool ws_interface_test_handler(std::ostream &output_stream_end_client,
                                marketplace::marketplace &test_marketplace,
                                json &j)
{
            /*
            *      Test/dev interface for the marketplace. Marketplace exists only during one
            *      session - that's until a page reloads
            *
            *
            */
    
            bool is_human_readable = !j["data"]["is_human_readable"].empty();
           
           // Adds a new price view request from a rider
           // On the map it looks like this:
           // 1. Operator clicks anywhere on the map and chooses "Ask price" from the context menu
           // 2. Operator then clicks anywhere on the map - that will be the point B
           // 3. A picture of a new rider appears on the map in point A
           // 4. A rider requested a price is painted yellow
           if (j["event"] == "test_price_view")
           {
               // -> {"event":"test_price_view", "data":{"latA":11.11,"lonA":12.12,"latB":13.13,"lonB":14.14}}
               // <- nothing

               srand(time(NULL));
               
               // ID is random or specified explicitly by a calling party
               long long id;
               
               if (j["data"]["id"].empty())
                   id = rand();
               else
                   id = j["data"]["id"];
               
               test_marketplace.add_price_view(marketplace::order(id, id, {.longitude=j["data"]["lonA"], .latitude=j["data"]["latA"]},
                              {.longitude=j["data"]["lonB"], .latitude=j["data"]["latB"]}), time(NULL));
               
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
           // Places an order
           // On the map it looks like this:
           // 1. Operator clicks a rider and chooses "Place order" from the context menu
           // 2. A rider placed an order is painted green
           // Note: rider_id identifies a rider and is received in the event test_get_everything - see below
           if (j["event"] == "test_order")
           {
               // -> {"event":"test_order", "data":{"rider_id":123}}
               // <- nothing
               
               // rider_id and pv_id are the same here
               test_marketplace.convert_price_view_to_order(j["data"]["rider_id"], j["data"]["rider_id"], time(NULL));
               
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
            // Accepts an order by a driver
            if (j["event"] == "test_accept_order")
            {
                // -> {"event":"test_accept_order", "data":{"rider_id":123, "driver_id"=456}}
                // <- nothing
                
                long long driver_id = j["data"]["driver_id"], rider_id = j["data"]["rider_id"];
                
                /*
                 
                    This is for the test reasons: after a driver accepted an order we begin to
                        simulate the motion
                 
                 */
                
                geo::point driver_pos = test_marketplace.get_driver_position(driver_id);
                geo::point A_pos, B_pos;
                
                if (!(driver_pos.latitude == 0 && driver_pos.longitude == 0))
                {
                    // Note: we imply here that rider_id == order_id. Which is true only for the test!!!
                    if (test_marketplace.get_order_points(rider_id, &A_pos, &B_pos))
                        test_simulate_driver_motion(driver_id, driver_pos, A_pos);
                    else
                        std::cerr << "test_accept_order: can't get order points, order_id/rider_id=" << rider_id << std::endl;
                }
                else
                    std::cerr << "test_accept_order: can't get driver position, driver_id=" << driver_id << std::endl;
                   
                // Note: rider_id and order id are the same here
                // TODO!!!
                test_marketplace.accept_order(rider_id, driver_id, time(NULL));
                
                dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
                  // output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
            }
            // Finishes a ride by a driver
            if (j["event"] == "test_finish_ride")
            {
                // -> {"event":"test_finish_ride", "data":{"driver_id":123}}
                // <- nothing
    
                long long driver_id = j["data"]["driver_id"];
        
                if (!test_marketplace.finish_ride(driver_id, time(NULL)))
                    std::cerr << "test_finish_ride: error" << std::endl;
                else
                    dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
                    //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
            }
            else
            // Starts a ride simulator
            if (j["event"] == "test_start_ride_simulator")
            {
                // -> {"event":"test_start_ride_simulator", "data":{"driver_id"=456}}
                // <- nothing
                
                long long driver_id = j["data"]["driver_id"];
                
                // Get driver's assigned order
                marketplace::order assigned_order = test_marketplace.get_assigned_order(driver_id);
                if (assigned_order.id_ == -1)
                {
                    std::cerr << "test_start_ride_simulator: error: driver " << driver_id << " does not have an assigned order" << std::endl;
                }
                else
                {
                    /*
                 
                    This is for the test reasons: after a driver starts a ride we begin to
                        simulate the motion
                 
                     */
                
                    geo::point driver_pos = test_marketplace.get_driver_position(driver_id);
                    geo::point A_pos, B_pos;
                
                    if (!(driver_pos.latitude == 0 && driver_pos.longitude == 0))
                    {
                        // Note: simulate a ride not from A to B but from the driver pos to B. Because
                        //  the pick-up could have been in a different place
                        test_simulate_driver_motion(driver_id, driver_pos, assigned_order.B_);
                    }
                    else
                        std::cerr << "test_start_ride_simulator: can't get driver position, driver_id=" << driver_id << std::endl;
                }
            }
            else
            if (j["event"] == "test_set_auto_accept_best_order_flag")
            {
                // -> {"event":"test_set_auto_accept_best_order_flag", "data":{"driver_id":123, "flag":1}}
                // <- nothing
                
                long long driver_id = j["data"]["driver_id"];
                int flag = j["data"]["flag"];
                if (!test_marketplace.set_auto_accept_best_order_flag(driver_id, flag, time(NULL)))
                    std::cerr << "test_set_auto_accept_best_order_flag: can't set_auto_accept_best_order_flag, driver_id=" << driver_id << std::endl;
                else
                    dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
                    //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
            }
            else
            if (j["event"] == "test_driver_got_A")
            {
                // -> {"event":"test_driver_got_A", "data":{"driver_id":123}}
                // <- nothing
                
                long long driver_id = j["data"]["driver_id"];
                if (!test_marketplace.driver_got_A(driver_id, time(NULL)))
                    std::cerr << "test_driver_got_A: can't driver_got_A, driver_id=" << driver_id << std::endl;
                else
                    dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
                    //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
            }
            else
            if (j["event"] == "test_driver_started_ride")
            {
                // -> {"event":"test_driver_started_ride", "data":{"driver_id":123}}
                // <- nothing
        
                long long driver_id = j["data"]["driver_id"];
                if (!test_marketplace.driver_started_ride(driver_id, time(NULL)))
                    std::cerr << "test_driver_started_ride: can't driver_started_ride, driver_id=" << driver_id << std::endl;
                else
                    dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
                    //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
            }
            else
            if (j["event"] == "test_driver_accepted_toll_wait")
            {
                // -> {"event":"test_driver_accepted_toll_wait", "data":{"driver_id":123, "seconds":456}}
                // <- nothing
        
                long long driver_id = j["data"]["driver_id"];
                long long seconds = j["data"]["seconds"];
                int real_seconds = test_marketplace.driver_accepted_toll_wait(driver_id, seconds, time(NULL));
                if (real_seconds == -1)
                    std::cerr << "test_driver_accepted_toll_wait: can't do driver_accepted_toll_wait, driver_id=" << driver_id << ", seconds=" << seconds << std::endl;
                else
                    dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
                    //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
            }
            else
            if (j["event"] == "test_driver_cancelled_order")
            {
                // -> {"event":"test_driver_cancelled_order", "data":{"driver_id":123}}
                // <- nothing
                
                long long driver_id = j["data"]["driver_id"];
    
                if (!test_marketplace.driver_cancelled_order(driver_id, time(NULL)))
                    std::cerr << "test_driver_cancelled_order: can't driver_cancelled_order, driver_id=" << driver_id << std::endl;
                else
                    dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
                    //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
            }
            else
            if (j["event"] == "test_rider_cancelled_order")
            {
                // -> {"event":"test_driver_cancelled_order", "data":{"rider_id":123}}
                // <- nothing
                
                long long rider_id = j["data"]["rider_id"];
 
                // Note: rider_id and order id are the same here
                // TODO!!!
                if (!test_marketplace.rider_cancelled_order(rider_id, time(NULL)))
                    std::cerr << "test_driver_cancelled_order: can't rider_cancelled_order, rider_id=" << rider_id << std::endl;
                else
                    dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
                    //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
            }
            else
           // On the map it looks like this:
           // 1. Operator clicks anywhere and chooses "New driver" from the context menu
           // 2. A new driver appears there
           // 3. A driver is painted green
           if (j["event"] == "test_newdriver")
           {
               // -> {"event":"test_newdriver", "data":{"lat":11.11,"lon":15.15}}
               // <- nothing
               srand(time(NULL));
               
               // ID is random or specified explicitly by a calling party
               long long id;
               
               if (j["data"]["id"].empty())
                   id = rand();
               else
                   id = j["data"]["id"];
               test_marketplace.update_driver_position(id,
                                                        {.longitude=j["data"]["lon"], .latitude=j["data"]["lat"]}, time(NULL));
               
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
              // output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
           // On the map it looks like this:
           // 1. Operator drags and drops driver anywhere
           // Note: driver_id identifies a driver and is received in the event test_get_everything - see below
           if (j["event"] == "test_update_driver_position")
           {
               // {"event":"test_update_driver_position", "data":{"driver_id","lat":11.11,"lon":15.15}}

               test_marketplace.update_driver_position(j["data"]["driver_id"],
                                                        {.longitude=j["data"]["lon"], .latitude=j["data"]["lat"]}, time(NULL));
              
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
           
               // This is for changing price-view's A & B by an end user
           if (j["event"] == "test_update_price_view_A")
           {
               // {"event":"test_update_price_view_A", "data":{"rider_id":123,"lat":11.11,"lon":15.15}}

               test_marketplace.update_price_view_A(j["data"]["rider_id"],
                                                        j["data"]["rider_id"],
                                                        {.longitude=j["data"]["lon"], .latitude=j["data"]["lat"]}, time(NULL));
               
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
               if (j["event"] == "test_update_price_view_B")
            {
                // {"event":"test_update_price_view_B", "data":{"rider_id":123,"lat":11.11,"lon":15.15}}

                test_marketplace.update_price_view_B(j["data"]["rider_id"],
                                                        j["data"]["rider_id"],
                                                         {.longitude=j["data"]["lon"], .latitude=j["data"]["lat"]}, time(NULL));
                
                dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
                //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
            }
            else
                
               
           // Returns all the information on riders and drivers. The map must be redrawn each time it
           // receives test_everything. The client side must call this event as frequent as it
           // does not slow down the UI :-) TODO: in a future this event will be sent from the server side
           // automatically each 100ms
           //
           // best_rider_id - is the id of the rider that the system thinks the best for this driver
           // Each time the operator clicks on a driver the map temporary highlights the best rider for that driver
           //
           // All unknown driver and rider fields must be shown as is in rider's card and driver's card that's
           // in the context menu
           //
           // Everytime a new field appears or any rider or driver field is changed EXCEPT CCORDINATES
           // the map must expand rider's or driver's card
           //
           // Note: riders can be removed by the system - that's because price view or order expired.
           // From the client side stand point it's no big deal - removed riders will not be there on the next call :-)
           if (j["event"] == "test_get_everything")
           {
               // For the test enviroment we need to rebalance the marketplace
               // manually every now and then because there is lack of events from clients to
               // rebalance it automatically as reaction on those events
               test_marketplace.force_rebalance(time(NULL));
  
		// No response for this because of the storm from Pavel's map :-)
		// Which we don't need because we anyway broadcast after each change
             
//              output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
           // Forces rebalance right now and dumps the marketplace state and notifications
           if (j["event"] == "test_force_rebalance")
           {
               test_marketplace.force_rebalance(time(NULL));
               
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
           if (j["event"] == "test_get_good_price_and_metric")
           {
               // -> {"event":"test_get_good_price_and_metric",
               //      "data":{"rider_id":123,"driver_id":123}}
               // <- {"event":"test_good_price_and_metric",
               //      "data":{"rider_id":123,"driver_id":123},"price":111.17,"metric":222.34}
               
               double price, metric;
               test_marketplace.get_good_price_and_metric(j["data"]["driver_id"], j["data"]["rider_id"],
                   price, metric);

               json result;
               result["event"] = "test_good_price_and_metric";
               result["data"] = {
                   {"rider_id", j["data"]["rider_id"]},
                   {"driver_id", j["data"]["driver_id"]},
                   {"price", price},
                   {"metric", metric}
               };
               
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << result.dump(is_human_readable ? 1 : -1) << std::endl;
           }
           else
           // In short: it's just an edit box and a button in the context menu. The operator enters the price and
           // presses the button and this event is sent. The price is prefilled with the value of "suggested_price"
           //
           // Long story:
           //
           // The system keeps changing prices based on its internal logic. Everytime the price of an order changes it
           // is sent to the client side through the test_get_everything event. If a rider is "yellow" - that's
           // asked the price but did not place an order - then prices change without the rider permission. If
           // a rider is "green" - that's placed an order - then a price change is suggested to and it's up to the
           // rider to accept it or not - that's why this button :-)
           if (j["event"] == "test_accept_price_change")
           {
               // -> {"event":"test_accept_price_change","data":{"rider_id":123,"new_price":290.83}}
               // <- nothing
               
               test_marketplace.change_order_price(j["data"]["rider_id"], j["data"]["new_price"], time(NULL));
            
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
           // It's just a button in the context menu. Next time test_get_everything is called this rider will
           // not be there :-)
           if (j["event"] == "test_remove_rider")
           {
               // -> {"event":"test_remove_rider","data":{"rider_id":123}}
               // <- nothing
               
               test_marketplace.cancel_price_view(j["data"]["rider_id"], time(NULL));
               test_marketplace.remove_unassigned_order(j["data"]["rider_id"], time(NULL));
               
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
           // It's just a button in the context menu. Same thing as with a rider :-)
           if (j["event"] == "test_remove_driver")
           {
               // -> {"event":"test_remove_driver","data":{"driver_id":123}}
               // <- nothing

               test_marketplace.remove_driver(j["data"]["driver_id"], time(NULL));
               
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
           // Clears all pending notifications
           if (j["event"] == "test_clear_notifications")
           {
               // -> {"event":"test_clear_notifications"}
               // <- nothing

               test_marketplace.clear_notifications();
               test_marketplace.force_rebalance(time(NULL));
               
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
           // Clears everything expired and deleted
           if (j["event"] == "test_clear_expired_and_deleted_objects")
           {
               // -> {"event":"test_clear_expired_and_deleted_objects"}
               // <- nothing

               test_marketplace.clear_expired_and_deleted_objects(time(NULL));
               test_marketplace.force_rebalance(time(NULL));
               
               dumper::send_marketplace_message_by_message(test_marketplace, output_stream_end_client);
               //output_stream_end_client << marketplace_to_json(test_marketplace, is_human_readable) << std::endl;
           }
           else
        
            /*
             *  Driver stat & billing
             */
               
            // Returns statistics
            if (j["event"] == "test_get_stat")
            {
                // -> {"event":"test_get_stat","data":{"stat_type":"rides","driver_id":123,"from":12345,"to":4567}}
                // <- nothing

                // Statistics goes to the personal message of a driver:
                // {"event":"personal_message",
                //      "data":{"auth_string":"driver-123531841","personal_message_type":"stat","entries":
                //      [
                //          { ... fields ... },
                //      ]
                //      }}
                
                std::string stat_type = j["data"]["stat_type"];
                long long driver_id = j["data"]["driver_id"];
                time_t ts_from = j["data"]["from"];
                time_t ts_to = j["data"]["to"];
                
                json pm = json::object();
                pm["event"] = "personal_message";
                pm["data"]["auth_string"] = "driver-" + std::to_string(driver_id);
                pm["data"]["personal_message_type"] = "stat";
                
                if (stat_type == "rides")
                {
                    std::vector<marketplace::driver_stat_entry_ride> result;
                    test_marketplace.get_driver_stat_entries(driver_id, ts_from, ts_to, result);
                    for (auto &e : result)
                    {
                        pm["data"]["entries"].push_back({
                            e.ts_order_accepted_,
                            e.price_,
                            e.real_distance_to_A_/1000,
                            e.real_seconds_to_A_/60,
                            e.real_distance_A_to_B_/1000,
                            e.real_seconds_A_to_B_/60,
                            e.price_ * 3600 / (e.real_seconds_to_A_ + e.real_seconds_A_to_B_ + 1),
                            e.fee_}
                        );
                        /*pm["data"]["entries"].push_back({
                            {"ts_order_accepted", e.ts_order_accepted_},
                            {"ts", e.ts_},
                            {"price", e.price_},
                            {"ride_status", e.ride_status_},
                            {"real_distance_to_A", e.real_distance_to_A_},
                            {"real_seconds_to_A", e.real_seconds_to_A_},
                            {"real_distance_A_to_B", e.real_distance_A_to_B_},
                            {"real_seconds_A_to_B", e.real_seconds_A_to_B_},
                            {"fee", e.fee_},
                        });*/
                    }
                }
                else
                if (stat_type == "aggr_rides")
                {
                    std::vector<marketplace::driver_stat_entry_ride> result;
                    test_marketplace.get_driver_stat_aggr_entries(driver_id, ts_from, ts_to, result);
                    for (auto &e : result)
                    {
                        pm["data"]["entries"].push_back({
                            e.ts_order_accepted_,
                            e.real_distance_A_to_B_, // Number of finished trips
                            e.real_seconds_A_to_B_, // Number of cancelled trips
                            e.price_,
                            e.real_distance_to_A_/1000,
                            e.real_seconds_to_A_/60,
                            e.price_ * 3600 / e.real_seconds_to_A_,
                            e.fee_}
                        );
                    }
                }
                else
                if (stat_type == "shift_purchases")
                {
                    std::vector<marketplace::driver_stat_entry_shift_purchase> result;
                    test_marketplace.get_driver_stat_entry_shift_purchase(driver_id, ts_from, ts_to, result);
                    for (auto &e : result)
                    {
                            pm["data"]["entries"].push_back({
                                e.ts_,
                                e.fee_}
                            );
                            /*pm["data"]["entries"].push_back({
                                {"ts", e.ts_},
                                {"fee", e.fee_}
                            });*/
                    }
                }
                else
                if (stat_type == "payments")
                {
                    std::vector<marketplace::driver_stat_payment> result;
                    test_marketplace.get_driver_stat_payments(driver_id, ts_from, ts_to, result);
                    for (auto &e : result)
                    {
                        pm["data"]["entries"].push_back({
                            e.ts_,
                            e.amount_,
                            e.agent_
                        });
                        /*pm["data"]["entries"].push_back({
                            {"ts", e.ts_},
                            {"amount", e.amount_},
                            {"agent", e.agent_}
                        });*/
                    }
                }
                else
                if (stat_type == "withdrawals")
                {
                    std::vector<marketplace::driver_stat_withdrawal> result;
                    test_marketplace.get_driver_stat_withdrawals(driver_id, ts_from, ts_to, result);
                    for (auto &e : result)
                    {
                        pm["data"]["entries"].push_back({
                             e.ts_,
                             e.amount_,
                             e.agent_
                         });
                        /*pm["data"]["entries"].push_back({
                            {"ts", e.ts_},
                            {"amount", e.amount_},
                            {"agent", e.agent_}
                        });*/
                    }
                }
                else
                
                if (stat_type == "cashbacks")
                {
                    std::vector<marketplace::driver_stat_cashback> result;
                    test_marketplace.get_driver_stat_cashbacks(driver_id, ts_from, ts_to, result);
                    for (auto &e : result)
                    {
                        pm["data"]["entries"].push_back({
                              e.ts_,
                              e.amount_,
                              e.cashback_reason_
                          });
                        /*pm["data"]["entries"].push_back({
                            {"ts", e.ts_},
                            {"amount", e.amount_},
                            {"cashback_reason", e.cashback_reason_}
                        });*/
                    }
                }
                else
                
                if (stat_type == "bonuses")
                {
                    std::vector<marketplace::driver_stat_bonus> result;
                    test_marketplace.get_driver_stat_bonuses(driver_id, ts_from, ts_to, result);
                    for (auto &e : result)
                    {
                        pm["data"]["entries"].push_back({
                              e.ts_,
                              e.amount_,
                              e.bonus_type_
                          });
                        /*pm["data"]["entries"].push_back({
                            {"ts", e.ts_},
                            {"amount", e.amount_},
                            {"bonus_type", e.bonus_type_}
                        });*/
                    }
                }
                else
                if (stat_type == "promos")
                {
                    std::vector<marketplace::driver_stat_promo> result;
                    test_marketplace.get_driver_stat_promos(driver_id, ts_from, ts_to, result);
                    for (auto &e : result)
                    {
                        pm["data"]["entries"].push_back({
                            e.ts_,
                            e.ts_start_,
                            e.ts_finish_,
                            e.promo_type_
                        });
                        /*pm["data"]["entries"].push_back({
                            {"ts", e.ts_},
                            {"ts_start", e.ts_start_},
                            {"ts_finish", e.ts_finish_},
                            {"promo_type", e.promo_type_}
                        });*/
                    }
                }
                else
                if (stat_type == "balance")
                {
                    marketplace::driver_stat_balance e = test_marketplace.restore_driver_balance(driver_id, ts_from);

                    pm["data"]["entries"].push_back({
                            ts_from,
                            e.payment_amout_,
                            e.cashbacks_,
                            e.bonuses_,
                            e.withheld_,
                            e.withdrawals_,
                            e.payment_amout_ + e.cashbacks_ + e.bonuses_ + e.withheld_ + e.withdrawals_,
                            std::max(0.0, std::min(
                                e.payment_amout_ + e.cashbacks_ + e.bonuses_ + e.withheld_ + e.withdrawals_,
                                e.cashbacks_ + e.withdrawals_
                            ))
                        
                    });
                    /*pm["data"]["entries"].push_back({
                            {"payment_amout", e.payment_amout_},
                            {"cashbacks", e.cashbacks_},
                            {"bonuses", e.bonuses_},
                            {"withheld", e.withheld_},
                            {"withdrawals", e.withdrawals_}
                    });*/
                }
                
                // Send a message to the driver (via a special file)
                send_personal_message_to_driver(driver_id, pm);
            }
            else
               return false;
    
    return true;
} // ws_interface_test_handler

}
