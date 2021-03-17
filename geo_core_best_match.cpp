/*
*    geo_core.cpp
*
*    (C) Denis Anikin 2020
*
*    Implementation for core geo services
*
*/

#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <iomanip>
#include <fstream>
#include <sys/time.h>

#include "geo_core.h"

namespace geo
{

void compact_addresses::best_street_name_match(const std::string &search_string,
                                                const point &location,
                                                std::vector<std::pair<unsigned long, float> > &best) const
{
    // 1. Lower case the search_string and divide it into words
    std::string lstr = tolower(search_string);
    auto words = extractWordsVector(lstr);
    
    // A special case
    if (words.empty())
    {
        best.clear();
        return;
    }
    
    // Two-dimensional vector of street ids - for the intersection
    std::vector<std::vector<unsigned long> > vv_ids;
    vv_ids.resize(words.size());
    
    // 2. For every word find a list of all streets with a search object beginning with those words
    int n = 0;
    for (auto &w : words)
    {
        for (auto i = std::lower_bound(compact_search_objects_streets_.begin(),
                                        compact_search_objects_streets_.end(),
                                        w,
                                        [](const std::pair<std::string, std::vector<unsigned long> > &a, const std::string &b)
                                        {
                                            return a.first < b;
                                        }); i != compact_search_objects_streets_.end(); ++i)
        {
             // Check if w is a prefix to i->first (or is found inside i->first)
             if (i->first.find(w) == std::string::npos)
                 break;
             
             vv_ids[n].insert(vv_ids[n].end(), i->second.begin(), i->second.end());
        }
        
        // Sort vv_ids[n] for later intersection
        std::sort(vv_ids[n].begin(), vv_ids[n].end());
        // Unique it
        vv_ids[n].erase(std::unique(vv_ids[n].begin(), vv_ids[n].end()), vv_ids[n].end());

        ++n;
    }
    
    // 3. Now interset ALL vv_ids with each other (normally users use 1 or 2 words - so it's not as
    //      bad as it sounds)
    auto *prev_vector = &vv_ids[0];
    std::vector<unsigned long> result;
    for (int i = 1; i < n; ++i)
    {
        result.clear();
        std::set_intersection(prev_vector->begin(), prev_vector->end(),
                              vv_ids[i].begin(), vv_ids[i].end(),
                              std::back_inserter(result));

        std::cerr << "compact_addresses::best_street_name_match: intersect two vectors: " <<
        prev_vector->size() << "," << vv_ids[i].size() << " -> " << result.size() << std::endl;
        
        result.swap(*prev_vector);

        // Now prev_vector == &vv_ids[i-1] and contains the intersection
        // of everything up to i and result has the content of previous vv_ids[i-1] which
        // we don't need anymore
    }
    
    result.swap(*prev_vector);
    
    if (result.empty())
    {
        std::cerr << "compact_addresses::best_street_name_match: intersection is empty" << std::endl;
        best.clear();
        return;
    }

    std::cerr << "compact_addresses::best_street_name_match: the intersection size is " << result.size() << std::endl;
    
    srand(time(NULL));
    
    // 4. Among intersected streets find ONES that's the best in terms of
    //  having first N houses nearest to "location"
    std::vector<std::pair<unsigned long, float> > temp_best;
    temp_best.resize(best.size());
    for (int i = 0; i < temp_best.size() ; ++i)
        temp_best[i] = {(unsigned long)-1, 1000000000};
    
    for (auto s_id : result)
    {
        const compact_street *cs = get_street_by_street_id(s_id);
        if (!cs)
            continue;
        
        // Bad street - no houses - don't know the location
        if (cs->houses_.empty())
            continue;
        
        // Note! Take 10 random houses for each street to prevent taking the house from
        // far street with the same name
        float distance = 1000000000;
        for (int k = 0; k < std::min(cs->houses_.size(), 10ul); ++k)
        {
            unsigned long house_n = rand() % cs->houses_.size();
            
            float distance_to_house = (float)shortest_distance_km(cs->houses_[house_n].lat_, cs->houses_[house_n].lon_,
                                                    location.latitude, location.longitude);
            if (distance_to_house < distance)
                distance = distance_to_house;
        }
        
        // Check if this distance is better than the worst in the temp result
        // And if it's so then substitute the worst with that
        // It's O(result.size() * temp_best.size()) instead of O(result.size() * Log(temp_best.size())) - not the best algorithm,
        // but for small temp_best.size()s like 3 or 5 it's better than messing around with partial sort :-)
        int worst = -1;
        float worst_distance = 0;
        for (int i = 0;i < temp_best.size(); ++i)
        {
            // It's the worst for sure :-)
            if (temp_best[i].first == (unsigned long)-1)
            {
                //std::cerr << "Found worst at " << i << std::endl;
                worst = i;
                break;
            }
            
            if (worst == -1 || temp_best[i].second > worst_distance)
            {
                worst = i;
                worst_distance = temp_best[i].second;
            }
        }
        //std::cerr << "distance=" << distance << ", worst_distance=" << worst_distance << std::endl;
        if (worst != -1 &&
            (temp_best[worst].first == (unsigned long)-1 || distance < worst_distance))
        {
            //std::cerr << "Substitute worst with this: " << cs->name_ << ", " << distance << std::endl;
            temp_best[worst].first = s_id;
            temp_best[worst].second = distance;
        }
    }
    
    // Fill and sort the result;
    best.clear();
    for (int i = 0; i < temp_best.size() ; ++i)
        if (temp_best[i].first != (unsigned long)-1)
            best.push_back({
                temp_best[i].first,
                temp_best[i].second
            });
    std::sort(best.begin(), best.end(), [] (typeof(best[0]) &a, typeof(best[0]) &b) { return a.second < b.second; } );
    
    std::cerr << "compact_addresses::best_street_name_match: found " << temp_best.size()
        << " variants" << std::endl;
    
    if (!best.empty())
    {
        std::cerr << "compact_addresses::best_street_name_match: the best is "
        << best[0].first << " -> " << get_street_by_street_id(best[0].first)->name_ << " , " << best[0].second << std::endl;
    }
}

void compact_addresses::best_poi_match(const std::string &search_string, const point &location,
                                                     std::vector<std::pair<unsigned long long, float> > &best) const
{
    // 1. Lower case the search_string and divide it into words
    std::string lstr = tolower(search_string);
    auto words = extractWordsVector(lstr);
    
    // A special case
    if (words.empty())
    {
        best.clear();
        return;
    }
    
    // Two-dimensional vector of POIs - for the intersection
    std::vector<std::vector<unsigned long long> > vv_ids;
    vv_ids.resize(words.size());
    
    // 2. For every word find a list of all POIs with a search object beginning with those words
    int n = 0;
    for (auto &w : words)
    {
        for (auto i = std::lower_bound(compact_search_objects_poi_.begin(),
                                        compact_search_objects_poi_.end(),
                                        w,
                                        [](const std::pair<std::string, std::vector<unsigned long long> > &a, const std::string &b)
                                        {
                                            return a.first < b;
                                        }); i != compact_search_objects_poi_.end(); ++i)
        {
            //std::cerr << "i->first='" << i->first << "'" << ", w='" << w << "'" << std::endl;
            
            // Check if w is a prefix to i->first (or is found inside i->first)
            if (i->first.find(w) == std::string::npos)
                break;
             
            vv_ids[n].insert(vv_ids[n].end(), i->second.begin(), i->second.end());
            
            /*std::cerr << " -> " << std::endl;
            
            for (auto &j : i->second)
            {
                unsigned long street_id = j >> 32;
                unsigned long house_id = j & 0x00000000ffffffffull;
                
                compact_street *cs = &compact_streets_[street_id];
                compact_house *ch = &cs->houses_[house_id];
                std::string &house_number_name = compact_house_numbers_[ch->number_id_];
                std::string &poi_name = compact_pois_[ch->poi_id_];
                
                std::cerr << "ID: " << j << ", Street ID: " << street_id << ", House ID: " << house_id <<
                    ", House number ID: " << ch->number_id_ << ", POI id: " << ch->poi_id_ <<
                ", House number name: " << house_number_name << ", POI name: " << poi_name <<
                ", Street name: '" << cs->name_ << "'" << ", lat/lon=" << ch->lat_ << "," << ch->lon_ << std::endl;
            }
            std::cerr << std::endl;*/
        }
        
        // Sort vv_ids[n] for later intersection
        std::sort(vv_ids[n].begin(), vv_ids[n].end());
        // Unique it
        vv_ids[n].erase(std::unique(vv_ids[n].begin(), vv_ids[n].end()), vv_ids[n].end());
        
        ++n;
    }
    
    // 3. Now interset ALL vv_ids with each other (normally users use 1 or 2 words - so it's not as
     //      bad as it sounds)
     auto *prev_vector = &vv_ids[0];
     std::vector<unsigned long long> result;
     for (int i = 1; i < n; ++i)
     {
         result.clear();
         std::set_intersection(prev_vector->begin(), prev_vector->end(),
                               vv_ids[i].begin(), vv_ids[i].end(),
                               std::back_inserter(result));

         std::cerr << "compact_addresses::best_poi_match: intersect two vectors: " <<
            prev_vector->size() << "," << vv_ids[i].size() << " -> " << result.size() << std::endl;
         
         result.swap(*prev_vector);

         // Now prev_vector == &vv_ids[i-1] and contains the intersection
         // of everything up to i and result has the content of previous vv_ids[i-1] which
         // we don't need anymore
     }
     
     result.swap(*prev_vector);
     
     if (result.empty())
     {
         std::cerr << "compact_addresses::best_poi_match: intersection is empty" << std::endl;
         best.clear();
         return;
     }

     std::cerr << "compact_addresses::best_poi_match: the intersection size is " << result.size() <<
        ". Searching for " << best.size() << " variants" << std::endl;
    
    std::string lower_buffer;

    // 4. Among intersected pois find nearest ONES
    for (int i = 0; i < best.size() ; ++i)
        best[i] = {(unsigned long long)-1, 1000000000};
    for (auto h_id : result)
    {
        const compact_house *ch = get_house_by_big_id(h_id);
        if (!ch)
            continue;

        float distance = (float)shortest_distance_km(ch->lat_, ch->lon_,
                                                    location.latitude, location.longitude);
        
        
        // If POI is very attractive (airport, train station) then give it a head
        // start of 100km to bubble up in the search
        // Note: for optimization don't take faraway POIs
        if (distance < 150)
        {
            auto *poi_name = get_poi_by_id(ch->poi_id_);
            if (poi_name)
            {
                tolower2(*poi_name, lower_buffer);
                if (/*(lower_buffer.find("вокзал") != std::string::npos)
                    ||*/ // Remove it becuase of lot of trash
                    (lower_buffer.find("терминал") != std::string::npos && lower_buffer.find("аэропорт") != std::string::npos) ||
                    (lower_buffer.find("международный") != std::string::npos && lower_buffer.find("аэропорт") != std::string::npos)
                    )
                    distance -= 100;
            }
        }
        
        // Check if this distance is better than the worst in the temp result
        // And if it's so then substistute the worst with that
        // It's O(result.size() * temp_best.size()) instead of O(result.size() * Log(temp_best.size())) - not the best algorithm,
        // but for small temp_best.size()s like 3 or 5 it's better than messing around with partial sort :-)
        //
        // Now there is one important thing! If there is already POI in the best list with
        // the same name but with a better distance then DON'T add this POI to the list
        // Also if this POI is BETTER than the one in the list with the same name then
        // substite it
        // Why? Because a user normally don't need lots of EXACT SAME POIs in suggestions
        // TODO: a big questions whether we have to apply this logic to grocery networks and other
        //  networks
        int to_substitute = -1;
        float worst_distance = 0;
        for (int i = 0;i < best.size(); ++i)
        {
            unsigned long long big_id = best[i].first;
            
            // Same ID - means the same name, because POI's name is stored in
            // compact_pois_ and poi_id_ is the index to that
            if (big_id != (unsigned long long)-1 &&
                    get_house_by_big_id(big_id)->poi_id_ == ch->poi_id_)
            {
                // Found the POI in the best with the same name, but current POI is better - substitute it
                if (distance < best[i].second)
                {
                    to_substitute = i;
                    //std::cerr << "Substitute POI with the same name because of the better distance, poi_id_="
                      //  << ch->poi_id_ << std::endl;
                }
                // Found the POI in the best with the same name and it's better than the current one
                // then keep it
                else
                {
                    //std::cerr << "Don't add POI to the best because the one with the same name and the better distance "
                      //  "is already there, poi_id_=" << ch->poi_id_ << std::endl;
                    to_substitute = -1;
                }
                break;
            }
            // It's the worst for sure :-)
            if (big_id == (unsigned long long)-1)
            {
                //std::cerr << "Found worst at " << i << std::endl;
                to_substitute = i;

                // Don't break! Because we can find a better substitution - the POI with the same name
            }

            if (to_substitute == -1 || best[i].second > worst_distance)
            {
                to_substitute = i;
                worst_distance = best[i].second;
            }
        }

        if (to_substitute != -1 &&
            (best[to_substitute].first == (unsigned long long)-1 || distance < worst_distance))
        {
            // std::cerr << "Substitute worst with this: " << h_id << ", " << distance << std::endl;
            best[to_substitute].first = h_id;
            best[to_substitute].second = distance;
        }
    }
    
    // Sort the result;
    std::sort(best.begin(), best.end(), [] (typeof(best[0]) &a, typeof(best[0]) &b) { return a.second < b.second; } );
    // Cut everything that is with first == -1
    // Note: it's all at the end of the list because all -1s have distance 1000000000 and
    // sorting is by distance
    // Note: it's crucial to cut right here, but not before the sort because "best" can have
    // entries with -1 here and there
    for (int i = 0;i < best.size(); ++i)
        if (best[i].first == (unsigned long long)-1)
        {
            // Cut from here
            best.resize(i);
            break;
        }
     
     std::cerr << "compact_addresses::best_poi_match: found " << best.size()
         << " variants" << std::endl;
     
     if (!best.empty())
     {
         std::cerr << "compact_addresses::best_poi_match: the best is "
         << best[0].first << ", " << best[0].second << std::endl;
     }
}

void compact_addresses::find_matching_houses_helper_(const compact_street &cs,
                                                     const std::string &house_pattern,
                                                     const point &location,
                                                     std::vector<std::pair<unsigned long long, float> > &result) const
{
    unsigned long just_street_id = &cs - &*compact_streets_.begin();
    
    std::cerr << "compact_addresses::find_matching_houses_helper_: just_street_id='"
        << just_street_id << ", street name=" << cs.name_ << ", house_pattern='" << house_pattern << "'" << std::endl;

    // Use the binary search for a house
    for (auto h = std::lower_bound(cs.houses_.begin(), cs.houses_.end(), house_pattern,
                               
                               // Put this predicate specifically here as a lambda to always
                               // be aware of the search complexity O(Log(houses per street)*Log(houses total))
                               [this](const compact_house &a, const std::string &s){
                                   // O(Log(total number of houses))
    return *get_house_number_by_house_number_id(a.number_id_) < s;
    }); h != cs.houses_.end(); ++h)
    {
        // Check if the pattern still matches
        if (get_house_number_by_house_number_id(h->number_id_)->find(house_pattern) == std::string::npos)
            break;
        
        std::cerr << "compact_addresses::find_matching_houses_helper_: found a house '" << *get_house_number_by_house_number_id(h->number_id_) << "'" << std::endl;
            
        float distance = (float)shortest_distance_km(h->lat_, h->lon_,
                                                    location.latitude, location.longitude);
        
        // Constuct the id of the house as a place
        unsigned long long big_id = get_combined_place_id(just_street_id, h - cs.houses_.begin());
        
        result.push_back({big_id, distance});
    }
}

void compact_addresses::best_house_match(const std::string &search_string,
                                               const point &location,
                                               std::vector<std::pair<unsigned long long, float> > &best) const
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long sec1 = tv.tv_sec * 1000000 + tv.tv_usec;

    // 1.   Consider the search_string as an EXACT street name + something and try to find
    //      a house in the street. Saves all the matches and distnances to that
    
    // Cut char after a char from the back of search_string util we find an exact street name
    std::string cut_string = search_string, house_pattern;
    while (!cut_string.empty())
    {
        auto cs = std::lower_bound(compact_streets_.begin(), compact_streets_.end(), cut_string);
        
        if (cs != compact_streets_.end() && cs->name_ == cut_string)
        {
            std::cerr << "compact_addresses::best_house_match: cut_string='"
                << cut_string << "'" << std::endl;
            
            // Got an EXACT street name! Now consider the remainder of a search string as a house name
            // and search for a house
            
            // Cut aleading divider
            int cut_divider = (search_string[cut_string.size()] == ' ' || search_string[cut_string.size()] == ',')?1:0;
            house_pattern.resize(search_string.size() - cut_string.size() - cut_divider);
            std::copy(search_string.begin() + cut_string.size() + cut_divider,
                      search_string.end(), house_pattern.begin());
            
            std::cerr << "compact_addresses::best_house_match: house_pattern='"
                << house_pattern << "'" << std::endl;
            
            // Search for matching houses and fill "best"
            // It's O(Log(houses per street)*Log(houses total))
            find_matching_houses_helper_(*cs, house_pattern, location, best);

            break;
        }
        
        cut_string.resize(cut_string.size() - 1);
    }
    
    // 2.  Take every word containing ONLY numbers and sequentially search for
    //      streets using a search_string EXCEPT that word
    
    std::cerr << "compact_addresses::best_house_match: stage 2" << std::endl;
    
    std::string lstr = tolower(search_string);
    auto words = extractWordsVector(lstr);
    std::string new_search_string;
    
    int i = 0;
    for (auto &alpha_word : words)
    {
        bool alpha = true;
        for (auto c : alpha_word)
            if (c < '0' || c > '9')
            {
                alpha = false;
                break;
            }
        if (alpha)
        {
            std::cerr << "compact_addresses::best_house_match: stage 2, alpha found: '" << alpha_word << "'" << std::endl;
            
            // Now construct a new search string EXCEPT this word
            new_search_string.clear();
            for (auto &w2 : words)
                if (&w2 - &*words.begin() != i)
                {
                    if (!new_search_string.empty())
                        new_search_string += " ";
                    new_search_string += w2;
                }
            
            // Do the search for this new string
            // TODO: change 100 with something better
            std::vector<std::pair<unsigned long, float> > streets(10);
            best_street_name_match(new_search_string, location, streets);
            
            // For each street in "streets" find all houses that match alpha_word
            for (auto cs : streets)
                find_matching_houses_helper_(*get_street_by_street_id(cs.first),
                                                alpha_word, location, best);
        }
        ++i;
    }
    
    std::sort(best.begin(), best.end(), [] (typeof(best[0]) &a, typeof(best[0]) &b) { return a.second < b.second; } );
    
    gettimeofday(&tv, NULL);
    long long sec2 = tv.tv_sec * 1000000 + tv.tv_usec;

    std::cerr << "compact_addresses::best_house_match has taken " << (sec2 - sec1)/1000 << " miliseconds,"
        << "result size: " << best.size() << std::endl;
}

void compact_addresses::best_match(const point &location,
                                   std::vector<std::pair<unsigned long long, float> > &best) const
{
    int resolution = 10;
    int ring_size = 3;
    
    // Find nearest house ids
    std::unordered_set<unsigned long long> house_ids;
    if (!house_geo_locations_.get_objects_in_radius_better_version(location, resolution, ring_size, house_ids))
    {
        std::cerr << "compact_addresses::get_houses_near_location debug: get_objects_in_radius failed" << std::endl;
        return;
    }
    
    //std::cerr << "addresses::get_houses_near_location debug: get_objects_in_radius returned " << house_ids.size()
    //    << " house ids" << std::endl;

    double best_distance;
    int best_house = -1;
    for (auto big_id : house_ids)
    {
        const compact_house *ch = get_house_by_big_id(big_id);
        
        if (!ch)
        {
            std::cerr << "compact_addresses::best_match: can't find house or street by id: " << big_id << std::endl;
            continue;
        }
        best.push_back({big_id, shortest_distance_km(location, {.longitude = ch->lon_, .latitude = ch->lat_})});
    }
    
    std::sort(best.begin(), best.end(), [] (typeof(best[0]) &a, typeof(best[0]) &b) { return a.second < b.second; } );
}

} // namespace geo
