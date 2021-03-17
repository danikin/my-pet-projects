/*
*       object_track.h
*
*       (C) Denis Anikin 2020
*
*       Header file for object tracks
*
*/

#ifndef _object_track_h_included_
#define _object_track_h_included_

#include <time.h>
#include <array>
#include <unordered_map>

#include "nlohmann/json.hpp"

#include "geolocation.h"

namespace geo
{

struct traffic_info_entry
{
    float avg_distance_ratio_ = 0;
    float avg_speed_ = 0;
    unsigned int n_points_ = 0;
};

inline std::ostream &operator<<(std::ostream &os, const traffic_info_entry &tie)
{
    return os << "avg_distance_ratio_=" << tie.avg_distance_ratio_ << ", avg_speed_=" << tie.avg_speed_ <<
        ", n_points_=" << tie.n_points_;
}

// Keeps recent tracks of objects and also keeps track of local average
//  speed and ratios of real distances to manhattan distances
class object_tracks
{
public:
    
    struct lentgh_time
    {
        // Distance so far
        float km_;
        
        // Starting time
        time_t when_started_;
        
        // Starting position
        point where_started_;
    };
    
    struct recent_track_info
    {
        static const int recent_points_number = 3;
        
        bool has_at_least_one_point = false;
        
        // Returns the most recent point of the track or NULL if the track is empty so far
        point *get_most_recent_point()
        {
            if (!has_at_least_one_point)
                return NULL;
            else
                return &recent_points_[(current_pos_-1+recent_points_number) % recent_points_number];
        }
        
        void add_point(const point &where, time_t when)
        {
            recent_whens_[current_pos_] = when;
            recent_points_[current_pos_] = where;
            current_pos_ = (current_pos_+1) % recent_points_number;
            has_at_least_one_point = true;
        }
        
        // Recent recent_points_number points and whens of the track: to know the orientation and speed
        std::array<point, recent_points_number> recent_points_;
        std::array<time_t, recent_points_number> recent_whens_;
        
        // Recent length and time of the route
        lentgh_time lt_;
        
        // 3km back length and time of the route
        lentgh_time lt_3km_back_;
        
        // Current position in recent arrays
        int current_pos_ = 0;
    };

    // Updates position of an object
    void update_object_position(unsigned long long object_id, point where, time_t when);
    
    // Returns the orientation of an object
    double get_object_orientation(long long object_id)
    {
        // TODO: implement
        
        return 0;
    }
    
    // Returns in recent_km_ distance travelled since previous call of this function
    lentgh_time get_recent_track_info(unsigned long long object_id);
    
    // Returns a traffic info per a point
    traffic_info_entry *get_traffic_info(const point &where);
    
    // Dumps it to json
    void to_json(nlohmann::json &result);
    
private:

    // Maps object id onto her/his recent track info
    std::unordered_map<unsigned long long, recent_track_info> recent_tracks_;
    
    // Hash of average speeds and ratios of distances
    // It's in 3.23 edge polygons (resolution 6)
    std::unordered_map<H3Index, traffic_info_entry> traffic_info_;
    
    const int traffic_info_resolution_ = 6;
    
}; // class object_tracks

} // namespace geo

#endif
