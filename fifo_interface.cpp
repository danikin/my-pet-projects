/*
*    fifo_interace.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for the fifo interface of the taxi service
*
*/

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <string>

#include "debug.h"
#include "fifo_interface.h"
#include "marketplace.h"

namespace fifo_interface
{

// A helper function
//
// Creates a fifo if it does not exist
// If it does not exist then it's created and true is returned
// If it exists then nothing is done and true is returned
// If it can't be created or it's not a fifo then false is returned
bool fifo_init_(const char *path_to_fifo)
{
    // First stat the fifo file
    struct stat stat_info;
    if (stat(path_to_fifo, &stat_info) == -1)
    {
        // It does not exist - make a fifo
        if (errno == ENOENT)
            // If it's made then return true and return false otherwise
            return !mkfifo(path_to_fifo, 0666);
        // Another error - too bad
        else
            return false;
    }
    // It exists. Return true if it's a fifo
    else
        return S_ISFIFO(stat_info.st_mode);
}

std::string get_headquarters_fifo_file_name()
{
    const char *file_name = "/tmp/anikin-taxi-headquarters-fifo";
    if (fifo_init_(file_name))
        return file_name;
    else
    {
        std::cerr << "get_headquarters_fifo_file_name: can't access fifo file by this path " << file_name << ", errno=" << strerror(errno) << std::endl;
        return "";
    }
}

std::string get_agent_fifo_file_name(int pid)
{
    std::string file_name = "/tmp/anikin-taxi-local-fifo-" + std::to_string(pid);
    
    if (fifo_init_(file_name.c_str()))
        return file_name;
    else
    {
        std::cerr << "get_agent_fifo_file_name: can't access fifo file by this path " << file_name << ", errno=" << strerror(errno) << std::endl;
        return "";
    }
}

void fifo_operation_read(const std::string &file_name, std::string &input_string)
{
    // Note: each time we read from fifo we do open-read-close. Otherwise it blocks
    std::cerr << "FIFO OPERATION. READ. CAN BLOCK! " << file_name << std::endl;
    std::ifstream is(file_name.c_str(), std::ifstream::in | std::ifstream::binary);
    std::cerr << "FIFO OPERATION. READ. DONE " << file_name << std::endl;

    std::cerr << "FIFO OPERATION. READ. CAN BLOCK! " << file_name << std::endl;
    getline(is, input_string);
    std::cerr << "FIFO OPERATION. READ. DONE " << file_name << std::endl;

    is.close();
}

void fifo_operation_write(const std::string &file_name, const std::string &output_string)
{
    // Note: each time we write to fifo we do open-write-close. Otherwise it blocks
    std::cerr << "FIFO OPERATION. OPEN WRITE. CAN BLOCK! " << file_name << std::endl;
    std::ofstream os(file_name.c_str(),
                            std::ofstream::out | std::ofstream::binary);
    std::cerr << "FIFO OPERATION OPEN WRITE. DONE " << file_name << std::endl;

    std::cerr << "FIFO OPERATION. WRITE. CAN BLOCK! " << file_name << std::endl;
    os << output_string << std::endl;
    std::cerr << "FIFO OPERATION. WRITE. DONE " << file_name << std::endl;

    os.close();
}

void message_from_headquartes_to_agent(int pid, const std::string &result)
{
    fifo_operation_write(fifo_interface::get_agent_fifo_file_name(pid), result);
    
#ifdef dswwewewwewewewe
    std::cout << debug::debug() << "message_from_headquartes_to_agent: initializing fifo, pid=" << pid
        << ", message=" << result << std::endl;

    std::string agent_fifo_file_name = fifo_interface::get_agent_fifo_file_name(pid);
    
    std::cout << debug::debug() << "message_from_headquartes_to_agent: Initializing agent fifo: " << agent_fifo_file_name << std::endl;
    
    // Note: either of flags std::ofstream::app, std::ofstream::ate leads to errno=29 Illegal seek. So I removed them
    
    std::cerr << "FIFO OPERATION. OPEN WRITE. CAN BLOCK! " << agent_fifo_file_name << std::endl;
    std::ofstream ofs(agent_fifo_file_name.c_str(), std::ofstream::out | std::ofstream::binary
                      /*| std::ofstream::app | std::ofstream::ate*/);
    std::cerr << "FIFO OPERATION. OPEN WRITE. DONE " << agent_fifo_file_name << std::endl;

    std::cout << debug::debug() << "message_from_headquartes_to_agent: after opening fifo "
    
    << " is_open=" << ofs.is_open()
    << " is_fail=" << ofs.fail()
    << " is_eof=" << ofs.eof()
    << " errno=" << errno << " strerror=" << strerror(errno)

        << std::endl;

    ofs << result << std::endl;
    ofs.flush();

	std::cout << debug::debug() << "message_from_headquartes_to_agent: sent to a local agent fifo this message: '" << result << "'"
    
    << " is_open=" << ofs.is_open()
    << " is_fail=" << ofs.fail()
    << " is_eof=" << ofs.eof()
    << " errno=" << errno << " strerror=" << strerror(errno)
    
    << std::endl;

    ofs.close();
    
    
    std::cout << debug::debug() << "message_from_headquartes_to_agent: after close"
    
    << " is_open=" << ofs.is_open()
    << " is_fail=" << ofs.fail()
    << " is_eof=" << ofs.eof()
    << " errno=" << errno << " strerror=" << strerror(errno)
    
    << std::endl;
#endif
}

void multicastor::multicast(long long object_id, const std::string &message)
{
    // TODO: solve problem of separate objects for separate auth from a single agent - if one end user
    // through one web socket sends many auth requests then there will be many entries in object_id_to_pid_ - one per each
    
    // Now we need to notify all local agents linked to the rider with id rider_id
    // Use Log(N) algorithm
    //
    // Also this is a good moment to
    // 1. Clear rider_id_to_pid_ and driver_id_to_pid_ from dangling pids
    // 2. Remove fifos for driver_id_to_pid_ pids
    for (auto i = object_id_to_pid_.lower_bound(object_id) ; ; )
    {
        if (i->first != object_id)
            break;
        
        int pid = i->second;
        
        // Check existence of an agent process
        if (kill(pid, 0) == -1 && errno == ESRCH)
        {
            // The process does not exist:
            // 1. Clear it from the map
            // 2. Remove dangling fifo
            // 3. Skip sending a message
            
            auto save = i;
            ++save;
            object_id_to_pid_.erase(i);
            i = save;
            
            remove(fifo_interface::get_agent_fifo_file_name(pid).c_str());
            
            continue;
        }
        
        // Send a message to an agent
        // TODO: it can block on hanging agents!!!
        fifo_operation_write(fifo_interface::get_agent_fifo_file_name(pid), message);
        
        ++i;
    }
}

void multicastor::link_object_to_pid(long long object_id, int pid)
{
    // Fill the map object_id -> pid
    object_id_to_pid_.insert({object_id, pid});
}


void multicastor::to_json(nlohmann::json &result)
{
    result["event"] = "dump_multicastor";
    for (auto &o : object_id_to_pid_)
        result["data"].push_back({
            {"object_id", o.first},
            {"pid", o.second}
        });
}

} // namespace fifo_interface

