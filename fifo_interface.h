
/*
*    fifo_interace.h
*
*    (C) Denis Anikin 2020
*
*    Headers for the fifo interface of the taxi service
*
*/

#ifndef _fifo_interface_h_included_
#define _fifo_interface_h_included_

#include <string>
#include <map>

#include "nlohmann/json.hpp"

namespace fifo_interface
{

// Returns a file name of a headquarters fifo (fifo is created if needed);
std::string get_headquarters_fifo_file_name();

// Returns a file name of an agent fifo (fifo is created if needed);
std::string get_agent_fifo_file_name(int pid);

// Sends the response to the local agent
void message_from_headquartes_to_agent(int pid, const std::string &result);

// Reads the next line from fifo
void fifo_operation_read(const std::string &file_name, std::string &input_string);

// Writes a line to fifo
void fifo_operation_write(const std::string &file_name, const std::string &output_string);

// Connects high level world of objects and low level world of fifos :-)
class multicastor
{
public:
    
    // Multicast a message to all agents associated with this object
    void multicast(long long object_id, const std::string &message);
    
    // Link object to pid
    void link_object_to_pid(long long object_id, int pid);

    // Dumps it to json
    void to_json(nlohmann::json &result);
    
private:
    
    // These are for sending messages from headquarters to agents
    std::multimap<long long, int> object_id_to_pid_;
};

}

#endif
