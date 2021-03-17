/*
*    osm_parser.cpp
*
*    (C) Denis Anikin 2020
*
*    Parser of osm files
*
*/

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unordered_map>

std::string find_between(const std::string &input_string,
                            const char *pattern1, int size1,
                            const char *pattern2)
{
    auto i = input_string.find(pattern1);
    if (i == std::string::npos)
        return std::string();
    
    auto j = input_string.find(pattern2, i + size1);
    if (j == std::string::npos)
        return std::string();
    
    return std::string(input_string, i + size1, j - i - size1);
}



int main()
{
    const char divider[] = "@@@^$$$";
    
    std::string input_string, lat, lon, name, street, housenumber, temp, temp2, id;
    int line_n = 0;

    //  1   -   outside node
    //  2   -   inside node
    int status = 1;
    
    // nd ref= to lan and lon
    std::unordered_map<long long, std::pair<double, double> > nodes;
    // way id to lan and lon
    std::unordered_map<long long, std::pair<double, double> > ways;
    
    while (true)
    {
        if (!getline(std::cin, input_string))
        {
            break;
            //usleep(10000);
            //continue;
        }
        ++line_n;
        if ((line_n % 100000) == 0)
            std::cerr << "line_n=" << line_n << std::endl;
        
        if (input_string.find(divider) != std::string::npos)
        {
            std::cerr << "DIVIDER FOUND!!! " << input_string << std::endl;
            break;
        }
        
        if (status == 1)
        {
            // If it's not a node then don't even try to find lat/lon - jump to the inside parsing and
            // expect that we will get lat/lon stored in "nodes" via a reference
            if (input_string.find("<way") != std::string::npos ||
                input_string.find("<relation") != std::string::npos)
            {
                // If this is "way" then save its id
                id = find_between(input_string, "id=\"", 4, "\"");
                status = 2;
                continue;
            }
            
            temp2 = find_between(input_string, "id=\"", 4, "\"");
            if (temp2.empty())
                continue;
            temp = find_between(input_string, "lat=\"", 5, "\"");
            if (temp.empty())
                continue;
            lon = find_between(input_string, "lon=\"", 5, "\"");
            if (lon.empty())
                continue;
            lat = temp;
            id = temp2;
            
            // Found a good node with id, lat, non
            
            // Save it in the cache
            nodes[std::stoll(id)] = {std::stod(lat), std::stod(lon)};
            
            // A node with internals - go inside
            if (input_string.find("/>") == std::string::npos)
            {
                //std::cerr << "GO TO STATUS 2, lat=" << lat << ",lon=" << lon << std::endl;
                status = 2;
            }
            else
            {
                lat.clear();
                lon.clear();
                id.clear();
            }
            continue;
        }
        else
        if (status == 2)
        {
            // Found a closing tag - internals have been finished - go next
            if (input_string.find("</node>") != std::string::npos ||
                    input_string.find("</way>") != std::string::npos ||
                    input_string.find("</relation>") != std::string::npos)
            {
                
                // If there is just a POI without house number and/or street number then it's OK, save it AS IS
                // It's still useful for suggestions
                if (!name.empty())
                {
                    if (street.empty())
                        street = "###POI_WITHOUT_STREET###";
                    if (housenumber.empty())
                        housenumber = "###POI_WITHOUT_HOUSENUMBER###";

                    if (id.empty())
                        std::cerr << "NODE WITH EMPTY id, input_string='" << input_string << "'" << std::endl;
                    
                    std::cout << id << divider << name << divider << street << divider << housenumber << divider << lat << divider << lon << std::endl;
                }
                
                name.clear();
                street.clear();
                housenumber.clear();
                lat.clear();
                lon.clear();
                id.clear();
                status = 1;
                continue;
            }
            
            temp = find_between(input_string, "k=\"name\" v=\"", 12, "\"");
            if (!temp.empty()) name = temp;
            temp = find_between(input_string, "k=\"addr:housenumber\" v=\"", 24, "\"");
            if (!temp.empty()) housenumber = temp;
            temp = find_between(input_string, "k=\"addr:street\" v=\"", 19, "\"");
            if (!temp.empty()) street = temp;
            
            // If there was not lat/lon in the fields then find it via a reference inside
            if (lat.empty() || lon.empty())
            {
                // First try to find a reference from the way
                temp = find_between(input_string, "<nd ref=\"", 9, "\"");
                if (!temp.empty())
                {
                    auto i = nodes.find(std::stoll(temp));
                    if (i == nodes.end())
                    {
                        std::cerr << "COULD NOT FIND a ref to NODE from WAY: '" << temp << "' in the cache" << std::endl;
                    }
                    else
                    {
                        // Found a node from the ref from the way
                        lat = std::to_string(i->second.first);
                        lon = std::to_string(i->second.second);
                        
                        // Also save the same coordinate with id of its node to use it in the relation
                        if (!id.empty())
                        {
                            // Found a way id. Now save it in ways hash - we will use it for relations
                            ways[std::stoll(id)] = {i->second.first, i->second.second};
                        }
                        else
                        {
                            std::cerr << "WAY WITH EMPTY id, input_string='" << input_string << "'" << std::endl;
                        }
                    }
                }

                // Then try to find a reference from relation to the way
                temp = find_between(input_string, "type=\"way\" ref=\"", 16, "\"");
                if (!temp.empty())
                {
                    auto i = ways.find(std::stoll(temp));
                    if (i == ways.end())
                    {
                        std::cerr << "COULD NOT FIND a ref to WAY from RELATION: '" << temp << "' in the cache" << std::endl;
                    }
                    else
                    {
                        // Found a way from the ref from the relation
                        lat = std::to_string(i->second.first);
                        lon = std::to_string(i->second.second);
                    }
                }
            }
            
            if (!street.empty() && !housenumber.empty())
            {
                if (lat.empty() || lon.empty())
                {
                    std::cerr << "LAT OR LON EMPTY! lat='" << lat << "', lon='" << lon << "', name="
                    << name << ", street=" << street << ",housenumber=" << housenumber
                    << std::endl;
                }
                
                if (id.empty())
                    std::cerr << "NODE WITH EMPTY id, input_string='" << input_string << "'" << std::endl;
                
                std::cout << id << divider << name << divider << street << divider << housenumber << divider << lat << divider << lon << std::endl;
                name.clear();
                street.clear();
                housenumber.clear();
                lat.clear();
                lon.clear();
                id.clear();
                status = 1;
            }
            
        }
    }
    
	return 0;
}
