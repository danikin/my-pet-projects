/*
*    ws_interace.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the web socket interface of the taxi service
*
*/

#include <iostream>
#include <fstream>
#include <unordered_map>

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <thread>

#include "nlohmann/json.hpp"

#include "marketplace.h"
#include "marketplace_api.h"
#include "ws_interface.h"
#include "ws_interface_test.h"
#include "ws_interface_external.h"
#include "fifo_interface.h"
#include "debug.h"

namespace ws_interface
{

using json = nlohmann::json;

/*
 *
 *      Web socket API consists of three message loops:
 *
 *      1.  Agent vs client loop:
 *
 *          It has access to the following streams:
 *
 *              Input:  messages from one specific end client (mobile device or web browser)
 *              Output: messages to its client
 *              Output: messages to headquarters    ( -> hq fifo )
 *              TODO: fifo may not work properly with multiple writes
 *
 *      2.  Agent vs headquarters loop:
 *
 *          It has access to the following streams:
 *
 *              Input:  messages from headquarters  ( <- per agent fifo)
 *              Output: messages to its client
 *
 *      3.  Headquarters loop
 *
 *          It has access to the following streams:
 *
 *              Input:  messages from agents        ( <- hq fifo)
 *              Output: messages to agents          ( -> per agent fifo)
 *
 *      Note: each agent and headquarters live in different address spaces
 *
 */

// Sends a message from a local agent to the headquarters
void send_message_to_headquarters(const std::string &fifo_file_name, json &j)
{
    fifo_interface::fifo_operation_write(fifo_file_name, j.dump());
}



/*
 *      Note! This function runs in a local agent
 */

void ws_agent::agent_vs_client_loop()
{
    auto &input_stream_end_client = std::cin;
    auto &output_stream_end_client = std::cout;
    
    std::string fifo_file_name = fifo_interface::get_headquarters_fifo_file_name();
    
    std::string input_string;
    while (true)
    {
        try
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            long long sec1 = tv.tv_sec * 1000000 + tv.tv_usec;
            
            // Read a json message from the end client
            if (!getline(input_stream_end_client, input_string))
            {
                usleep(10000);
                continue;
            }

            gettimeofday(&tv, NULL);
            long long sec2 = tv.tv_sec * 1000000 + tv.tv_usec;
            
            // If getline took less than 0.1 miliseconds then it's very likely that the
            // server is overloaded because it's too little of waiting for a next client call
            if (sec2 - sec1 < 100)
            {
              //  std::cerr << "ws_agent::agent_vs_client_loop: WARNING! It's very likely that we had pending requests. The getline call has returned in " << (sec2-sec1) << " microseconds. The server could be overloaded!" << ", sec1=" << sec1 << ", sec2=" << sec2 << std::endl;
            }

            // If there is something pending in the input queue then it's very likely that the server is
            // overloaded. It could be overloaded either because of enourmous workload or because that
            // rebalancing gets too slow
            // In both cases we need to debug it
            // So check the pending bytes
            char buf[1];
            if (input_stream_end_client.readsome(buf, 1) == 1)
            {
                // Return the char back to the stream
                input_stream_end_client.putback(buf[0]);
                
                bool b_fail = input_stream_end_client.fail();
                
                std::cerr << "ws_agent::agent_vs_client_loop: pending data right after reading a line. The server could be overloaded! The failbit is " << b_fail << std::endl;
            }
        
            // Parse the json message
            json j = json::parse(input_string);
            
            // A new marketplace
            if (mpl2_.process_json_request(j, output_stream_end_client)) {} else
            
            /*
             *      Test/dev interface for the marketplace. Marketplace exists only during one
             *      session - that's until a page reloads
             *
             *
             */
            
            if (ws_interface_test_handler(output_stream_end_client, test_marketplace_, j)) {} else
   
            /*
            *      Interface for the quick marketplace test: create from scratch, rebalance, see result
            */

            if (ws_interface_external_handler(output_stream_end_client, j)) {} else
                
            /*
             *      Production interface
             */
                
                
            // Authentication
            // It's normally sent once per connnection
            // But
            if (j["event"] == "auth_by_token")
            {
                j["data"]["_originator_pid_"] = getpid();

                // Send our pid to headquarters back
                send_message_to_headquarters(fifo_file_name, j);

                // Save token localy for the hack: when we get response back then we check if it is
                // for us or the pid has just changed. In the second case scenario the response
                // will just be lost. But that's no big deal - the app will send auth request again
                token_ = j["data"]["token"];
                
                // Note: the fact that we saved the token DOES NOT mean that the user is authenticated
                // The user is only authenticated when we obtained her/his ids - which will happen after
                // a positive response from the headquarters
                
                // No response to the end user right now. The response will be sent by the headquarters later
            }
            else
            // Position of the user changed
            if (j["event"] == "pos_ch")
            {
                // {"event":"pos_ch",data:["lon":55.1,"lat":111.2]}
                
                // If the user is authenticated
                if (user_id_ != -1)
                {
                    // Add user info to the event
                    // {"event":"pos_ch",data:["user_id":123,"rider_id":456,"driver_id":789,"lon":55.1,"lat":111.2]}
                    j["data"]["user_id"] = user_id_;
                    j["data"]["rider_id"] = rider_id_;
                    j["data"]["driver_id"] = driver_id_;
                    
                    // Route the message to headquarters
                    // Headquarters will change marketplace if needed and notify other agents
                    // to route changed position info to end users
                    j["data"]["_originator_pid_"] = getpid();
                    send_message_to_headquarters(fifo_file_name, j);
                    
                    // No response to the end user neeeded because this method is VERY FREQUENT
                    // Unless we need a debug
                    if (!j["data"]["debug"].empty())
                    {
                        json result;
                        result["event"] = "pos_ch_ok";
                        output_stream_end_client << result << std::endl;
                    }
                }
                else
                {
                    json result;
                    result["event"] = "pos_ch_error";
                    result["data"]["reason"] = "not_authenticated";
                    output_stream_end_client << result << std::endl;
                }

            } // if (j["event"] == "pos_ch")

            // TODO: Message from a rider to a driver
            // TODO: Message from a driver to a rider
        }
        catch (std::exception &e)
        {
            std::cerr << "ws_server::agent_vs_client_loop exception: " << e.what() << std::endl;
            
            json result = {{"event","error"}, {"data",e.what()}};
            
            std::cout << result.dump(-1) << std::endl;
            
            // This is to ignore any json errors
            usleep(10000);
            continue;
        }
    } // while (true);
}

/*
*      Note! This function runs in a local agent
*/

void ws_agent::agent_vs_headquarters_loop()
{
    std::string agent_fifo_file_name = fifo_interface::get_agent_fifo_file_name(getpid());

    auto &output_stream_end_client = std::cout;

    
    // Note: output_stream_end_client could block because it's connected to a client over Internet
    // But it is ok because this is a loop for one specific end client
    
    std::string input_string;
    while (true)
    {
        try
        {
            // Read from fifo
            // Note: each time we read from fifo we do open-read-close. Otherwise it blocks
            fifo_interface::fifo_operation_read(agent_fifo_file_name, input_string);

            std::cerr << "ws_agent::agent_vs_headquarters_loop read line " << input_string
            << std::endl;

            // Parse the json message
            json j = json::parse(input_string);
                         
            // Headquarters autenticated sucessfully
            if (j["event"] == "auth_ok")
            {
                // {"event":"auth_ok",:"data":{"token":"blablabla","user_id":123,"rider_id":-1,"driver_id":456}}
                              
                // Check that tokens match! This is because the response could be sent wrongly because
                // of pid reusage
                // Note: in this very rare case scenario we just ask an end user to auth again
                if (token_ != j["data"]["token"])
                {
                    j["event"] = "auth_failed";
                    j["data"] = {};
                    j["data"]["reason"] = "try_again_later";
                }
                else
                {
                    // This is a positive response to authentication - save credentials
                    // TODO: multithearding sync!

                    user_id_ = j["data"]["user_id"];
                    rider_id_ = j["data"]["rider_id"];
                    driver_id_ = j["data"]["driver_id"];
                }

                // Send response to the end user
                // Note: if it's something else than auth_ok then just send it transparently to the end user
                output_stream_end_client << j << std::endl;
            } // if (j["event"] == "auth_ok")
            else
            // Headquarters autenticated not sucessfully
            if (j["event"] == "auth_failed")
            {
                // Send response to the end user as is
                output_stream_end_client << j << std::endl;
            }
        }
        catch (std::exception &e)
        {
            std::cerr << "ws_server::agent_vs_headquarters_loop exception: " << e.what() << std::endl;
            // This is to ignore any json errors
            usleep(10000);
            continue;
        }
    } // while (true)
}
                      
/*
*      Note! This function runs in headquarters
*/

void headquarters_loop(auth::authenticator &a,
                                        marketplace::marketplace_api &mrktplc,
                                        fifo_interface::multicastor &rider_multicastor,
                                        fifo_interface::multicastor &driver_multicastor)
{
    std::string fifo_file_name = fifo_interface::get_headquarters_fifo_file_name();
    
    std::string input_string;
    while (true)
    {
        try
        {
            // Read from fifo
            fifo_interface::fifo_operation_read(fifo_file_name, input_string);
            
            std::cout << debug::debug()
                << "headquarters_loop: read line " << input_string << std::endl;

            // Parse the json message
            json j = json::parse(input_string);
            
            // A local agent asks the headquarters to authenticate its end client
            if (j["event"] == "auth_by_token")
            {
                json result;
                // -> {"event":"auth_by_token", "data":{"_originator_pid_":123,"token":"foobar"}}

                // A message from a local agent MUST ALWAYS have this field
                int _originator_pid_ = j["data"]["_originator_pid_"];

                // Authenticate the client
                auto auth_result = a.authenticate_by_token(j["data"]["token"]);
                if (auth_result.is_ok())
                {
                    //  <-  {"event":"auth_ok",:"data":
                    //      {"token":"blablabla","token":"foobar",
                    //      "user_id":123,"rider_id":-1,"driver_id":456}}

                    result["event"] = "auth_ok";
                
                    // Note: we send the token back to the local agent to
                    // hack the possible situation when the agent exists and a new one takes the same pid
                    // Note: we don't send _originator_pid_ to the agent - no need
                    result["data"]["token"] = j["data"]["token"];
                    result["data"]["user_id"] = auth_result.user_id_;
                    result["data"]["rider_id"] = auth_result.u_.rider_id_;
                    result["data"]["driver_id"] = auth_result.u_.driver_id_;
                    
                    // Link rider_id -> pid and driver_id -> pid - for multicasting end users
                    rider_multicastor.link_object_to_pid(auth_result.u_.rider_id_,_originator_pid_);
                    driver_multicastor.link_object_to_pid(auth_result.u_.driver_id_,_originator_pid_);
                }
                else
                {
                    result["event"] = "auth_failed";
                    result["data"]["reason"] = "invalid_token";
                }

                std::cout << debug::debug()
                    << "headquarters_loop: message an agent " << _originator_pid_ << ", result:"
                    << result.dump() << std::endl;
                                
                std::string str_mes = result.dump();

                // Now we need to send a response back to the agent to be sent to the end client
                // Note: it's in a separate thread because agent can hang and headquarters MUST work anyway
                std::thread *t = new std::thread(
                                                 
                                                 [_originator_pid_, str_mes](void){
                    try {
                    std::cout << debug::debug() << "headquarters_loop: started a thread of sending to agent " <<
                        _originator_pid_ << std::endl;
                    fifo_interface::message_from_headquartes_to_agent(_originator_pid_, str_mes);
                    } catch (std::exception &e)
                    {
                        std::cout << debug::debug() << "headquarters_loop: the thread of sending to agent stopped by error: "
                        << e.what() << std::endl;
                    }
                });

                std::cout << debug::debug()
                    << "headquarters_loop: message to an agent " << _originator_pid_ << " sent" << std::endl;

            } //  if (j["event"] == "auth_by_token")
            else
            // A local agent notifies headquarters about user's position change on her/his behalf
            if (j["event"] == "pos_ch")
            {
                // {"event":"pos_ch",data:["user_id":123,"rider_id":456,"driver_id":789,"lon":55.1,"lat":111.2]}
                float lat = j["data"]["lat"];
                float lon = j["data"]["lon"];
                double user_id = j["data"]["user_id"];
                double rider_id = j["data"]["rider_id"];
                double driver_id = j["data"]["driver_id"];
            
                if (driver_id != -1)
                {
                    // If the driver is currently on the order (after acceptance) then
                    // find its rider and notify her/him
                    long long other_party_id = mrktplc.get_rider_id_by_driver(driver_id);
                    if (other_party_id != -1)
                    {
                        // Resent pos_ch message without user_id and driver_id because
                        // the rider does not need it
                        j["data"]["user_id"].clear();
                        j["data"]["rider_id"].clear();
                        
                        std::string result = j.dump();

                        // Multicast all agents of a rider (multicast in case if there are many end users
                        // per a rider)
                        //{"event":"pos_ch",data:["driver_id":789,"lon":55.1,"lat":111.2]}
                        // TODO: multicastor can block on hanging agents! Consider separate thread for this. But without race conditions :-)
                        rider_multicastor.multicast(other_party_id, result);
                    }

                    // If the driver is not currently on the way from A to B then it's open for
                    // order distribution (even if she/he is on the way to A), so notify marketplace
                    if (!mrktplc.is_driver_on_the_way_from_A_to_B(driver_id))
                        mrktplc.update_driver_position(driver_id, {.longitude=lon, .latitude=lat});
                } // if (driver_id != -1)
                
                // Note: in a future we need to send a rider position back to its driver
                // but this is only by a rider acceptance
                
            } // if (j["event"] == "pos_ch")
        }
        catch (std::exception &e)
        {
            std::cerr << debug::debug() << "ws_server::headquarters_loop exception: " << e.what() << std::endl;
            // This is to ignore any json errors
            usleep(10000);
            continue;
        }
    } // while (true);
}

/*
 *      Incoming messages
 *
 *      1.  Endpoint authentication (once per connection)
 *      2.  Geo position update
 *
 *      Messages sent by websocket impl
 *
 *      1.  The other party position changed
 *
 *      Messages from marketplace (outgoing - sent after each request to the marketplace as part of a bunch)
 *
 *      1.  To a rider: price of a price view changed
 *      2.  To a rider: permission to increase an order price
 *      3.  To a driver: the top order or its price has changed
 *
 *      Message from a rider to a driver (outgoing - sent by http_inteface impl)
 *
 *      1.  I canceled the order
 *
 *      Message from a driver to a rider (outgoing - sent by http_inteface impl)
 *
 *      1.  I accepted the order
 *      2.  I'll be later
 *      3.  I canceled the order
 *      4.  I got to the point
 *      5.  Free waiting is over
 *      6.  Ride began
 *      7.  Ride ended, pay
 *      8.  Payment done, good buy :-)
 */

}

