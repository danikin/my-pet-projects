/*
*       test_motion_simulator.cpp
*
*       (C) Denis Anikin 2020
*
*      	Simulator of the motion along a track
* 
*/

#include <iostream>
#include <vector>

#include <math.h>
#include <time.h>
#include <unistd.h>

#include "polylineencoder.h"
#include "nlohmann/json.hpp"

#include "motion_simulator.h"
#include "profiler.h"

#include "marketplace.h"

namespace marketplace
{
    profiler g_profiler_(3);
}

using json = nlohmann::json;

void test(long long driver_id,
            const std::vector<geo::point> &route,
            double duration,
            double interval,
            bool is_personal)
{
    std::vector<geo::point> intermediate_points = geo::get_intermediate_points(route, duration, interval);
    
    std::cout.precision(17);
    
    for (auto &p : intermediate_points)
    {
//        std::cout << p.latitude << ", " << p.longitude << std::endl;
        //std::cout << "{\"event\":\"test_update_driver_position\",\"data\":{\"driver_id\":" << driver_id << ",\"lat\":" << p.latitude << ",\"lon\":" << p.longitude << "}}" << std::endl;
        
        if (is_personal)
        {
            // Send a personal message for a driver about coordinate change
            std::cout << "{\"event\":\"personal_message\",\"data\":{\"auth_string\":\"driver-" << driver_id
            << "\",\"personal_message_type\":\"simulator_coordinate_change\",\"lat\":" << p.latitude << ",\"lon\":"
            << p.longitude
            << "}}" << std::endl;
        }
        else
        {
            // Send a broadcast message
            std::cout << "{\"event\":\"marketplace2_update_driver_position\",\"data\":{\"driver_id\":" << driver_id << ",\"lat\":" << p.latitude << ",\"lon\":" << p.longitude << "}}" << std::endl;
        }
        
        usleep(interval * 1000000);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3 && argc != 6 && argc != 4)
        std::cout << "Usage: ./test_motion_simulator personal(1|0) driver_id [encoded polyline duration interval]" << std::endl <<
                     "       curl 'http://router.project-osrm.org/...' | ./test_motion_simulator driver_id interval" << std::endl;
    else
    if (argc == 3)
    {
        bool personal = atoi(argv[1]);
        std::vector<geo::point> route = {
            {.longitude=50.8583,.latitude=61.65058 },
            {.longitude=50.8587,.latitude=61.65096 },
            {.longitude=50.85932,.latitude=61.65075 },
            {.longitude=50.85991,.latitude=61.64985 },
            {.longitude=50.88211,.latitude=61.65303 },
            {.longitude=50.87968,.latitude=61.65723 },
            {.longitude=50.87587,.latitude=61.65822 },
            {.longitude=50.86906,.latitude=61.66329 },
            {.longitude=50.8581,.latitude=61.66579 },
            {.longitude=50.85613,.latitude=61.66576 },
            {.longitude=50.85199,.latitude=61.66614 },
            {.longitude=50.84614,.latitude=61.66803 },
            {.longitude=50.83855,.latitude=61.66386 },
            {.longitude=50.8346,.latitude=61.66546 },
            {.longitude=50.83207,.latitude=61.66768 },
            {.longitude=50.82155,.latitude=61.66518 },
            {.longitude=50.81979,.latitude=61.66686 },
            {.longitude=50.82372,.latitude=61.66778 },
            {.longitude=50.82479,.latitude=61.66682 },
            {.longitude=50.825,.latitude=61.66686 }
        };
        
        test(atoi(argv[2]), route, 300, 1, personal);
    }
    else
    if (argc == 6)
    {
        bool personal = atoi(argv[1]);
        int driver_id = atoi(argv[2]);
        
        gepaf::PolylineEncoder::Polyline pl = gepaf::PolylineEncoder::decode(argv[3]);
        
        std::vector<geo::point> route;
        for (auto p : pl)
            route.push_back({.longitude = p.longitude(), .latitude = p.latitude()});

        test(driver_id, route, atoi(argv[4]), atoi(argv[5]), personal);
    }
    else
    if (argc == 4)
    {
        bool personal = atoi(argv[1]);
        long long driver_id = atoll(argv[2]);
        int interval = atoi(argv[3]);
        
        /*double latA = atof(argv[2]);
        double lonA = atof(argv[3]);
        double latB = atof(argv[4]);
        double lonB = atof(argv[5]);*/
        
        // Build route
      /*  std::string uri = "/trip/v1/driving/" + std::to_string(lonA) + "," + std::to_string(latA) + ";" + std::to_string(lonB) + "," + std::to_string(latB) + "?source=first&destination=last&roundtrip=false";
        
        httplib::Client("http://router.project-osrm.org:80").Get("/", [&](const char *data, size_t data_length) {
            std::cout << std::string(data, data_length) << std::endl;
          return true;
        });
        
        return 0;*/
        
        /*
        auto res = c.Get(uri.c_str());
        if (!res || res->status != 200)
        {
            std::cerr << "could not build route, uri=" << uri << "" << std::endl;
            return 0;
        }*/
        
       // std::cout << uri << std::endl;
        
        // Parse route
        //json j;//(res->body);

        std::string s;
        
        try
        {

        std::cin >> s;
            if (s.empty())
            {
                std::cerr << "test_motion_simulator: INPUT STRING IS EMPTY! PROBABLY A PROBLEM WITH ROUTING SERVICE" << std::endl;
                return 0;
            }
        json j = json::parse(s);
        
        // Decode route
        gepaf::PolylineEncoder::Polyline pl = gepaf::PolylineEncoder::decode(j["trips"][0]["geometry"]);

        std::vector<geo::point> route;
        for (auto p : pl)
            route.push_back({.longitude = p.longitude(), .latitude = p.latitude()});

        // Run test
        test(driver_id, route, j["trips"][0]["duration"], interval, personal);
            
        }
        catch (std::exception &e)
        {
            std::cerr << "test_motion_simulator: EXCEPTION: e.what()='" << e.what() << ", s='" << s << "'" << std::endl;
        }
    }
    
    return 0;
}
