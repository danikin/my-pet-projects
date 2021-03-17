/*
*       balanced_diet_search.cpp
*
*       (C) Denis Anikin 2020
*
*       Impl for the balance food search algorithms
*
*/

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <map>
#include <unordered_set>
#include <set>
#include <array>
#include <string.h>

#include "balanced_diet.h"

namespace balanced_diet
{

bool starts_with(const std::string &a, const std::string &b)
{
    return a.size() >= b.size() && std::equal(b.begin(), b.end(), a.begin());
}

void foods::suggest_words_json(const std::string &search_string,
                  std::ostream &output)
{
    // Divide search string into words and lower case those words
    std::vector<std::string> words;
    std::string lc_str_temp;
    words.clear();
    divide_into_words(search_string, words);
    /*for (auto &w : words)
    {
        lc_str_temp.clear();
        lower_str(w, lc_str_temp);
        
        // Search variants for each word
    }*/
    if (words.empty())
    {
        output << "{\"event\":\"suggest_words\"}";
        return;
    }
    
    output << "{\"event\":\"suggest_words\", \"data\":[" << std::endl;
    
    lc_str_temp.clear();
    const std::string &word = words[words.size()-1];
    lower_str(word, lc_str_temp);
    
    std::cerr << "suggest_words_json: lc_str_temp=" << lc_str_temp << std::endl;
    
    bool b = true;
    
    // Search nutrients
    for (auto it = std::lower_bound(all_nutrient_words_.begin(), all_nutrient_words_.end(), lc_str_temp, []
                                    (const std::pair<std::string, int> &a, const std::string &b) {
                              return a.first < b;
                          });
         it != all_nutrient_words_.end() && starts_with(it->first, lc_str_temp);
         ++it)
    {
        if(!b)output<<",";b=false;
        output << "\"" << get_nutrient_middle_name(it->second) << "\"" << std::endl;
    }
    
    // Search foods
    for (auto it = std::lower_bound(all_food_words_.begin(), all_food_words_.end(), lc_str_temp);
         it != all_food_words_.end() && starts_with(*it, lc_str_temp);
         ++it)
    {
        if(!b)output<<",";b=false;
        output << "\"" << *it << "\"" << std::endl;
    }
    
    
    output << "]}" << std::endl;
}

} // namespace balanced_diet

