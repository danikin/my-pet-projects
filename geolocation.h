/*
*       geolocation.h
*
*       (C) Denis Anikin 2020
*
*       Header file for geolocation services
*
*/

#ifndef _geolocation_h_included_
#define _geolocation_h_included_

#include <math.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>

#include <h3/h3api.h>

namespace geo
{

// Geo point
class point
{
public:

    // In degrees
    double longitude, latitude;
};

inline std::ostream &operator<<(std::ostream &os, const point &p)
{
    return os << "lat=" << p.latitude << ", lon=" << p.longitude;
}

// Converts degree to radians
inline long double toRadians(const long double degree)
{
    long double one_deg = (M_PI) / 180;
    return (one_deg * degree);
}

// Returns the shortest distance (bird fly) between two points in km
long double shortest_distance_km(long double lat1, long double long1, long double lat2, long double long2);

inline long double shortest_distance_km(point p1, point p2)
{
    return shortest_distance_km(p1.latitude, p1.longitude, p2.latitude, p2.longitude);
}

// Returns manhattan distance in km
inline long double manhattan_distance_km(point A, point B)
{
    long double shortest = shortest_distance_km(A.latitude, A.longitude, B.latitude, B.longitude);
    long double distance_beetween_lats = abs(A.latitude - B.latitude) * 111.045;
 
    // Now use pythagorean theorem
    return sqrt(shortest * shortest - distance_beetween_lats * distance_beetween_lats) + distance_beetween_lats;
}

// Converts degree point to radians point
GeoCoord point_to_geocoord(const point &p);

// Space indexing database
// Can be used for reverse geocoding or for fast searching for objects within
// the specified distance to a point
class global_position_db
{
public:
    
    // max_resolution   -   maximum resolutions in terms of H3
    // The bigger the max_resolution the bigger the index, but the better is the precision
    // 11               -   radius approx 25 meters
    // 10               -   radius approx 65 meters
    // 9                -   radius approx 175 meters
    // 8                -   radius approx 461 meters
    // 7                -   radius approx 1.22km
    // 6                -   radius approx 3.23km
    // 5                -   radius approx 8.5km
    // 4                -   radius approx 22.6km
    // 3                -   radius approx 59.8km
    // 2                -   radius approx 158km
    // 1                -   radius approx 419km
    global_position_db(int min_resolution, int max_resolution) :
        min_resolution_(min_resolution), max_resolution_(max_resolution) {}
    
    // Updates indexes according to changed position of an object
    // Note: if this is the first time the position of an object is changing then
    //  previous_position MUST BE NULL
    bool update_object_position(unsigned long long object_id,
                                                    const point *previous_position,
                                                    const point *new_position,
                                                    bool debug_print_on);

    // Returns ids of all objects close to the "p"
    // Note: "close" means by air distance
    // Note: resolution and ring_size mean the size of hexagon and it's neighbours to describe the
    //  area of search
    // Note: this function works approximately (because a circle is simulated with a bunch of hexagons
    //  under the hood)
    bool get_objects_in_radius(const point &p,
                                int resolution,
                                int ring_size,
                                std::vector<H3Index> &neighbours_temp_buffer,
                                std::vector<H3Index> &compacted_temp_buffer,
                                std::vector<unsigned long long> &object_ids,
                                bool debug_print_on);
    
    // Returns the id of the nearest object to the point "p"
    long long get_nearest_object(const point &p);

	void print_stat();
    
    void dump_compacted_things(std::ostream &os);
    void undump_compacted_things(std::istream &is);
    
    // Places an object to the better version structure
    // Note: geo_location_n - to avoid push backs and save RAM
    void update_object_position_better_version(size_t geo_location_n,
                                               unsigned long long object_id,
                                               const point &new_position);
    
    // Sort and shrink the better version structure
    void sort();
    void resize_and_shrink_to_fit(size_t size);
    
    bool get_objects_in_radius_better_version(
                                              const point &p,
                int resolution, int ring_size, std::unordered_set<unsigned long long> &object_ids) const;
    
    int get_resolution_spread()
    {
        return max_resolution_ - min_resolution_ + 1;
    }
    
private:
    // GeoCoord
    
    
    // For each H3Index stores a list of all objects inside this hexagon
    // Note: we really need two nested hash tables. Why?
    // Some objects change their location frequently. So we need to quickly find them
    // inside the index, remove from the index and add to another index. Everything is at O(1) :-)
    std::unordered_map<H3Index, std::unordered_set<unsigned long long> > h3_to_objects_;
    
    // A better version of index - to use less RAM
    std::vector<std::pair<H3Index, unsigned long long> > h3_to_objects_better_version_;
    
    // Minimum and maximum resolution that we need have indexes for
    int min_resolution_, max_resolution_;
};

} // namespace geo

#endif
