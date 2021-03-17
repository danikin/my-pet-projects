/*
*       test_riders_drivers.cpp
*
*       (C) Denis Anikin 2020
*
*      	Simulator of riders placing orders and drivers moving around and accepting them
* 
*/

#include <iostream>
#include <vector>

#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "polylineencoder.h"
#include "nlohmann/json.hpp"

#include "motion_simulator.h"

using json = nlohmann::json;

void test_simulate_drivers(double lat, double lon, int driver_wait, int count)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec + getpid());
    
    while (count)
    {
        // Create a new driver
        double d_lat = lat + (0.0 + (rand() + getpid()) % RAND_MAX - RAND_MAX/2)/RAND_MAX * 0.05;
        double d_lon = lon + (0.0 + (rand() + getpid()) % RAND_MAX - RAND_MAX/2)/RAND_MAX * 0.05;
        
        unsigned long long driver_id = rand() + getpid();

        json j = json::object();
        j["event"] = "marketplace2_update_driver_position";
        j["data"]["driver_id"] = driver_id;
        j["data"]["lat"] = d_lat;
        j["data"]["lon"] = d_lon;

        std::cout << j << std::endl;
        
        // Wait
        usleep(driver_wait * 1000);
        
        --count;
    }
}

// Places an order once "interval" seconds in a random place around lat and lon to
// a random place around there
void test_simulate_orders(double lat, double lon, int pv_wait, int order_wait, int count)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec + getpid());
    
    while (count)
    {
        // Create a new price view
        double latA = lat + (0.0 + (rand() + getpid()) % RAND_MAX - RAND_MAX/2)/RAND_MAX * 0.05;
        double lonA = lon + (0.0 + (rand() + getpid()) % RAND_MAX - RAND_MAX/2)/RAND_MAX * 0.05;
        double latB = lat + (0.0 + (rand() + getpid()) % RAND_MAX - RAND_MAX/2)/RAND_MAX * 0.05;
        double lonB = lon + (0.0 + (rand() + getpid()) % RAND_MAX - RAND_MAX/2)/RAND_MAX * 0.05;
        
        unsigned long long rider_id = rand() + getpid();
        
        json j = json::object();
        j["event"] = "marketplace2_udpate_order";
        j["data"]["rider_id"] = rider_id;
        j["data"]["latA"] = latA;
        j["data"]["lonA"] = lonA;
        j["data"]["latB"] = latB;
        j["data"]["lonB"] = lonB;
        j["data"]["placed_order_price"] = 0;

        std::cout << j << std::endl;
        
        // Wait
        usleep(pv_wait * 1000);
        
        // Place an order
        j["data"]["place_order_at_market_price"] = 1;
        
        std::cout << j << std::endl;
        
        // Wait
        usleep(order_wait * 1000);
        
        --count;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 7)
    {
        std::cerr << "Usage: ./test_riders_drivers [drivers|riders] lat lon pv_wait order_wait_miliseconds count" << std::endl;
    }
    else
    if (!strcmp(argv[1], "drivers"))
        test_simulate_drivers(atof(argv[2]), atof(argv[3]), atoi(argv[4]), atoi(argv[6]));
    else
        test_simulate_orders(atof(argv[2]), atof(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
 
    return 0;
}
