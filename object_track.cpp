/*
*       object_track.cpp
*
*       (C) Denis Anikin 2020
*
*       Impl file for object tracks
*
*/

#include "geolocation.h"

#include "object_track.h"

namespace geo
{

void object_tracks::to_json(nlohmann::json &result)
{
    result["event"] = "dump_object_track";
    for (auto &t : recent_tracks_)
    {
        result["data"].push_back({
            {"object_id", t.first},
            {"distance_so_far_km", t.second.lt_.km_},
            {"when_started", t.second.lt_.when_started_},
            {"where_started_lat", t.second.lt_.where_started_.latitude},
            {"where_started_lon", t.second.lt_.where_started_.longitude}
        });
        for (int i = t.second.current_pos_; i < t.second.current_pos_ + recent_track_info::recent_points_number; ++i)
        {
            result["data"][result["data"].size()-1]["recent_points"].push_back({
                {"lat", t.second.recent_points_[i % recent_track_info::recent_points_number].latitude},
                {"lon", t.second.recent_points_[i % recent_track_info::recent_points_number].longitude},
                {"when", t.second.recent_whens_[i % recent_track_info::recent_points_number]}
            });
        }
    }
};

void object_tracks::update_object_position(unsigned long long object_id, point where, time_t when)
{
    // If this is first time the object is updated then
    // remember its when and set the distance to 0
    auto i = recent_tracks_.find(object_id);
    if (i == recent_tracks_.end())
    {
        i = recent_tracks_.insert({object_id, recent_track_info()}).first;
        i->second.lt_.km_ = 0;
        i->second.lt_.when_started_ = when;
        i->second.lt_.where_started_ = where;
        
        i->second.lt_3km_back_ = i->second.lt_;
        
        /*std::cerr << "object_tracks: after insert: object_id=" << object_id << ", lt_3km_back_.km_="
            << i->second.lt_3km_back_.km_ << ", where_started=" << i->second.lt_3km_back_.where_started_
            << ", when_started=" << i->second.lt_3km_back_.when_started_ << std::endl;*/
    }
    // If this is not the first time then calc distance beetween previous point
    // and the current position
    else
    {
        /*std::cerr << "object_tracks: after find: object_id=" << object_id << ", lt_3km_back_.km_="
            << i->second.lt_3km_back_.km_ << ", where_started=" << i->second.lt_3km_back_.where_started_
            << ", when_started=" << i->second.lt_3km_back_.when_started_ << std::endl;*/
        
        // In case if when_started_ has been reset by the previous call to get_recent_track_info
        // Otherwise don't touch "when_started_"
        if (!i->second.lt_.when_started_)
        {
            i->second.lt_.when_started_ = when;
            i->second.lt_.where_started_ = where;
        }
        
        // Update total distance
        // Note: here manhattan_distance_km is fine. Because it's between two close points
        point *prev_point = i->second.get_most_recent_point();
        float distance_delta = prev_point ? shortest_distance_km(*prev_point, where) : 0;
        i->second.lt_.km_ += distance_delta;
        
        // Every 3 km of a journey update the traffic info
        lentgh_time &lt_3km_back = i->second.lt_3km_back_;
        if (lt_3km_back.km_ < 3.0)
        {
            lt_3km_back.km_ += distance_delta;
            
            /*std::cerr << "object_tracks: after find(2): object_id=" << object_id << ", lt_3km_back_.km_="
                << i->second.lt_3km_back_.km_ << ", where_started=" << i->second.lt_3km_back_.where_started_
                << ", when_started=" << i->second.lt_3km_back_.when_started_
                << ", distance_delta=" << distance_delta << ", prev_point:";
                if (prev_point)
                    std::cerr << *prev_point;
                else
                    std::cerr << "NULL";
                std::cerr << std::endl;*/
        }
        else
        {
            // It's time to save traffic info
            float real_distance = lt_3km_back.km_;
            float manhattan_distance = manhattan_distance_km(lt_3km_back.where_started_, where);
            float distance_ratio = real_distance / manhattan_distance;
            // Note: speed is in km per minute
            float avg_speed = 60.0 * real_distance / (when - lt_3km_back.when_started_);
            
            // Reset it
            lt_3km_back.when_started_ = when;
            lt_3km_back.where_started_ = where;
            lt_3km_back.km_ = 0;
            
            // Save new average in the traffic_info_entry
            GeoCoord coord = point_to_geocoord(where);
            H3Index h3 = geoToH3(&coord, traffic_info_resolution_);
            traffic_info_entry &tie = traffic_info_[h3];

            /*std::cerr << "object_tracks: object_id=" << object_id << ", h3=" << h3 << ", distance_delta=" << distance_delta << ", real_distance=" << real_distance << ", manhattan_distance=" <<
                manhattan_distance << ", distance_ratio=" << distance_ratio << ", avg_speed=" <<
                avg_speed << ", tie: " << tie << std::endl;*/
            
            // new average = (old average * N + new value) / (N+1)
            tie.avg_distance_ratio_ = (tie.avg_distance_ratio_ * tie.n_points_ + distance_ratio) / (tie.n_points_+ 1);
            tie.avg_speed_ = (tie.avg_speed_ * tie.n_points_ + avg_speed) / (tie.n_points_+ 1);
            ++tie.n_points_;
        }
    }

    // Add the point to the track
    i->second.add_point(where, when);
}

traffic_info_entry *object_tracks::get_traffic_info(const point &where)
{
    GeoCoord coord = point_to_geocoord(where);
    H3Index h3 = geoToH3(&coord, traffic_info_resolution_);
    
    auto i = traffic_info_.find(h3);
    if (i == traffic_info_.end())
        return NULL;
    return &i->second;
}

object_tracks::lentgh_time object_tracks::get_recent_track_info(unsigned long long object_id)
{
    auto i = recent_tracks_.find(object_id);
    if (i == recent_tracks_.end())
        return {.km_ = 0, .when_started_ = 0};
    else
    {
        // Reset length_time and set the time to now before return
        // Meaning: the next update_object_position will start from scratch
        
        auto save = i->second.lt_;
        i->second.lt_.km_ = 0;
        i->second.lt_.when_started_ = 0; // It will be updated with the next update_object_position call
        i->second.lt_.where_started_ = {0.0, 0.0}; // It will be updated with the next update_object_position call
        
        return save;
    }
}

} // namespace geo
