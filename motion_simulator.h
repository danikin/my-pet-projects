/*
*       motion_simulator.h
*
*       (C) Denis Anikin 2020
*
*      	Simulator of the motion along a track
* 
*/

#ifndef _motion_simulator_h_
#define _motion_simulator_h_

#include <vector>

#include "geolocation.h"

namespace geo
{

// Simulates motion from one point to another like a bird
// From: startring point
// To: finishing point
// interval: how long to move
// pifagors_per_second: speed
point bird_flight_simulate(const point &from, const geo::point &to, double interval, double speed);

double get_speed(const std::vector<point> &route, double duration);

// Returns all intermediate points for the motion along the specified route
// with the specified duration through the specified interval
// Note: the less the interval the more the points in the result
std::vector<point> get_intermediate_points(const std::vector<point> &route,double duration, double interval);

} // namespace geo

#endif

