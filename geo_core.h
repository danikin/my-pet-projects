/*
*    geo.h
*
*    (C) Denis Anikin 2020
*
*    Headers for geo services
*
*/

#ifndef _geo_core_h_included_
#define _geo_core_h_included_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>

#include "geo_core.h"
#include "geolocation.h"

namespace geo
{

// House
struct house
{
    // Address part of the house
    std::string number_;
    
    // Full address
    std::string full_address_;
    
    // House position
    point house_position_;
    
    // Nearest road position
    point nearest_road_position_;
};

// Street
struct street
{
    // Street name
    std::string name_;
    
    // Houses
    std::vector<long long> house_ids_;
};

// All addresses with geocoding and search
class addresses
{
public:
    
    addresses();
    
    // Uploads a street along with its components to search
    // Each word in each component, each component and the street itself
    // will be searched by subsequential calls of auto_complete
    // street_id is needed to lately connect it with uploaded houses
    void upload_street(const std::vector<std::string> &street_components,
                       const std::string &street_name, long long street_id);
    
    // Uploads a poi
    // Each word in each component, each component and the poi itself
    // will be searched by subsequential calls of auto_complete
    // Note: poi autocompletes directyly to a house
    void upload_poi(const std::vector<std::string> &poi_components,
                            const std::string &poi_name,
                            const std::string &full_address);
    
    // Uploads a house
    // Returns false if it's already there
    //bool upload_house(long long street_id, long long house_id, house &&h);
    
    // Auto-completes address by first chars
    // Returns pais of steets ids and names
    std::vector<std::pair<int, std::string> > auto_complete_street(const std::string &str_);
    
    // Auto-completes house name by a beginning part of its full address
    // Returns the full house :-)
    std::vector<house> auto_complete_house(const std::string &str_);

    // Auto-completes house name by a beginning part of a poi
    // Returns a collection of pairs {house, poi_name}
    std::vector<std::pair<house, std::string> > auto_complete_house_by_poi(const std::string &str_);

    // Returns a nearest house to the specified location
    house get_nearest_house(const point &location);

    // Return houses near the specified location
    //std::vector<house> get_houses_near_location(const point &location, int resolution, int ring_size);
    
    // Returns an index into vector of houses for a house which is closer
    // to the specified location
    int get_nearest_house(const point &location, const std::vector<house> &houses);

    void print_stat();

private:

    void auto_complete_street_impl(const std::string &lstr, std::vector<std::pair<int, std::string> > &result);
    
    // Objects to search streets
    // Search word -> pair (street id, street name)
    // Note: multimap because one word can resolve in many streets
    std::multimap<std::string, std::pair<int, std::string> > search_objects_streets_;
    
    // Objects to search directly addresses of houses (for POIs)
    // Search word -> pair (house_id, POI name)
    // Note: multimap because one word can resolve in many pois
    std::multimap<std::string, std::pair<long long, std::string>> search_objects_poi_;
    
    // All streets
    // street it -> street object
    std::unordered_map<long long, street> streets_;
    
    // All houses
    // house id -> house object
    std::unordered_map<long long, house> houses_;
    
    // House full address -> house object (index)
    // Note: map, not multi because one full address resolves to one house
    // TODO: change to trie in a future!!!
    std::map<std::string, long long> full_address_to_house_id_;
    
    // Geo coding database for houses
    global_position_db house_geo_locations_;
};

// House
struct compact_house
{
    //bool operator<(const compact_house &rhs) const { return number_ < rhs.number_; }
    //bool operator<(const std::string &rhs) const { return number_ < rhs; }
    
    // ID of address part of the house
    // To get the string with a house number lookup here: compact_house_numbers_
    unsigned long number_id_;
    
    // If house has a poi then it's ID here otherwise is -1
    // To get the poi name lookup here: compact_pois_
    unsigned long poi_id_;
    
    // House position
    float lat_, lon_;
};

// Street
struct compact_street
{
    // Street EXACT name
    std::string name_;
    
    // Houses sorted by names
    // Note: this vector is SORTED
    // Note!!! We store ONE street with a name. So this one street will have MANY copies
    // of houses with same names but different coordinate
    std::vector<compact_house> houses_;
};

// For lower_bound
inline bool operator<(const compact_street &lhs,
               const compact_street &rhs) { return lhs.name_ < rhs.name_; }

inline bool operator<(const compact_street &lhs,
                const std::string &rhs) { return lhs.name_ < rhs; }

class compact_addresses
{
public:
    
    compact_addresses();
    
    // How to uploads data:
    //  1.  Upload all streets via upload_streets and all houses via upload_houses_once
    //      It MUST be called once for all streets and all houses!!!
    //  2.  Upload each house via upload_house
    //  3.  Sort uploaded houses via sort_uploaded_houses
    //      Important! If don't sort then the search will not work properly!
    //  4.  Fill a temporary map: lat,lon -> street_id,house_id
    //      Note: this map can be filled more than once - it will just be updated
    //  5.  Once temp_map is ready for all pois they are uploaded via upload_poi_once
    //      Note: this MUST be done once!!!
    //  6.  Sort uploaded pois
    
    void upload_streets_once(const std::set<std::string> &street_names);
    void upload_house_numbers_once(const std::set<std::string> &house_numbers);
    void upload_pois_once(const std::set<std::string> &pois);
    
    // Uploads a house and returns the id of its poi in poi_id or -1 if
    // the house does not have POI
    void upload_house(const std::string &poi,
                        const std::string &street,
                        const std::string &house,
                        float lat,
                        float lon);
    
    void sort_uploaded_houses(std::map<std::string, std::vector<unsigned long long> > &temp_poi_search_word_to_house_id);
    void sort_uploaded_pois(const std::map<std::string, std::vector<unsigned long long> > &temp_poi_search_word_to_house_id);

    void print_stat();
    
    // Dump everything to a file for later fast uploading
    void dump_compacted_things(std::ostream &os);
    
    // Uploads everything from a file - a faster mean of start
    void undump_compacted_things(std::istream &is);
    
    // Autocompletes the street and returns a vector of street ids and names
    std::vector<std::pair<int, std::string> > auto_complete_street(const std::string &str_) const;
    
    // Finds best matches for a street name given a search string and location
    // It works like this:
    //  1. Lower cases the search_string and divides it into words
    //  2. For every word find a list of all streets with a search object beginning with those words
    //  3. Intersect those lists
    //  4. Among intersected streets finds the one that's has the first house nearest
    //      to "location"
    // The amount of best matches that return is the size of best best
    // The array will decrease its size if needed
    // "best" is sorted
    void best_street_name_match(const std::string &search_string, const point &location,
                                       std::vector<std::pair<unsigned long, float> > &best) const;

    // The same as above but for POI
    // Returns POI's house ID
    void best_poi_match(const std::string &search_string, const point &location,
                                      std::vector<std::pair<unsigned long long, float> > &best) const;

    // Finds for a best macth for a house
    // Steps:
    //  1.  Consider the search_string as an EXACT street name + something and try to find
    //      a house in the street. Saves all the matches and distnances to that
    //  2.  Take every word containing ONLY numbers and sequentially search for
    //      streets using a search_string EXCEPT that word
    //  3.  Each such a street is narrowed by only houses that match that word
    //      sorts houses by distance
    //  4.  Mixes 1 and 4, sorts that and returns it as a result
    void best_house_match(const std::string &search_string,
                                                   const point &location,
                                                   std::vector<std::pair<unsigned long long, float> > &best) const;

    // Returns best match of houses/pois near the specified location
    // Note: this is basically reverse geocoding
    void best_match(const point &location,
                                       std::vector<std::pair<unsigned long long, float> > &best) const;
    
    // Autocompletes the full address and returns separetaly its street part and
    // suggestions for houses
    std::vector<compact_house> auto_complete_house(const std::string &str_, std::string &street) const;

    // Autocompletes the full address by a poi name
    // Returns a vector of pairs - house + street_name
    std::vector<std::pair<compact_house, std::string> > auto_complete_poi(const std::string &str_) const;

    std::vector<std::pair<compact_house, std::string> > get_houses_near_location(const point &location, int resolution, int ring_size, compact_house *nearest_h, std::string *nearest_s) const;
    
    // Returns a poi name by its id
    const std::string *get_poi_by_id(unsigned long poi_id) const
    {
        // No POI - that's OK
        if (poi_id == (unsigned long)-1)
            return NULL;
        else
        if (poi_id >= compact_pois_.size())
        {
            std::cerr << "compact_addresses::get_poi_by_id: WRONG ID: " << poi_id << std::endl;
            return NULL;
        }
        else
            return &compact_pois_[poi_id];
    }
    
    // Returns a string with house number by its id and vice versa
    const std::string *get_house_number_by_house_number_id(unsigned long house_number_id) const
    {
        if (house_number_id >= compact_house_numbers_.size())
        {
            std::cerr << "compact_addresses::get_house_number_by_id: WRONG ID: " << house_number_id << std::endl;
            return NULL;
        }
        else
            return &compact_house_numbers_[house_number_id];
    }
    unsigned long get_house_id_by_number(const std::string &number);

    // Only decl, no body - to generate an error is being used
    const std::string *get_street_name_by_big_id(unsigned long) const;
    std::string *get_street_name_by_big_id(unsigned long);

    const std::string *get_street_name_by_big_id(unsigned long long id) const
    {
        const compact_street *cs = get_street_by_street_id(id >> 32);
        if (!cs)
            return NULL;
        else
            return &cs->name_;
    }
    
    unsigned long get_street_id_by_big_id(unsigned long long id) const
    {
        return id >> 32;
    }

    unsigned long get_house_id_by_big_id(unsigned long long id) const
    {
        return id & 0x00000000ffffffffull;
    }
    
    template <class COMPACT_HOUSE_POINTER, class CLASS_POINTER>
    static COMPACT_HOUSE_POINTER get_house_by_big_id_impl_(CLASS_POINTER this_, unsigned long long id)
    {
        unsigned long street_id = id >> 32;
        unsigned long house_id = id & 0x00000000ffffffffull;
        
        auto cs = this_->get_street_by_big_id(id);
        if (!cs)
            return NULL;

        if (house_id >= cs->houses_.size())
        {
            std::cerr << "compact_addresses::get_house_by_id_: INVALID ID! " << id << ", house_id="
            << house_id << ", street->houses_.size()="<< cs->houses_.size() << std::endl;
            return NULL;
        }
        
        return &cs->houses_[house_id];
    }

    // Only decl, no body - to generate an error is being used
    const compact_house *get_house_by_big_id(unsigned long id) const;
    compact_house *get_house_by_big_id(unsigned long id);

    const compact_house *get_house_by_big_id(unsigned long long id) const
    {
        return get_house_by_big_id_impl_<const compact_house*>(this, id);
    }
    compact_house *get_house_by_big_id(unsigned long long id)
    {
        return get_house_by_big_id_impl_<compact_house*>(this, id);
    }

    const compact_street *get_street_by_big_id(unsigned long long id) const
    {
        return get_street_by_street_id(id >> 32);
    }

    const compact_street *get_street_by_street_id(unsigned long street_id) const
    {
        if (street_id >= compact_streets_.size())
        {
            std::cerr << "compact_addresses::get_street_by_street_id: INVALID ID! street_id="
            << street_id << ", compact_streets_.size()="<< compact_streets_.size() << std::endl;
            return NULL;
        }
        else
            return &compact_streets_[street_id];
    }
    
private:
    
    compact_street *get_street_by_big_id(unsigned long long id)
    {
        return get_street_by_street_id(id >> 32);
    }
    
    compact_street *get_street_by_street_id(unsigned long street_id)
    {
        if (street_id >= compact_streets_.size())
        {
            std::cerr << "compact_addresses::get_street_by_street_id: INVALID ID! street_id="
            << street_id << ", compact_streets_.size()="<< compact_streets_.size() << std::endl;
            return NULL;
        }
        else
            return &compact_streets_[street_id];
    }

    void find_matching_houses_helper_(const compact_street &cs,
                                                         const std::string &house_pattern,
                                                         const point &location,
                                                         std::vector<std::pair<unsigned long long, float> > &result) const;
    
    // Sorted lowcase words to search streets
    // word -> street_ids
    std::vector<std::pair<std::string, std::vector<unsigned long> > > compact_search_objects_streets_;
    
    // Sorted lowcase words to search directly addresses of houses (for POIs)
    // word -> {street_id, house_id}
    std::vector<std::pair<std::string, std::vector<unsigned long long> > > compact_search_objects_poi_;

    // All street sorted by names
    // Note: a street ID is a sequence number to this vector
    // Note: a house ID is pair of a street ID + a sequence number to a vector
    // inside compact_street
    // Note: this vector is SORTED
    std::vector<compact_street> compact_streets_;
    
    // All pois names are sorted by names and stored with EXACT names
    // Note: it's for saving on RAM
    std::vector<std::string> compact_pois_;

    // All house numbers are sorted by names and stored with EXACT names
    // Note: it's for saving on RAM
    std::vector<std::string> compact_house_numbers_;
    
    // Geo coding database for houses (location -> {street_id, house_id})
    global_position_db house_geo_locations_;
};

inline unsigned long long get_combined_place_id(unsigned long street_id,
                                                unsigned long house_id)
{
    return (((unsigned long long)street_id) << 32) + house_id;
}

// Helper functions
std::vector<std::string> extractWordsVector(std::string const& s);
std::string tolower(const std::string &s);
void tolower2(const std::string &s, std::string &result);

} // namespace geo

#endif

