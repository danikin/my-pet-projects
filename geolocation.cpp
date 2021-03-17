/*
*       geolocation.cpp
*
*       (C) Denis Anikin 2020
*
*       Implementation file for geolocation services
*
*/

#include <iostream>
#include <algorithm>

#include "geolocation.h"

#include "marketplace2.h"

namespace geo
{

long double shortest_distance_km(long double lat1, long double long1, long double lat2, long double long2)
{
    // Convert the latitudes
      // and longitudes
      // from degree to radians.
      lat1 = toRadians(lat1);
      long1 = toRadians(long1);
      lat2 = toRadians(lat2);
      long2 = toRadians(long2);
        
      // Haversine Formula
      long double dlong = long2 - long1;
      long double dlat = lat2 - lat1;
    
      long double ans = pow(sin(dlat / 2), 2) +
                            cos(lat1) * cos(lat2) *
                            pow(sin(dlong / 2), 2);
    
      ans = 2 * asin(sqrt(ans));
    
      // Radius of Earth in
      // Kilometers, R = 6371
      // Use R = 3956 for miles
      long double R = 6371;
        
      // Calculate the result
      ans = ans * R;
    
    // Return value in km
    return ans;
}

// Converts degree point to radians point
GeoCoord point_to_geocoord(const point &p)
{
    return {.lat = degsToRads(p.latitude), .lon = degsToRads(p.longitude)};
}

bool global_position_db::update_object_position(unsigned long long object_id,
                                const point *previous_position,
                                const point *new_position,
                                bool debug_print_on)
{
    auto ___dont_use_me___ = marketplace::g_profiler_.profile("global_position_db::update_object_position");
    
    if (!previous_position && !new_position)
    {
        std::cerr << "global_position_db::update_object_position: no positions for object_id=" <<
            object_id << std::endl;
        return false;
    }
    
    if (debug_print_on)
    {
        std::cerr << "global_position_db::update_object_position: begin: object " << object_id
        << " min_resolution_=" << min_resolution_ << ", max_resolution_=" << max_resolution_;
        if (previous_position)
            std::cerr << " previous_position: " << previous_position->latitude << ", " << previous_position->longitude;
        if (new_position)
            std::cerr << " new_position: " << new_position->latitude << ", " << new_position->longitude;
        std::cerr << std::endl;
    }
    
    GeoCoord prev_coord, new_coord;
    
    if (previous_position)
        prev_coord = point_to_geocoord(*previous_position);
    if (new_position)
        new_coord = point_to_geocoord(*new_position);
    
    // Iterate all resolutions
    for (int res = min_resolution_; res <= max_resolution_; ++res)
    {
        auto ___dont_use_me___ = marketplace::g_profiler_.profile("resolution loop");
        
        // Special case - object is deleted - just delete it
        if (!new_position)
        {
            H3Index H3_ind_prev = geoToH3(&prev_coord, res);
            if (debug_print_on)
                std::cerr << "global_position_db::update_object_position: erase object " << object_id
                << " with H3 index " << H3_ind_prev << ", res=" << res << std::endl;
            
            auto ___dont_use_me___ = marketplace::g_profiler_.profile("erase geo object");
            
            h3_to_objects_[H3_ind_prev].erase(object_id);
            continue;
        }
        
        H3Index H3_ind_new;
        
        {
        auto ___dont_use_me___ = marketplace::g_profiler_.profile("H3_ind_new=geoToH3");
        
        H3_ind_new = geoToH3(&new_coord, res);
        
        // Special case - object is added first time - just add it
        if (!previous_position)
        {
            if (debug_print_on)
                std::cerr << "global_position_db::update_object_position: insert object " << object_id
                << " with H3 index " << H3_ind_new << ", res=" << res << std::endl;

            auto ___dont_use_me___ = marketplace::g_profiler_.profile("insert geo object");
            
            h3_to_objects_[H3_ind_new].insert(object_id);
            continue;
        }
        }

        H3Index H3_ind_prev;
        {
        auto ___dont_use_me___ = marketplace::g_profiler_.profile("H3_ind_prev=geoToH3");
        
        H3_ind_prev = geoToH3(&prev_coord, res);
        }
        
        auto ___dont_use_me___4 = marketplace::g_profiler_.profile("before finds");
        
        // Find hash sets for previous and new positions of the object
        auto previous_hash_set_i = h3_to_objects_.find(H3_ind_prev);
        auto new_hash_set_i = h3_to_objects_.find(H3_ind_new);
        
        // Skip special case - object new coordinates reside in the same index as old coordinates
        // Note: it's normally very frequent scenario especially for coarse resolutions
        if (!(previous_hash_set_i != h3_to_objects_.end() && previous_hash_set_i == previous_hash_set_i))
        {
            auto ___dont_use_me___ = marketplace::g_profiler_.profile("same h3");

            // Erase the object from its previous index
            if (previous_hash_set_i != h3_to_objects_.end())
            {
                if (debug_print_on)
                    std::cerr << "global_position_db::update_object_position: erase the object " << object_id
                    << " from (H3_ind_prev=" << H3_ind_prev << ", res=" << res << ")" << std::endl;
                
                previous_hash_set_i->second.erase(object_id);
            }
            
            // The index entry is empty = create a new one with a hash set of a single object
            if (new_hash_set_i == h3_to_objects_.end())
            {
                if (debug_print_on)
                    std::cerr << "global_position_db::update_object_position: insert the object " << object_id
                    << " to NEW ENTRY (H3_ind_new=" << H3_ind_new << ", res=" << res << ")" << std::endl;
                
                h3_to_objects_.insert({H3_ind_new, {object_id}});
            }
            // The index entry is not empty - insert the object there
            else
            {
                if (debug_print_on)
                    std::cerr << "global_position_db::update_object_position: insert the object " << object_id
                    << " to EXISTING ENTRY (H3_ind_new=" << H3_ind_new << ", res=" << res << ")" << std::endl;
                
                new_hash_set_i->second.insert(object_id);
            }
        }
    } // for (int res = min_resolution_; res < max_resolution_; ++res)
    
    return true;
}

bool global_position_db::get_objects_in_radius(const point &p,
                                                int resolution,
                                                int ring_size,
                                                std::vector<H3Index> &neighbours_temp_buffer,
                                                std::vector<H3Index> &compacted_temp_buffer,
                                                std::vector<unsigned long long> &object_ids,
                                                bool debug_print_on)
{
    GeoCoord coord = point_to_geocoord(p);
    
    H3Index ind = geoToH3(&coord, resolution);
    
    // Get all neighbours for the point
    // All the neighbours will be at the same resolution
    int max_neighbours = maxKringSize(ring_size);
    neighbours_temp_buffer.clear();
    neighbours_temp_buffer.resize(max_neighbours, 0);
    kRing(ind, ring_size, &neighbours_temp_buffer[0]);
    
//    std::cerr << "global_position_db::get_objects_in_radius debug: ind=" << ind << ", resolution=" << resolution
//        << ", ring_size=" << ring_size << ", max_neighbours=" << max_neighbours
//    << ", h3_to_objects_.size()=" << h3_to_objects_.size() << std::endl;
    
    // TODO: some of neighbours could be zeroes. Probably we need to get rid of them
    // before calling compact

    // Compact them
    compacted_temp_buffer.clear();
    compacted_temp_buffer.resize(max_neighbours, 0);
    int err = compact(&neighbours_temp_buffer[0], &compacted_temp_buffer[0], max_neighbours);
    if (err)
    {
        std::cerr << "global_position_db::get_objects_in_radius debug: compact failed. max_neighbours=" << max_neighbours
        << ", ind=" << ind << std::endl;
        return false;
    }
    
    int n_lookups = 0;
    object_ids.clear();
    for (auto &i : compacted_temp_buffer)
    {
        // Skip zero indexes
        if (i)
        {
            //std::cerr << "global_position_db::get_objects_in_radius debug: inside compact loop, H3Index=" << i
              //  << std::endl;
            
            // Each index contains a hash table of objects
            // Add all of them to the result
            // Note: don't mind duplicated ids - that's why the result is an unordered_set :-)
            auto objects_per_index_i = h3_to_objects_.find(i);
            if (objects_per_index_i != h3_to_objects_.end())
            {
               // std::cerr << "global_position_db::get_objects_in_radius debug: object found"
                 //   << std::endl;
                
                for (auto j : objects_per_index_i->second)
                    object_ids.push_back(j);
            }
        }
        ++ n_lookups;
    } // for (auto &i : compacted)
    
    // Leave only unique IDS in object_ids
    int n_insertions = object_ids.size();
    std::sort(object_ids.begin(), object_ids.end());
    object_ids.erase(std::unique(object_ids.begin(), object_ids.end()), object_ids.end());
    
    if (debug_print_on)
        std::cerr << "global_position_db::get_objects_in_radius debug: neighbours_temp_buffer.size()="
        << neighbours_temp_buffer.size() << ", compacted_temp_buffer.size()=" << compacted_temp_buffer.size()
        << ", n_lookups=" << n_lookups << ",n_insertions=" << n_insertions << ", object_ids.size()="
        << object_ids.size() << std::endl;
    
    return true;
}

void global_position_db::print_stat()
{
    unsigned long long total_object_ids =  0;
    for (auto &i : h3_to_objects_)
        total_object_ids += i.second.size();
    std::cerr << "global_position_db::print_stat: h3_to_objects_.size()=" << h3_to_objects_.size() <<
        ", total_object_ids=" << total_object_ids << std::endl;

    std::cerr << "global_position_db::print_stat: h3_to_objects_better_version_.size()=" << h3_to_objects_better_version_.size() << ", h3_to_objects_better_version_.capacity()="
    << h3_to_objects_better_version_.capacity() <<
    ", it's APPROX " << (h3_to_objects_better_version_.capacity() * sizeof(std::pair<H3Index, long long>))/1048576
    << " Mb" << std::endl;
}

void global_position_db::dump_compacted_things(std::ostream &os)
{
    os << "LOCATIONS" << std::endl;
    os << h3_to_objects_better_version_.size() << std::endl;
    for (auto &i : h3_to_objects_better_version_)
        os << i.first << "," << i.second << std::endl;
}

void global_position_db::undump_compacted_things(std::istream &is)
{
    std::string s;
    std::getline(is, s);
    std::getline(is, s);
    size_t size = std::stoi(s);
    h3_to_objects_better_version_.resize(size);
    h3_to_objects_better_version_.shrink_to_fit();
    for (auto &i : h3_to_objects_better_version_)
    {
        std::getline(is, s);
        size_t n = s.find(",");
        if (n == std::string::npos)
        {
            std::cerr << "global_position_db::undump_compacted_things, file is boken: " << s << std::endl;
            break;
        }
        i.first = std::stoll(std::string(s.begin(), s.begin() + n));
        i.second = std::stoll(std::string(s.begin() + n + 1, s.end()));
    }
}

void global_position_db::update_object_position_better_version(size_t geo_location_n,
                                            unsigned long long object_id,
                                           const point &new_position)
{
    GeoCoord prev_coord;
    GeoCoord new_coord = point_to_geocoord(new_position);
       
    // Iterate all resolutions
    for (int res = min_resolution_; res <= max_resolution_; ++res)
    {
        H3Index H3_ind_new = geoToH3(&new_coord, res);
        h3_to_objects_better_version_[geo_location_n].first = H3_ind_new;
        h3_to_objects_better_version_[geo_location_n].second = object_id;
        geo_location_n++;
    }
    
    // Note! We will need to sort it!
}

bool operator<(const std::pair<H3Index, unsigned long long> &a, const std::pair<H3Index, unsigned long long> &b)
{
    return a.first < b.first;
}

void global_position_db::sort()
{
    std::sort(h3_to_objects_better_version_.begin(), h3_to_objects_better_version_.end());
}

void global_position_db::resize_and_shrink_to_fit(size_t size)
{
    h3_to_objects_better_version_.resize(size);
    h3_to_objects_better_version_.shrink_to_fit();
}

bool global_position_db::get_objects_in_radius_better_version(
                                                              const point &p, int resolution, int ring_size, std::unordered_set<unsigned long long> &object_ids) const
{
    GeoCoord coord = point_to_geocoord(p);
    
    H3Index ind = geoToH3(&coord, resolution);
    
    // Get all neighbours for the point
    // All the neighbours will be at the same resolution
    int max_neighbours = maxKringSize(ring_size);
    std::vector<H3Index> neighbours(max_neighbours);
    kRing(ind, ring_size, &neighbours[0]);

    // TODO: some of neighbours could be zeroes. Probably we need to get rid of them
    // before calling compact

    // Compact them
    std::vector<H3Index> compacted(max_neighbours);
    int err = compact(&neighbours[0], &compacted[0], max_neighbours);
    if (err)
    {
        std::cerr << "global_position_db::get_objects_in_radius_better_version debug: compact failed. max_neighbours=" << max_neighbours
        << ", ind=" << ind << std::endl;
        return false;
    }
    
    
    //std::cerr << "global_position_db::get_objects_in_radius_better_version debug: # of compacted" <<
    //    compacted.size() << std::endl;
    
    for (auto &i : compacted)
    {
        // Skip zero indexes
        if (i)
        {
            //std::cerr << "global_position_db::get_objects_in_radius debug: inside compact loop, H3Index=" << i
              //  << std::endl;
            
            // Search for H3 index equals "i" using a binary search
            // There can be a number of such objects
            for (auto objects_per_index_i = std::lower_bound(
                                                        h3_to_objects_better_version_.begin(),
                                                        h3_to_objects_better_version_.end(),
                                                             i,
                                                             [] (const std::pair<H3Index, unsigned long long> &a, const H3Index &i)
                                                             {
                                                                 return a.first < i;
                                                             });
                 objects_per_index_i != h3_to_objects_better_version_.end();
                 ++objects_per_index_i)
            {
                
               // std::cerr << "global_position_db::get_objects_in_radius_better_version debug: found an object, searching for " << i << ", objects_per_index_i->first=" << objects_per_index_i->first << std::endl;
                
                // Iterate all of the objects with equal H3 = break;
                if (objects_per_index_i->first != i)
                    break;
                
               // std::cerr << "global_position_db::get_objects_in_radius debug: object found"
                 //   << std::endl;
                
                object_ids.insert(objects_per_index_i->second);
            }
        }
    } // for (auto &i : compacted)
    
    return true;
}
} // namespace geo
