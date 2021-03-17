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

#include "geo_core.h"

namespace geo
{

unsigned char *StrToLwrExt(unsigned char *pString)
{
    if (pString && *pString) {
        unsigned char *p = pString;
        unsigned char *pExtChar = 0;
        while (*p) {
            if ((*p >= 0x41) && (*p <= 0x5a)) // US ASCII
                (*p) += 0x20;
            else if (*p > 0xc0) {
                pExtChar = p;
                p++;
                switch (*pExtChar) {
                case 0xc3: // Latin 1
                    if ((*p >= 0x80)
                        && (*p <= 0x9e)
                        && (*p != 0x97))
                        (*p) += 0x20; // US ASCII shift
                    break;
                case 0xc4: // Latin Exteneded
                    if ((*p >= 0x80)
                        && (*p <= 0xb7)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    else if ((*p >= 0xb9)
                        && (*p <= 0xbe)
                        && (*p % 2)) // Odd
                        (*p)++; // Next char is lwr
                    else if (*p == 0xbf) {
                        *pExtChar = 0xc5;
                        (*p) = 0x80;
                    }
                    break;
                case 0xc5: // Latin Exteneded
                    if ((*p >= 0x80)
                        && (*p <= 0x88)
                        && (*p % 2)) // Odd
                        (*p)++; // Next char is lwr
                    else if ((*p >= 0x8a)
                        && (*p <= 0xb7)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    else if ((*p >= 0xb9)
                        && (*p <= 0xbe)
                        && (*p % 2)) // Odd
                        (*p)++; // Next char is lwr
                    break;
                case 0xc6: // Latin Exteneded
                    switch (*p) {
                    case 0x82:
                    case 0x84:
                    case 0x87:
                    case 0x8b:
                    case 0x91:
                    case 0x98:
                    case 0xa0:
                    case 0xa2:
                    case 0xa4:
                    case 0xa7:
                    case 0xac:
                    case 0xaf:
                    case 0xb3:
                    case 0xb5:
                    case 0xb8:
                    case 0xbc:
                        (*p)++; // Next char is lwr
                        break;
                    default:
                        break;
                    }
                    break;
                case 0xc7: // Latin Exteneded
                    if (*p == 0x84)
                        (*p) = 0x86;
                    else if (*p == 0x85)
                        (*p)++; // Next char is lwr
                    else if (*p == 0x87)
                        (*p) = 0x89;
                    else if (*p == 0x88)
                        (*p)++; // Next char is lwr
                    else if (*p == 0x8a)
                        (*p) = 0x8c;
                    else if (*p == 0x8b)
                        (*p)++; // Next char is lwr
                    else if ((*p >= 0x8d)
                        && (*p <= 0x9c)
                        && (*p % 2)) // Odd
                        (*p)++; // Next char is lwr
                    else if ((*p >= 0x9e)
                        && (*p <= 0xaf)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    else if (*p == 0xb1)
                        (*p) = 0xb3;
                    else if (*p == 0xb2)
                        (*p)++; // Next char is lwr
                    else if (*p == 0xb4)
                        (*p)++; // Next char is lwr
                    else if (*p == 0xb8)
                        (*p)++; // Next char is lwr
                    else if (*p == 0xba)
                        (*p)++; // Next char is lwr
                    else if (*p == 0xbc)
                        (*p)++; // Next char is lwr
                    else if (*p == 0xbe)
                        (*p)++; // Next char is lwr
                    break;
                case 0xc8: // Latin Exteneded
                    if ((*p >= 0x80)
                        && (*p <= 0x9f)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    else if ((*p >= 0xa2)
                        && (*p <= 0xb3)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    else if (*p == 0xbb)
                        (*p)++; // Next char is lwr
                    break;
                case 0xcd: // Greek & Coptic
                    switch (*p) {
                    case 0xb0:
                    case 0xb2:
                    case 0xb6:
                        (*p)++; // Next char is lwr
                        break;
                    default:
                        if (*p == 0xbf) {
                            *pExtChar = 0xcf;
                            (*p) = 0xb3;
                        }
                        break;
                    }
                    break;
                case 0xce: // Greek & Coptic
                    if (*p == 0x86)
                        (*p) = 0xac;
                    else if (*p == 0x88)
                        (*p) = 0xad;
                    else if (*p == 0x89)
                        (*p) = 0xae;
                    else if (*p == 0x8a)
                        (*p) = 0xaf;
                    else if (*p == 0x8c) {
                        *pExtChar = 0xcf;
                        (*p) = 0x8c;
                    }
                    else if (*p == 0x8e) {
                        *pExtChar = 0xcf;
                        (*p) = 0x8d;
                    }
                    else if (*p == 0x8f) {
                        *pExtChar = 0xcf;
                        (*p) = 0x8e;
                    }
                    else if ((*p >= 0x91)
                        && (*p <= 0x9f))
                        (*p) += 0x20; // US ASCII shift
                    else if ((*p >= 0xa0)
                        && (*p <= 0xab)
                        && (*p != 0xa2)) {
                        *pExtChar = 0xcf;
                        (*p) -= 0x20;
                    }
                    break;
                case 0xcf: // Greek & Coptic
                    if (*p == 0x8f)
                        (*p) = 0xb4;
                    else if (*p == 0x91)
                        (*p)++; // Next char is lwr
                    else if ((*p >= 0x98)
                        && (*p <= 0xaf)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    else if (*p == 0xb4)
                        (*p) = 0x91;
                    else if (*p == 0xb7)
                        (*p)++; // Next char is lwr
                    else if (*p == 0xb9)
                        (*p) = 0xb2;
                    else if (*p == 0xbb)
                        (*p)++; // Next char is lwr
                    else if (*p == 0xbd) {
                        *pExtChar = 0xcd;
                        (*p) = 0xbb;
                    }
                    else if (*p == 0xbe) {
                        *pExtChar = 0xcd;
                        (*p) = 0xbc;
                    }
                    else if (*p == 0xbf) {
                        *pExtChar = 0xcd;
                        (*p) = 0xbd;
                    }

                    break;
                case 0xd0: // Cyrillic
                    if ((*p >= 0x80)
                        && (*p <= 0x8f)) {
                        *pExtChar = 0xd1;
                        (*p) += 0x10;
                    }
                    else if ((*p >= 0x90)
                        && (*p <= 0x9f))
                        (*p) += 0x20; // US ASCII shift
                    else if ((*p >= 0xa0)
                        && (*p <= 0xaf)) {
                        *pExtChar = 0xd1;
                        (*p) -= 0x20;
                    }
                    break;
                case 0xd1: // Cyrillic supplement
                    if ((*p >= 0xa0)
                        && (*p <= 0xbf)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    break;
                case 0xd2: // Cyrillic supplement
                    if (*p == 0x80)
                        (*p)++; // Next char is lwr
                    else if ((*p >= 0x8a)
                        && (*p <= 0xbf)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    break;
                case 0xd3: // Cyrillic supplement
                    if ((*p >= 0x81)
                        && (*p <= 0x8e)
                        && (*p % 2)) // Odd
                        (*p)++; // Next char is lwr
                    else if ((*p >= 0x90)
                        && (*p <= 0xbf)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    break;
                case 0xd4: // Cyrillic supplement & Armenian
                    if ((*p >= 0x80)
                        && (*p <= 0xaf)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    else if ((*p >= 0xb1)
                        && (*p <= 0xbf)) {
                        *pExtChar = 0xd5;
                        (*p) -= 0x10;
                    }
                    break;
                case 0xd5: // Armenian
                    if ((*p >= 0x80)
                        && (*p <= 0x96)
                        && (!(*p % 2))) // Even
                        (*p)++; // Next char is lwr
                    break;
                case 0xe1: // Three byte code
                    pExtChar = p;
                    p++;
                    switch (*pExtChar) {
                    case 0x82: // Georgian
                        if ((*p >= 0xa0)
                            && (*p <= 0xbf)) {
                            *pExtChar = 0x83;
                            (*p) -= 0x10;
                        }
                        break;
                    case 0x83: // Georgian
                        if (((*p >= 0x80)
                            && ((*p <= 0x85)
                                || (*p == 0x87)))
                            || (*p == 0x8d))
                            (*p) += 0x30;
                        break;
                    case 0xb8: // Latin extened
                        if ((*p >= 0x80)
                            && (*p <= 0xbf)
                            && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        break;
                    case 0xb9: // Latin extened
                        if ((*p >= 0x80)
                            && (*p <= 0xbf)
                            && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        break;
                    case 0xba: // Latin extened
                        if ((*p >= 0x80)
                            && (*p <= 0x94)
                            && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0x9e)
                            && (*p <= 0xbf)
                            && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        break;
                    case 0xbb: // Latin extened
                        if ((*p >= 0x80)
                            && (*p <= 0xbf)
                            && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        break;
                    case 0xbc: // Greek extened
                        if ((*p >= 0x88)
                            && (*p <= 0x8f))
                            (*p) -= 0x08;
                        else if ((*p >= 0x98)
                            && (*p <= 0x9f))
                            (*p) -= 0x08;
                        else if ((*p >= 0xa8)
                            && (*p <= 0xaf))
                            (*p) -= 0x08;
                        else if ((*p >= 0xb8)
                            && (*p <= 0x8f))
                            (*p) -= 0x08;
                        break;
                    case 0xbd: // Greek extened
                        if ((*p >= 0x88)
                            && (*p <= 0x8d))
                            (*p) -= 0x08;
                        else if ((*p >= 0x98)
                            && (*p <= 0x9f))
                            (*p) -= 0x08;
                        else if ((*p >= 0xa8)
                            && (*p <= 0xaf))
                            (*p) -= 0x08;
                        else if ((*p >= 0xb8)
                            && (*p <= 0x8f))
                            (*p) -= 0x08;
                        break;
                    case 0xbe: // Greek extened
                        if ((*p >= 0x88)
                            && (*p <= 0x8f))
                            (*p) -= 0x08;
                        else if ((*p >= 0x98)
                            && (*p <= 0x9f))
                            (*p) -= 0x08;
                        else if ((*p >= 0xa8)
                            && (*p <= 0xaf))
                            (*p) -= 0x08;
                        else if ((*p >= 0xb8)
                            && (*p <= 0xb9))
                            (*p) -= 0x08;
                        break;
                    case 0xbf: // Greek extened
                        if ((*p >= 0x88)
                            && (*p <= 0x8c))
                            (*p) -= 0x08;
                        else if ((*p >= 0x98)
                            && (*p <= 0x9b))
                            (*p) -= 0x08;
                        else if ((*p >= 0xa8)
                            && (*p <= 0xac))
                            (*p) -= 0x08;
                        break;
                    default:
                        break;
                    }
                    break;
                case 0xf0: // Four byte code
                    pExtChar = p;
                    p++;
                    switch (*pExtChar) {
                    case 0x90:
                        pExtChar = p;
                        p++;
                        switch (*pExtChar) {
                        case 0x92: // Osage
                            if ((*p >= 0xb0)
                                && (*p <= 0xbf)) {
                                *pExtChar = 0x93;
                                (*p) -= 0x18;
                            }
                            break;
                        case 0x93: // Osage
                            if ((*p >= 0x80)
                                && (*p <= 0x93))
                                (*p) += 0x18;
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                pExtChar = 0;
            }
            p++;
        }
    }
    return pString;
}

std::string tolower(const std::string &s)
{
    std::vector<unsigned char> v; for (auto i : s) v.push_back((unsigned char)i);
    v.push_back(0);
    StrToLwrExt(&v[0]);
    v.resize(v.size()-1);
    std::string lwcharstr;for (auto i : v) lwcharstr += (char)i;
    return lwcharstr;
}

void tolower2(const std::string &s, std::string &result)
{
    std::vector<unsigned char> v;
    v.resize(s.size() + 1);
    v[s.size()] = 0;
    std::copy(s.begin(), s.end(), v.begin());
 
    StrToLwrExt(&v[0]);
 
    result.resize(s.size());
    std::copy(v.begin(), v.begin() + s.size(), result.begin());
}


template <typename OutputIterator>
void extractWords(std::string const& s, OutputIterator out)
{
    static auto const isSpace = [](char letter){ return letter == ' ' || letter == '(' || letter == '-'; };

    auto beginWord = std::find_if_not(begin(s), end(s), isSpace);

    while (beginWord != end(s))
    {
        auto const endWord = std::find_if(beginWord, end(s), isSpace);
        if (beginWord != endWord)
        {
            *out = std::string(beginWord, endWord);
            ++out;
            beginWord = endWord;
        }
        else
            ++beginWord;
    }
}

std::vector<std::string> extractWordsVector(std::string const& s)
{
    std::vector<std::string> results;
    extractWords(s, back_inserter(results));
    return results;
}

// 12 - 9 meters - we don't need a finer resolution because there are normally
//  a small number of houses within 9 meters
// 6 - 3 km - we don't need a coarser resolution because houses normally aren't that big :-)
// Note! Changed 6 to 9 - otherwise too much RAM (28.08.2020) it's 175 meters
// Note! Changed 9 - 10 - 175 meters -> 65 ,meters - all to save RAM
addresses::addresses() : house_geo_locations_(/*6*/ 9, 10/*12*/)
{
}

void addresses::upload_street(const std::vector<std::string> &street_components,
                       const std::string &street_name,
                       long long street_id)
{
    // Fill search objects for streets with
    // a) street name itself
    // b) all components of street name
    // c) all words of each component
    for (const auto &c : street_components)
    {
        // Divide c into words
        auto v = extractWordsVector(c);
            
        for (const auto &i : v)
            search_objects_streets_.insert({tolower(i), {street_id, street_name}});
        search_objects_streets_.insert({tolower(c), {street_id, street_name}});
    }
    search_objects_streets_.insert({tolower(street_name), {street_id, street_name}});

    // Save street along with its id - this is for houses
    // Note! We don't lower street here because it's it real name
    streets_.insert({street_id, {street_name, {}}});
}

void divide_into_words(const std::vector<std::string> &components,
                       const std::string &name,
                       std::vector<std::string> &result)
{
    for (const auto &c : components)
    {
        // Divide c into words
        auto v = extractWordsVector(c);
            
        for (const auto &i : v)
            result.push_back(tolower(i) + "$" + name);
        result.push_back(tolower(c) + "$" + name);
    }
}

void addresses::upload_poi(const std::vector<std::string> &poi_components,
                            const std::string &poi_name,
                            const std::string &full_address)
{
    // Try it find a house connected to this poi
    // Note: we need a house becuase it's the only way to get poi's coordinate
    // Note: it's ok to lower the full address because it's lowerd in the full_address_to_house_id_ map
    auto h = full_address_to_house_id_.find(tolower(full_address));
    
    // No house - no poi
    if (h == full_address_to_house_id_.end())
        return;
    
    long long house_id = h->second;
    
    // Fill search objects for poi with
    // a) poi name itself
    // b) all components of poi name
    // c) all words of each component
    for (const auto &c : poi_components)
    {
        // Divide c into words
        auto v = extractWordsVector(c);
            
        for (const auto &i : v)
            search_objects_poi_.insert({tolower(i), {house_id, poi_name}});
        search_objects_poi_.insert({tolower(c), {house_id, poi_name}});
    }
    search_objects_poi_.insert({tolower(full_address), {house_id, poi_name}});
}

/*bool addresses::upload_house(long long street_id, long long house_id, house &&h)
{
    if (street_id == -1 || house_id == -1)
        return false;
    
    // Check if the house is already there
    if (houses_.find(house_id) != houses_.end())
        return false;
    
    // Construct house full address
    h.full_address_ = streets_[street_id].name_ + " " + h.number_;
    
    // Save the house in the database
    houses_.insert({house_id, h});
    
    // Add house's id to the street
    streets_[street_id].house_ids_.push_back(house_id);
    
    // Add the house to the house map
    // Note! We lower the full address because of case insensitive search. The caller MUST use the
    // normail address from the house structure
    full_address_to_house_id_.insert({tolower(h.full_address_), house_id});
    
    // Add the house to the geocoding db
    house_geo_locations_.update_object_position(house_id, NULL, h.house_position_);
    
    return true;
}*/

std::vector<std::pair<int, std::string> > addresses::auto_complete_street(const std::string &str_)
{
    std::string lstr = tolower(str_);
        
    std::vector<std::pair<int, std::string> > res;
        
    // Typo check: TODO
    while (true)
    {
        auto_complete_street_impl(lstr, res);
        if (!res.empty())
            return res;

        // TODO
        break;
    }
        
    return res;
}
    
void addresses::auto_complete_street_impl(const std::string &lstr, std::vector<std::pair<int, std::string> > &result)
{
    // Binary search
    int n = 0;
    for (auto i = search_objects_streets_.lower_bound(lstr); i != search_objects_streets_.end(); ++i)
    {
        if (i->first.find(lstr) == std::string::npos)
            break;
        result.push_back(i->second);
        
        // Limit result with 100 items
        if (++n >= 100)
            break;
    }
}

std::vector<house> addresses::auto_complete_house(const std::string &str_)
{
    std::vector<house> result;
    
    std::string lstr = tolower(str_);

    // Binary search
    int n = 0;
    for (auto i = full_address_to_house_id_.lower_bound(lstr); i != full_address_to_house_id_.end(); ++i)
    {
        //std::cout << "auto_complete_house: " << lstr << " " << i->first << std::endl;
        
        // Found everything prefixed with str_
        if (i->first.find(lstr) == std::string::npos)
            break;
        
        // i->second is the house id - find a house by this id
        auto house_i = houses_.find(i->second);
        if (house_i == houses_.end())
        {
            // Corrupted data!!!
            std::cerr << "addresses::auto_complete_house: debug: house_id " << i->second <<
            " exists in full_address_to_house_id_ but does not exist in houses_!!!" << std::endl;
            return result;
        }
        
        // Save this house in the results
        result.push_back(house_i->second);
        
        // Limit result with 100 items
        if (++n >= 100)
            break;
    }
    
    return result;
}

std::vector<std::pair<house, std::string> > addresses::auto_complete_house_by_poi(const std::string &str_)
{
    std::vector<std::pair<house, std::string> > result;
        
    std::string lstr = tolower(str_);
        
    // Binary search in pois
    int n = 0;
    for (auto i = search_objects_poi_.lower_bound(lstr); i != search_objects_poi_.end(); ++i)
    {
        // Found everything prefixed with str_
        if (i->first.find(lstr) == std::string::npos)
            break;
            
        // i->second is {house id, poi_name} - find a house by this id
        auto house_i = houses_.find(i->second.first);
        if (house_i == houses_.end())
        {
            // Corrupted data!!!
            std::cerr << "addresses::auto_complete_house_by_poi: debug: house_id " << i->second.first <<
                " exists in full_address_to_house_id_ but does not exist in houses_!!!" << std::endl;
            return result;
        }
            
        // Save this {house, poi_name} in the results
        // Note: i is an iterator that points to the pair, the second of this pair is
        //   std::pair<long long, std::string> which is {house_id, poi_name}
        //   so convert it to {house, poi_name}
        result.push_back({house_i->second, i->second.second});
            
        // Limit result with 100 items
        if (++n >= 100)
            break;
    }
        
    return result;
}
    
/*std::vector<house> addresses::get_houses_near_location(const point &location, int resolution, int ring_size)
{
    // Find nearest house ids
    std::unordered_set<long long> house_ids;
    if (!house_geo_locations_.get_objects_in_radius(location, resolution, ring_size, house_ids))
    {
        std::cerr << "addresses::get_houses_near_location debug: get_objects_in_radius failed" << std::endl;
        return {};
    }
    
    //std::cerr << "addresses::get_houses_near_location debug: get_objects_in_radius returned " << house_ids.size()
    //    << " house ids" << std::endl;

    // Convert ids to houses
    std::vector<house> result;
    for (auto &i : house_ids)
    {
        auto h_i = houses_.find(i);
        if (h_i == houses_.end())
            std::cerr << "addresses::get_houses_near_location debug: bad house id " << i << std::endl;
        else
            result.push_back(h_i->second);
    }
    
    return result;
}*/

int addresses::get_nearest_house(const point &location, const std::vector<house> &houses)
{
    double best_distance;
    int best_house = -1;
    int i = 0;
    for (const auto &h : houses)
    {
        double distance = shortest_distance_km(location, h.house_position_);
        if (best_house == -1 || distance < best_distance)
        {
            best_house = i;
            best_distance = distance;
        }
        ++i;
    }
    
    return best_house;
}

void addresses::print_stat()
{
    std::cerr << "addresses::print_stat: search_objects_streets_.size()=" << search_objects_streets_.size() << std::endl;

    long long total_search_objects_streets_size = 0;
    for (auto &i : search_objects_streets_)
        total_search_objects_streets_size += i.first.size() + sizeof(int) + i.second.second.size();
    
    std::cerr << "addresses::print_stat: search_objects_streets_ approx total size=" <<
        total_search_objects_streets_size << std::endl;

    std::cerr << "addresses::print_stat: search_objects_poi_.size()=" << search_objects_poi_.size() << std::endl;
    
    long long total_search_objects_poi_size = 0;
    for (auto &i : search_objects_poi_)
        total_search_objects_poi_size += i.first.size() + sizeof(long long) + i.second.second.size();
    
    std::cerr << "addresses::print_stat: search_objects_poi_ approx total size=" << total_search_objects_poi_size << std::endl;
    
    std::cerr << "addresses::print_stat: streets_.size()=" << streets_.size() << std::endl;

    long long total_streets_size = 0;
    for (auto &i : streets_)
        total_streets_size += sizeof(long long) + i.second.name_.size() + i.second.house_ids_.size() * sizeof(long long);

   std::cerr << "addresses::print_stat: streets_ approx total size=" << total_streets_size << std::endl;
   
    std::cerr << "addresses::print_stat: houses_.size()=" << houses_.size() << std::endl;
    
    long long total_houses_size = 0;
    for (auto &h : houses_)
        total_houses_size += h.second.number_.size() + h.second.full_address_.size()
        + sizeof(double)*4 + sizeof(long long)*2;

    std::cerr << "addresses::print_stat: houses_ approx total size=" << total_houses_size << std::endl;
    
     std::cerr << "addresses::print_stat: full_address_to_house_id_.size()=" << full_address_to_house_id_.size() << std::endl;
    
    long long total_full_address_to_house_id_size = 0;
    for (auto &i : full_address_to_house_id_)
        total_full_address_to_house_id_size += i.first.size() + sizeof(long long);

    std::cerr << "addresses::print_stat: full_address_to_house_id_ approx total size=" << total_full_address_to_house_id_size << std::endl;
    
    // Geo coding database for houses
    house_geo_locations_.print_stat();
}


/*
 *      New compact way of dealing with addresses
 */


// 12 - 9 meters - we don't need a finer resolution because there are normally
//  a small number of houses within 9 meters
// 6 - 3 km - we don't need a coarser resolution because houses normally aren't that big :-)
// Note! Changed 6 to 9 - otherwise too much RAM (28.08.2020) it's 175 meters
// Note! Changed 9 - 10 - 175 meters -> 65 ,meters - all to save RAM
// Note! Changed 10 - 10 - 65 ,meters - all to save RAM
compact_addresses::compact_addresses() : house_geo_locations_(/*6*/ 10, 10)
{
}

void compact_addresses::upload_streets_once(const std::set<std::string> &street_names)
{
    std::map<std::string, std::vector<unsigned long>> temp;
    
    // Reserve and shrink to fit in advance
    // And do the same with internals of comtact_streets_
    compact_streets_.resize(street_names.size());
    compact_streets_.shrink_to_fit();
    
    // Street id is just a sequential number in compact_streets_
    unsigned long street_id = 0;
    
    for (auto &i : street_names)
    {
        const std::string &name = i;
        std::string &target_name = compact_streets_[street_id].name_;
        
        // Save street name as it is
        // We need the name to form the full address
        target_name.resize(name.size());
        target_name.shrink_to_fit();
        std::copy(name.begin(), name.end(), target_name.begin());
        
        // Divide the street name into words and save it in the
        // temporary map
        auto v = extractWordsVector(name);

        // Save each word + the name of the street - all in the lowercase
        for (const auto &i : v)
        {
            auto e = temp.find(tolower(i));
            if (e == temp.end())
                temp.insert({tolower(i), {street_id}});
            else
                e->second.push_back(street_id);
        }
        temp.insert({tolower(name), {street_id}});
        
        ++street_id;
    }
    
    // Now convert a map to a sorted vector
    // Why a sorted vector? It takes less RAM and searches faster than a map
    
    // Reserve and shrink to fit in advance
    // And the same to both container elements of the vector - all to save RAM
    compact_search_objects_streets_.resize(temp.size());
    compact_search_objects_streets_.shrink_to_fit();
    
    size_t n = 0;
    for (auto &m : temp)
    {
        std::string &word = compact_search_objects_streets_[n].first;
        std::vector<unsigned long> &ids = compact_search_objects_streets_[n].second;
        
        word.resize(m.first.size());
        word.shrink_to_fit();
        ids.resize(m.second.size());
        ids.shrink_to_fit();
        
        std::copy(m.first.begin(), m.first.end(), word.begin());
        std::copy(m.second.begin(), m.second.end(), ids.begin());
        n++;
    }
}
 
void compact_addresses::upload_house_numbers_once(const std::set<std::string> &house_numbers)
{
    // Resize and shrink to fit in advance
    // And do the same to the internal string
    compact_house_numbers_.resize(house_numbers.size());
    compact_house_numbers_.shrink_to_fit();

    // Iterate all houses in the set and save the house number as it is
    size_t n = 0;
    for (auto &i : house_numbers)
    {
        compact_house_numbers_[n].resize(i.size());
        compact_house_numbers_[n].shrink_to_fit();
        std::copy(i.begin(), i.end(), compact_house_numbers_[n].begin());
        ++n;
    }
}

void compact_addresses::upload_pois_once(const std::set<std::string> &pois)
{
    compact_pois_.resize(pois.size());
    compact_pois_.shrink_to_fit();
    size_t n = 0;
    for (auto &p : pois)
    {
        compact_pois_[n].resize(p.size());
        compact_pois_[n].shrink_to_fit();
        std::copy(p.begin(), p.end(), compact_pois_[n].begin());
        ++n;
    }
}

unsigned long compact_addresses::get_house_id_by_number(const std::string &number)
{
    auto i = std::lower_bound(compact_house_numbers_.begin(), compact_house_numbers_.end(), number);
    if (i != compact_house_numbers_.end() && *i == number)
        return i - compact_house_numbers_.begin();
    else
    {
        std::cerr << "compact_addresses::get_house_id_by_number: WRONG HOUSE NUMBER!!! '" << number << "'" << std::endl;
        return (unsigned long)-1;
    }
}

void compact_addresses::upload_house(const std::string &poi,
                    const std::string &street,
                    const std::string &house,
                    float lat,
                    float lon)
{
    // Search for a street by EXACT name in the sorted vector
    auto it = std::lower_bound(compact_streets_.begin(),
                               compact_streets_.end(),
                               street);
    if (it == compact_streets_.end() || it->name_ != street)
    {
        std::cerr << "compact_addresses::upload_house could not find a street! street: '" << street << "'" << std::endl;
        return;
    }

    // Search for POI by EXACT name is the sorted vector
    // Note: POIs have been uploaded and sorted up to the moment
    // Note: There is nothing wrong with an empty POI - it's a house without a POI
    unsigned long poi_id = (unsigned long)-1;
    if (!poi.empty())
    {
        auto poi_it = std::lower_bound(compact_pois_.begin(),
                                    compact_pois_.end(),
                                    poi);
        if (poi_it == compact_pois_.end() || *poi_it != poi)
        {
            std::cerr << "compact_addresses::upload_house could not find POI! POI: '" << poi << "'" << std::endl;
            return;
        }
        
        poi_id = poi_it - compact_pois_.begin();
    }
    
    // Create an entry for a house
    compact_house h = {.number_id_ = get_house_id_by_number(house),
                        .poi_id_ = poi_id,
                        .lat_ = lat,
                        .lon_ = lon};
    
    // Save the house name ID in the street
    // Note! House ids in this vector are not unique with is fine because streets with same name
    //  naturally merge in a SINGLE street with ALL the houses from BOTH. Those houses will have
    //  same numbers but different coordinate
    it->houses_.push_back(h);

    // Note: later we need to sort it!
    // It MUST be done by sort_uploaded_houses
    // Note: the house id as a seq number in the vector
    //  can be used only after the sorting is done!
}

void compact_addresses::sort_uploaded_pois(const std::map<std::string, std::vector<unsigned long long> > &temp_poi_search_word_to_house_id)
{
    size_t biggest_number_of_poi_per_search_object = (size_t)-1;
    std::string biggest_poi_search_object;

    // We know the size in advance - so reserve and shrink to fit in advance for not to
    // move huge blocks of RAM around :-)
    compact_search_objects_poi_.resize(temp_poi_search_word_to_house_id.size());
    compact_search_objects_poi_.shrink_to_fit();
    
    std::cerr << "compact_addresses::sort_uploaded_pois: compact_search_objects_poi_.shrink_to_fit() done" << std::endl;

    // Save sorted pois
    size_t n = 0;
    for (auto &i : temp_poi_search_word_to_house_id)
    {
        if (biggest_number_of_poi_per_search_object == (size_t)-1 || i.second.size() > biggest_number_of_poi_per_search_object)
        {
            biggest_number_of_poi_per_search_object = i.second.size();
            biggest_poi_search_object = i.first;
        }
        
        // Do this to avoid wasteful allocations and save RAM
        std::pair<std::string, std::vector<unsigned long long> > &search_object =
            compact_search_objects_poi_[n];
        
        search_object.first.resize(i.first.size());
        search_object.first.shrink_to_fit();
        search_object.second.resize(i.second.size());
        search_object.second.shrink_to_fit();
        
        std::copy(i.first.begin(), i.first.end(), search_object.first.begin());
        std::copy(i.second.begin(), i.second.end(), search_object.second.begin());

        ++n;
    }
    
    std::cerr << "compact_addresses::sort_uploaded_pois: biggest_number_of_poi_per_search_object="
        << biggest_number_of_poi_per_search_object << ", biggest_poi_search_object='"
        << biggest_poi_search_object << "'" << std::endl;
}

void compact_addresses::sort_uploaded_houses(std::map<std::string, std::vector<unsigned long long> > &temp_poi_search_word_to_house_id)
{
    size_t biggest_number_of_houses_per_street = (size_t)-1;
    std::string biggest_street;
    
    // Reserve and shrink to fit in advance
    size_t geo_loc_size = 0;
    for (auto &s : compact_streets_)
        geo_loc_size += s.houses_.size();
    
    // Note: we  need to multiply geo_loc_size by number of entries per geo point
    // Because each point can be stored in a number of resolutions in the geolocation index
    geo_loc_size *= house_geo_locations_.get_resolution_spread();
    
    house_geo_locations_.resize_and_shrink_to_fit(geo_loc_size);
    
    // Iterate all streets and sort houses
    unsigned long street_id = 0;
    size_t geo_location_n = 0;
    for (auto &s : compact_streets_)
    {
        if (geo_location_n >= geo_loc_size)
        {
            std::cerr << "compact_addresses::sort_uploaded_houses: BAD geo_location_n!!! " <<
                geo_location_n << std::endl;
            break;
        }
        
        // TODO! This must be done through resize in advance! Otherwise it may not be shrinked
        s.houses_.shrink_to_fit();
        
        if (biggest_number_of_houses_per_street == (size_t)-1 || s.houses_.size() > biggest_number_of_houses_per_street)
        {
            biggest_number_of_houses_per_street = s.houses_.size();
            biggest_street = s.name_;
        }
        
        // Note! The only reason we sort houses is to have a quick search by a house number
        // in auto completion
        // But the thing is that the complexity will be
        // O(Log(number of houses per street) * (Log(total number of houses))
        // It's a big question whether it's better than to full scan a street. But still sorting is done once
        // So if it's better to do this kind of search than the full scan then it'll help
        std::sort(s.houses_.begin(), s.houses_.end(),
                  [this](const compact_house &a, const compact_house &b){
                    // O(Log(total number of houses))
                    return *get_house_number_by_house_number_id(a.number_id_) < *get_house_number_by_house_number_id(b.number_id_);
                });
        
        // Now that we have a good id of a house - update its position in a geolocation db
        //
        // It's not the end of the show :-)
        // Also now that we have good house ids we will link it to the POI through the map
        unsigned long house_id = 0;
        for (auto &h : s.houses_)
        {
            house_geo_locations_.update_object_position_better_version(geo_location_n,
                                                        get_combined_place_id(street_id, house_id),
                                                                       {.longitude = h.lon_, .latitude = h.lat_});
             // Note: we increase this number by the spread because update_object_position_better_version
             // unser the hood updates an index per a resolution
             geo_location_n += house_geo_locations_.get_resolution_spread();
            
            // Take the POI name if any
            if (h.poi_id_ != (unsigned long)-1)
            {
                // Dereference it
                if (h.poi_id_  < compact_pois_.size())
                {
                    unsigned long long big_id = get_combined_place_id(street_id, house_id);
                    
                    std::string &poi_name = compact_pois_[h.poi_id_];
                    
                    // Divide it into words, lowercase each word and save
                    // a link between a word and this house
                    // Note: each house is iterataed EXACTLY once in the loop above,
                    //  so there will be a single entry per ONE word of a house
                    auto v = extractWordsVector(poi_name);
                    for (const auto &w : v)
                        temp_poi_search_word_to_house_id[tolower(w)].push_back(big_id);
                    temp_poi_search_word_to_house_id[tolower(poi_name)].push_back(big_id);
                }
                else
                    std::cerr << "compact_addresses::sort_uploaded_houses: BAD POI ID! h.poi_id_=" << h.poi_id_
                        << ", compact_pois_.size()=" << compact_pois_.size() << std::endl;
            }

            ++house_id;
        } // for (auto &h : s.houses_)
        
        ++street_id;
    } // for (auto &s : compact_streets_)
    
    std::cerr << "compact_addresses::sort_uploaded_houses: biggest_number_of_houses_per_street="
        << biggest_number_of_houses_per_street << ", biggest_street='" << biggest_street << "'" << std::endl;

    // Sort it
    house_geo_locations_.sort();
}

void compact_addresses::print_stat()
{
    std::cerr << "compact_addresses::print_stat: search_objects_streets_.size()=" << compact_search_objects_streets_.size()
        << ", search_objects_streets_.capacity()=" << compact_search_objects_streets_.capacity() << std::endl;
    
    long long total_search_objects_streets_size = 0;
    for (auto i : compact_search_objects_streets_)
        total_search_objects_streets_size += i.first.size() + i.second.size() * 4;
    std::cerr << "compact_addresses::print_stat: search_objects_streets_ APPROX total size=" <<
        (total_search_objects_streets_size +
    compact_search_objects_streets_.capacity() * sizeof(compact_search_objects_streets_[0]))/1048576
    
    
    << " Mb" << std::endl;

    std::cerr << "compact_addresses::print_stat: compact_search_objects_poi_.size()=" << compact_search_objects_poi_.size()
        << ", compact_search_objects_poi_.capacity()=" << compact_search_objects_poi_.capacity() << std::endl;
    long long total_search_objects_poi_size = 0;
    for (auto i : compact_search_objects_poi_)
        total_search_objects_poi_size += i.first.size() + i.second.size() * 8;
    std::cerr << "compact_addresses::print_stat: search_objects_poi_ APPROX total size=" <<
        (total_search_objects_poi_size +
    compact_search_objects_poi_.capacity() * sizeof(compact_search_objects_poi_[0]))/1048576
    << " Mb"<< std::endl;
     
    std::cerr << "compact_addresses::print_stat: compact_streets_.size()=" << compact_streets_.size()
        << ", compact_streets_.capacity()=" << compact_streets_.capacity() << std::endl;
    long long total_streets_size = 0;
    long long total_houses = 0, total_houses_capacity = 0;;
    for (auto &i : compact_streets_)
    {
        total_houses += i.houses_.size();
        total_houses_capacity += i.houses_.capacity();
        
        total_streets_size += sizeof(compact_street) +
            i.name_.capacity() + i.houses_.capacity() * sizeof(compact_house);
    }
    std::cerr << "compact_addresses::print_stat: streets_ APPROX total size=" <<
    (total_streets_size)/1048576
    <<
        " Mb, total_houses=" << total_houses << " , total_houses_capacity=" << total_houses_capacity << std::endl;
    
    int poi_size = 0, poi_capacity = 0;
    for (auto &p : compact_pois_)
    {
        poi_size += p.size();
        poi_capacity += p.capacity();
    }
    std::cerr << "compact_addresses::print_stat: compact_pois_.size()=" << compact_pois_.size()
        << ", compact_pois_.capacity()=" << compact_pois_.capacity()
    << ", compact_pois_ APPROX total size=" <<
        (poi_capacity + sizeof(std::string) * compact_pois_.capacity())/1048576
        << " Mb, compact_pois_ total capacity=" << poi_capacity << std::endl;
    
    
    int chn_size = 0, chn_capacity = 0;;
    for (auto &chn : compact_house_numbers_)
    {
        chn_size += chn.size();
        chn_capacity += chn.capacity();
    }
    
    std::cerr << "compact_addresses::print_stat: compact_house_numbers_.size()=" <<
    compact_house_numbers_.size() << ", compact_house_numbers_.capacity()=" <<
    compact_house_numbers_.capacity() << ", chn_size_size=" << chn_size <<
    ", chn_capacity=" << chn_capacity << ", APPROX total size=" <<
    (chn_capacity + sizeof(std::string) * compact_house_numbers_.capacity())/1048576 << " Mb" << std::endl;
    
    // Geo coding database for houses
    house_geo_locations_.print_stat();
}

bool operator<(const std::pair<std::string, std::vector<unsigned long> > &a, const std::string &b)
{
    return a.first < b;
}

bool operator<(const std::pair<std::string, std::vector<unsigned long> > &a,
               const std::pair<std::string, std::vector<unsigned long> > &b)
{
    return a.first < b.first;
}

std::vector<std::pair<int, std::string> > compact_addresses::auto_complete_street(const std::string &str_) const
{
    std::string lstr = tolower(str_);
    std::vector<unsigned long> ids;

    // Binary search
    int n = 0;
    for (auto i = std::lower_bound(compact_search_objects_streets_.begin(),
                                   compact_search_objects_streets_.end(),
                                   lstr,
                                        [](const std::pair<std::string, std::vector<unsigned long> > &a, const std::string &b)
                                        {
                                            return a.first < b;
                                        }); i != compact_search_objects_streets_.end(); ++i)
    {
        // Check if lstr is a prefix to i->first
        if (i->first.find(lstr) == std::string::npos)
            break;
        
        //std::cerr << "compact_addresses::auto_complete_street: i->first='" << i->first << std::endl;
        
        // If it's a prefix then all the associated ids are OK
        ids.insert(ids.end(), i->second.begin(), i->second.end());
        
        // Limit result with 100 items
        if (ids.size() >= 100)
            break;
    }
    
    // Get a street name for each ID and return
    std::vector<std::pair<int, std::string> > result;
    for (auto i : ids)
    {
        //std::cerr << "compact_addresses::auto_complete_street: street id: " << i << std::endl;
        result.push_back({i, get_street_by_street_id(i)->name_});
    }
    
    return result;
}

std::vector<compact_house> compact_addresses::auto_complete_house(const std::string &str_, std::string &street) const
{
    std::vector<compact_house> result;
    
    street = str_;

    // Cut letter by letter from the end of str_ until we find EXCTLY ONE street in searchables
    int n = str_.size();
    while (!str_.empty())
    {
        std::vector<std::pair<int, std::string> > r = auto_complete_street(street);
        if (r.size() == 1)
        {
            // We got it!
            
            // Now try to find a house by a remainder
            std::string house_pattern(str_.begin() + n, str_.end());
            
            //std::cerr << "compact_addresses::auto_complete_house(" << str_ << "): found the street: street='" <<
              //  street << "', house_pattern='" << house_pattern << "', street ID " << r[0].first << std::endl;
            
            
            const compact_street *cs = get_street_by_street_id(r[0].first);
            if (!cs)
                break;
            
            
            // Ignore the eading space
            if (!house_pattern.empty() && house_pattern[0] == ' ')
                house_pattern = std::string(str_.begin() + n + 1, str_.end());
            
            //std::cerr << "compact_addresses::auto_complete_house: " << cs->houses_.size() << " houses totally" << std::endl;
            
            // Use the binary search just like with street
            for (auto h = std::lower_bound(cs->houses_.begin(), cs->houses_.end(), house_pattern,
                                           
                                           // Put this predicate specifically here as a lambda to always
                                           // be aware of the search complexity O(Log(houses per street)*Log(houses total))
                                           [this](const compact_house &a, const std::string &s){
                                               // O(Log(total number of houses))
                return *get_house_number_by_house_number_id(a.number_id_) < s;
                                           });
                 
                 h != cs->houses_.end(); ++h)
            {
                //std::cerr << "compact_addresses::auto_complete_house: house number '" << h->number_ << "'" << std::endl;
                
                // Lookup a string representation of a house number in the global sorted vector
                const std::string &house_number = *get_house_number_by_house_number_id(h->number_id_);
                
                // Check if house_pattern is a prefix of h->number_
                if (house_pattern.size() <= house_number.size() &&
                    std::equal(house_pattern.begin(), house_pattern.end(), house_number.begin()))
                    result.push_back(*h);
                else
                    break;
            }
            
            break;
        }
        street.resize(street.size()-1);
        --n;
    }
    
    return result;
}

std::vector<std::pair<compact_house, std::string> > compact_addresses::get_houses_near_location(const point &location, int resolution, int ring_size, compact_house *nearest_h, std::string *nearest_s) const
{
    // Find nearest house ids
    std::unordered_set<unsigned long long> house_ids;
    if (!house_geo_locations_.get_objects_in_radius_better_version(location, resolution, ring_size, house_ids))
    {
        std::cerr << "compact_addresses::get_houses_near_location debug: get_objects_in_radius failed" << std::endl;
        return {};
    }
    
    //std::cerr << "addresses::get_houses_near_location debug: get_objects_in_radius returned " << house_ids.size()
    //    << " house ids" << std::endl;

    // Convert ids to houses
    std::vector<std::pair<compact_house, std::string> > result;
    double best_distance;
    int best_house = -1;
    for (auto &i : house_ids)
    {
        const compact_house *ch = get_house_by_big_id(i);
        const compact_street *cs = get_street_by_big_id(i);
        
        if (!cs || !ch)
        {
            std::cerr << "compact_addresses::get_houses_near_location: can't find house or street by id: " << i << std::endl;
            continue;
        }
        double distance = shortest_distance_km(location, {.longitude = ch->lon_, .latitude = ch->lat_});
        if (best_house == -1 || distance < best_distance)
        {
            best_house = i;
            *nearest_h = *ch;
            *nearest_s = cs->name_;
            best_distance = distance;
        }
        
        result.push_back({*ch, cs->name_});
    }
    
    return result;
}

std::vector<std::pair<compact_house, std::string> > compact_addresses::auto_complete_poi(const std::string &str_) const
{
    std::string lstr = tolower(str_);
    std::vector<unsigned long long> house_ids;

    // Binary search
    int n = 0;
    for (auto i = std::lower_bound(compact_search_objects_poi_.begin(),
                                   compact_search_objects_poi_.end(),
                                   lstr,
                                        [](const std::pair<std::string, std::vector<unsigned long long> > &a, const std::string &b)
                                        {
                                            return a.first < b;
                                        }); i != compact_search_objects_poi_.end(); ++i)
    {
        // Check if lstr is a prefix to i->first
        // Note: find it not the prefix, but it gives more results :-)
        if (i->first.find(lstr) == std::string::npos)
            break;
        
        //std::cerr << "compact_addresses::auto_complete_street: i->first='" << i->first << std::endl;
        
        // If it's a prefix then all the associated ids are OK
        for (auto &j : i->second)
            house_ids.push_back({j /* ID of one of houses matched with this POI */});
        
        // Limit result with 100 items
        if (house_ids.size() >= 100)
            break;
    }
    
    // Get a house and a street for each ID and return
    std::vector<std::pair<compact_house, std::string> > result;
    for (auto &i : house_ids)
    {
        unsigned long long id = i;
        
        const compact_street *cs = get_street_by_big_id(id);
        if (!cs)
        {
            std::cerr << "compact_addresses::auto_complete_poi: BAD ID! CAN'T GET STREET: " << id << std::endl;
            continue;
        }
        const compact_house *ch = get_house_by_big_id(id);
        if (!ch)
        {
            std::cerr << "compact_addresses::auto_complete_poi: BAD ID! CAN'T GET HOUSE: " << id << std::endl;
            continue;
        }
        //std::cerr << "compact_addresses::auto_complete_street: street id: " << i << std::endl;
        result.push_back({*ch, cs->name_});
    }
    
    return result;
}

void compact_addresses::dump_compacted_things(std::ostream &os)
{
    os << "SEARCH_OBJECTS_STREETS" << std::endl;
    os << compact_search_objects_streets_.size() << std::endl;
    for (auto &i : compact_search_objects_streets_)
    {
        os << i.first << std::endl;
        os << i.second.size() << std::endl;
        for (auto &j : i.second)
            os << j << std::endl;
    }
    
    os << "SEARCH_OBJECTS_POIS" << std::endl;
    os << compact_search_objects_poi_.size() << std::endl;
    for (auto &i : compact_search_objects_poi_)
    {
        os << i.first << std::endl;
        os << i.second.size() << std::endl;
        for (auto &j : i.second)
            os << j << std::endl;
    }
    
    os << "STREETS" << std::endl;
    os << compact_streets_.size() << std::endl;
    os << std::setprecision(5);
    for (auto &i : compact_streets_)
    {
        os << i.name_ << std::endl;
        os << i.houses_.size() << std::endl;
        for (auto &j : i.houses_)
            os << j.number_id_ << std::endl << j.poi_id_ << std::endl << j.lat_ << std::endl << j.lon_ << std::endl;
    }
    
    os << "POIS" << std::endl;
    os << compact_pois_.size() << std::endl;
    for (auto &i : compact_pois_)
        os << i << std::endl;

    os << "HOUSE_NUMBERS" << std::endl;
    os << compact_house_numbers_.size() << std::endl;
    for (auto &i : compact_house_numbers_)
        os << i << std::endl;
    
    house_geo_locations_.dump_compacted_things(os);
}

unsigned long long get_number_helper(std::istream &is, std::string &buffer)
{
    std::getline(is, buffer);
    return std::stoll(buffer);
}

float get_fnumber_helper(std::istream &is, std::string &buffer)
{
    std::getline(is, buffer);
    return std::stof(buffer);
}


void compact_addresses::undump_compacted_things(std::istream &is)
{
    // SEARCH_OBJECTS_STREETS
    std::string s;
    std::getline(is, s);
    compact_search_objects_streets_.resize(get_number_helper(is, s));
    compact_search_objects_streets_.shrink_to_fit();
    for (auto &i : compact_search_objects_streets_)
    {
        std::getline(is, i.first);
        i.second.resize(get_number_helper(is,s));
        i.second.shrink_to_fit();
        for (auto &j : i.second)
            j = get_number_helper(is, s);
    }
    
    // SEARCH_OBJECTS_POIS
    std::getline(is, s);
    compact_search_objects_poi_.resize(get_number_helper(is, s));
    compact_search_objects_poi_.shrink_to_fit();
    for (auto &i : compact_search_objects_poi_)
    {
        std::getline(is, i.first);
        i.second.resize(get_number_helper(is,s));
        i.second.shrink_to_fit();
        for (auto &j : i.second)
            j = get_number_helper(is, s);
     }
    
    // STREETS
    std::getline(is, s);
    compact_streets_.resize(get_number_helper(is, s));
    compact_search_objects_poi_.shrink_to_fit();
    for (auto &i : compact_streets_)
    {
        std::getline(is, i.name_);
        i.houses_.resize(get_number_helper(is, s));
        i.houses_.shrink_to_fit();
        for (auto &j : i.houses_)
        {
            j.number_id_ = get_number_helper(is, s);
            j.poi_id_ = get_number_helper(is, s);
            j.lat_ = get_fnumber_helper(is, s);
            j.lon_ = get_fnumber_helper(is, s);
        }
    }
}

} // namespace geo

/*
std::getline(is, s);
size_t n = s.find(",");
if (n == std::string::npos)
{
    std::cerr << "global_position_db::undump_compacted_things, file is boken: " << s << std::endl;
    break;
}
i.first = std::stoll(std::string(s.begin(), s.begin() + n));
i.second = std::stoll(std::string(s.begin() + n + 1, s.end()));
*/
