/*
*       balanced_diet_upload.cpp
*
*       (C) Denis Anikin 2020
*
*       Upload the data for the balanced diet
*
*/

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <map>
#include <set>
#include <unordered_map>
#include <fstream>

#include "balanced_diet.h"

namespace balanced_diet
{

void divide_into_words(const std::string &str, std::vector<std::string> &words)
{
    auto i = str.begin();
    while (i != str.end())
    {
        auto j = std::find(i, str.end(), ' ');
        if (j == str.end())
        {
            words.push_back(std::string(i, str.end()));
            break;
        }
        else
            words.push_back(std::string(i, j));

        while (*j == ' ')
            ++j;
        i = j;
    }
}

void divide_into_words_lc(const std::string &str, std::vector<std::string> &words)
{
    divide_into_words(str, words);
    std::string lc;
    for (auto &w : words)
    {
        lower_str(w, lc);
        w = lc;
    }
}

void foods::save_recipe(const std::string &name,
                            std::vector<std::pair<int, float> > &foods_to_balance)
{
    food new_food;
    new_food.food_name_ = name;
    new_food.food_category_ = "user recipe";
    
    // Fill the nutrient map of a newly created food
    std::map<int, float> new_nutrients;
    float total_food_amount = 0;
    for (auto &x : foods_to_balance)
    {
        int food_id = x.first;
        food *f = get_food(food_id);
        if (!f || f->food_name_.empty())
            continue;
        total_food_amount += x.second;
        
        for (auto &y : f->nutrients_)
            new_nutrients[y.first] += y.second * x.second;
    }
    
    if (!total_food_amount)
        return;

    // Normalize the nutrient map and convert it to the vector
    new_food.nutrients_.reserve(new_nutrients.size());
    for (auto &m : new_nutrients)
    {
        m.second /= total_food_amount;
        new_food.nutrients_.push_back({m.first, m.second});
    }
    
    // Save the food in RAM
    all_foods_.push_back(new_food);
    
    // Save the food to file
    // TODO!
}

void foods::catalogue_to_json(std::ostream &output)
{
    output << "{\"event\":\"catalogue\",\"data\":[" << std::endl;
    bool is_first = true;
    int catalogue_item_id = 0;
    for (auto &ci : food_catalogue_)
    {
        if (!is_first) output << "," << std::endl;
        is_first = false;
        output
        << "{\"catalogue_item_id\":" << catalogue_item_id
        << ",\"catalogue_item_name\":\"" << ci.catalogue_item_name_ << "\""
        << ",\"example_food_id\":" << ci.example_food_id_
        << "}";
        ++catalogue_item_id;
    }
    output << std::endl << "]}" << std::endl;
}

void foods::upload_food(int food_id, const std::string &food_name, const std::string &food_category)
{
    if (all_foods_.size() < food_id + 1)
        all_foods_.resize(food_id + 1);
    all_foods_[food_id].food_name_ = food_name;
    all_foods_[food_id].food_category_ = food_category;
    all_foods_[food_id].food_catalogue_item_id_ = -1;
}

void foods::upload_nutrient(int nutrient_id,
                            const std::string &nutrient_name,
                            const std::string &nutrient_unit_name)
{
    if (all_nutrients_.size() < nutrient_id + 1)
        all_nutrients_.resize(nutrient_id + 1);
    all_nutrients_[nutrient_id].nutrient_name_ = nutrient_name;
    all_nutrients_[nutrient_id].nutrient_unit_name_ = nutrient_unit_name;
}

void foods::upload_food_nutrient(int food_id, int nutrient_id, float amount)
{
    if (food_id < 0 || food_id >= all_foods_.size())
    {
        std::cerr << "foods::upload_food_nutrient: bad food_id! " << food_id << std::endl;
        return;
    }
    
    all_foods_[food_id].nutrients_.push_back({nutrient_id, amount});
}

void foods::postupload_steps()
{
    // Fill links to the catalogue
    int ci_id = 0;
    for (auto &ci : food_catalogue_)
    {
        //std::cerr << "ci_id=" << ci_id << ", all_foods_.size()=" << all_foods_.size() << std::endl;

        // Get ids for this catalogue item
        std::vector<int> food_ids = ci.exact_food_ids_;
        
        std::vector<std::pair<int, float> > good_nutrients = {};
        std::vector<std::pair<int, float> > bad_nutrients = {};
        
        std::vector<std::string> inc, exc;
        divide_into_words(ci.search_string_include_, inc);
        divide_into_words(ci.search_string_exclude_, exc);
        
        food_rank_json(
                good_nutrients,
                bad_nutrients,
                    inc,
                    exc,
                    ci.search_string_include_exact_,
                    ci.search_string_exclude_exact_,
                   -1,
                   -1,
                       -1,
                       ci.search_nutrient_lower_limits_,
                       ci.search_nutrient_upper_limits_,
                   NULL,
                       -1,
                   &food_ids,
                    -1,
                    -1,
                    NULL,
                       0,
                       0,
                       0,
                       false,
                       
                       0,
                       0.0,
                       
                   std::cerr);
        
        //std::cerr << "ci_id=" << ci_id << ", food_ids.size()=" << food_ids.size() << std::endl;

        // Link this catalogue item to the food
        // Also check if any of those foods have a picture - and if it has - fill example_food_id_ for the picture
        //  of this food in the catalogue
        ci.example_food_id_ = -1;
        for (auto food_id : food_ids)
        {
            get_food(food_id)->food_catalogue_item_id_ = ci_id;

            if (ci.example_food_id_ == -1 &&
                    std::ifstream("./food_pics/" + std::to_string(food_id) + ".jpeg").good())
                ci.example_food_id_ = food_id;
        }

        ++ci_id;
    }

    // Fill all words from foods
    std::vector<std::string> words;
    std::string lc_str_temp;
    int food_id = 0;
    for (auto &f : all_foods_)
    {
        if (f.food_name_.empty())
        {
            ++food_id;
            continue;
        }
        words.clear();
        divide_into_words(f.food_name_, words);
        for (auto &w : words)
        {
            lc_str_temp.clear();
            lower_str(w, lc_str_temp);
            all_food_words_.push_back(lc_str_temp);
        }
        ++food_id;
    }
    
    // Fill all words from nutrient names
    for (auto &n : nutrient_augmented_data_)
    {
        words.clear();
        divide_into_words(n.second.nutrient_middle_name_, words);
        for (auto &w : words)
        {
            lc_str_temp.clear();
            lower_str(w, lc_str_temp);
            all_nutrient_words_.push_back({lc_str_temp, n.first});
        }
    }
    
    std::cerr << "all_food_words_.size()=" << all_food_words_.size() << std::endl;
    
    // Sort and unique words
    std::sort(all_food_words_.begin(), all_food_words_.end());
    all_food_words_.erase(std::unique(all_food_words_.begin(), all_food_words_.end()), all_food_words_.end());
    
    std::cerr << "after unique all_food_words_.size()=" << all_food_words_.size() << std::endl;
    
    std::sort(all_nutrient_words_.begin(), all_nutrient_words_.end(), []
              (const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
        return a.first < b.first;
    });
}

void foods::sort_nutrients()
{
    int food_id = 0;
    for (auto &f : all_foods_)
    {
        if (f.food_name_.empty())
        {
            ++food_id;
            continue;
        }
        
        // If there is no Energy, cal (1008) then just add it
        bool cal_found = false;
        for (auto &x : f.nutrients_)
        {
            if (x.first == 1008)
            {
                cal_found = true;
                break;
            }
        }
        if (!cal_found)
            f.nutrients_.push_back({1008, 0.0});
        
        std::sort(f.nutrients_.begin(), f.nutrients_.end(), [](const std::pair<int, float> &a,
                                    const std::pair<int, float> &b){
            return a.first < b.first;
        });

        // If calories absent or NULL then restore it by PFC + fiber
        // Note: cal_amount can't be NULL because it just was pushed back
        float *cal_amount = f.get_nutrient_amount(1008);
        if (!*cal_amount)
        {
            float protein = f.get_nutrient_amount2(1003);
            float fat = f.get_nutrient_amount2(1004);
            float carbs = f.get_nutrient_amount2(1005);
            float fiber = f.get_nutrient_amount2(1079);
            
            *cal_amount = protein * 4 + fat * 9 + carbs * 4;
            
            // If fiber presents and its amount is not erroneous (sometimes it's more than carbs)
            //  then partially exclude fiber from calories - use the formula that there is
            //  1.5 cal per 1gr of fiber
            if (fiber && fiber < carbs)
                *cal_amount -= fiber * 2.5;
            
/*            if (!*cal_amount)
            {
                std::cerr << "STILL ZERO CAL FOR " << food_id << ", " << f.food_name_ << std::endl;
            }*/
        }
        
        f.nutrients_hash_.reserve(f.nutrients_.size() * 2);
        for (auto &x : f.nutrients_)
            f.nutrients_hash_[x.first] = x.second;
        f.fast_access_prot_ = f.get_nutrient_amount2(1003);
        f.fast_access_cal_ = f.get_nutrient_amount2(1008);
        
        // Fill nutrients and percent of daily norms sorted by that
        for (auto &x : f.nutrients_)
        {
            float norm = get_nutrient_recommended_min_amout(x.first);
            f.nutrients_sorted_daily_norm_.push_back({x.first, (norm ? x.second / norm : 0)});
        }
        std::sort(f.nutrients_sorted_daily_norm_.begin(), f.nutrients_sorted_daily_norm_.end(), [](const std::pair<int, float> &a,
                                    const std::pair<int, float> &b){
            return a.second > b.second;
        });
        
        //f.fast_access_cal_lt_2000_grams_ = 100.0 * 2000.0 / f.fast_access_cal_;
        //f.fast_access_prot_gt_100_grams_ = 100.0 * 100 / f.fast_access_prot_;
        //f.fast_access_prot_gt_100_cal_lt_2000_rank_ = f.fast_access_prot_gt_100_grams_ /
          //  f.fast_access_cal_lt_2000_grams_;

        ++food_id;
    } // for (auto &f : all_foods_)
}

} // namespace balanced_diet
