/*
*       motion_simulator.cpp
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

#include "motion_simulator.h"

namespace geo
{

point bird_flight_simulate(const point &from, const point &to, double interval, double speed)
{
	double distance = shortest_distance_km(from, to);
    
    double distance_per_interval_ratio = (interval * speed) / distance;
    
    return {.longitude = from.longitude + distance_per_interval_ratio * (to.longitude - from.longitude),
        .latitude = from.latitude + distance_per_interval_ratio * (to.latitude - from.latitude)
    };
}

// Determines the speed along a route in pifagors per second for the specified route with the
// specified duration
double get_speed(const std::vector<point> &route,
                                double duration)
{
    double distance = 0;
    const point *prev_waypoint = NULL;
    for (auto &waypoint : route)
    {
        if (prev_waypoint)
            distance += shortest_distance_km(*prev_waypoint, waypoint);
        prev_waypoint = &waypoint;
    }
    
    return distance / duration;
}

// Returns all intermediate points for the motion along the specified route
// with the specified duration through the specified interval
// Note: the less the interval the more the points in the result
std::vector<point> get_intermediate_points(const std::vector<point> &route,
                                                        double duration,
                                                        double interval)
{
    if (route.size() < 2)
        return {};
    
    std::vector<point> result;
    
    double speed = get_speed(route, duration);
    
   // std::cout << "speed = " << speed << std::endl;
    
    const point *prev_waypoint = &route[0], *waypoint = &route[1];
    
    // How much time does it take to pass the first segment of the way?
    double this_segment_interval = shortest_distance_km(*prev_waypoint, *waypoint) / speed;

    //std::cout << "first segment interval = " << this_segment_interval << std::endl;
    
    double timer = 0;
    while (true)
    {
        // Go along the way as long as "timer" seconds (for the first iteration it's just "interval" seconds)
            
        // Have we reached the end of the segment?
        if (timer < this_segment_interval)
        {
            // No, we haven't
            // Simulate the bird flight from the begining of this segment for "timer" seconds
            result.push_back(bird_flight_simulate(*prev_waypoint, *waypoint, timer, speed));
            timer += interval;
        }
        else
        {
            // Yes, we have
            // Now first we add the waypoint to the result
            result.push_back(*waypoint);
                        
            // And then we go to the next waypoint, but keeping in mind that the timer is
            // partially cleared
            timer = timer - this_segment_interval;
                        
            prev_waypoint = waypoint;
            ++waypoint;

            // The route is finished
            if (waypoint - &route[0] >= route.size())
                break;
                        
            this_segment_interval = shortest_distance_km(*prev_waypoint, *waypoint) / speed;
        }
    }
    
    return result;
}

} // namespace geo

