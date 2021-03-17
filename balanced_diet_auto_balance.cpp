/*
*       balanced_diet_auto_balance.cpp
*
*       (C) Denis Anikin 2020
*
*       Impl for the auto balance algorithms
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
#include "balanced_diet_recipe.h"

namespace balanced_diet
{

float foods::get_food_rank_with_current_values2(food *f,
                          std::vector<std::pair<int, float> > &good_nutrients,
                          std::vector<std::pair<int, float> > &bad_nutrients,
                            std::unordered_map<int, float> &nutrient_current_values,
                           float *good_grams,
                           float *bad_grams,
                        float *amount_to_reach_quota,
                                               float hard_limit_K,
                                               float weight_limit_K,
                                               float total_meal_weight,
                                               float total_meal_weight_limit)
{
    float max_amount_before_limits = 1000000000;
    float max_amount_to_reach_quota = 0;
    
    /*
     *  We will save in "max_amount_to_before_limits" the maximum amount of this food that makes sense
     *  to eat to reach quota for all good nutrients that this food has
     *
     *  We will save in "max_amount_to_reach_quota" the maximum amount of this food that we can eat
     *  before we reach the upper limit on at least one good or bad nutrient
     *  The idea behind that is this: we want the food with the best rank as much as possible but no more
     *  than limits on nutrients
     */

    int n_good_nutrients_to_fulfill = 0;
    
    for (auto &n_i : good_nutrients)
    {
        int nutrient_id = n_i.first;
        auto *nutrient_aug_data = get_nutrient_augmented_data(nutrient_id);
        
        float nutrient_quota = n_i.second;
        
        // Determine the upper limit of a nutrient
        float nutrient_upper_limit;
        if (!nutrient_aug_data)
            nutrient_upper_limit = nutrient_quota * hard_limit_K;
        else
        {
            nutrient_upper_limit = nutrient_aug_data->max_recommended_amount_;
            if (nutrient_upper_limit == -1)
                nutrient_upper_limit = 1000000000;
            else
                nutrient_upper_limit *= hard_limit_K;
        }
        
        float nutrient_current_value = nutrient_current_values[nutrient_id];

        // Increase the number of good nutrient that still have not reached the quota
        // Note: it does not matter if this food contains those nutrients or not - we use this number as
        //  a denominator to determine the percent of maximum quota completion by this food
        if (nutrient_current_value < nutrient_quota)
            ++n_good_nutrients_to_fulfill;

        float nutrient_amount_per_100g = f->get_nutrient_amount_fast(nutrient_id);
        if (!nutrient_amount_per_100g)
            continue;
        
        /*if (f - &all_foods_[0] == 335604)
        {
            std::cerr << "nutrient_id=" << nutrient_id << ", nutrient_amount_per_100g=" <<
            nutrient_amount_per_100g << ", nutrient_current_value=" << nutrient_current_value <<
            ", max_amount_before_limits=" << max_amount_before_limits << ", max_amount_to_reach_quota=" << max_amount_to_reach_quota << ", nutrient_quota=" << nutrient_quota << std::endl;
        }*/
        
        // If this food contains a nutrient for which we've already reached the maximum amount then
        //  don't consider this food at all
        if (nutrient_current_value >= nutrient_upper_limit)
        {
            *amount_to_reach_quota = 0;
            *good_grams = 0;
            return 1000000000;
            //continue;
        }
        
        // "m" is how much this food in grams we can eat to reach the quota with this nutrient
        float m = 100.0 * (nutrient_upper_limit - nutrient_current_value) / nutrient_amount_per_100g;
        if (m < max_amount_before_limits)
            max_amount_before_limits = m;
        m = 100.0 * (nutrient_quota - nutrient_current_value) / nutrient_amount_per_100g;
        if (m > max_amount_to_reach_quota)
            max_amount_to_reach_quota = m;
    
        /*if (f - &all_foods_[0] == 335604)
        {
            std::cerr << "m=" << m << std::endl;
        }*/
        
        // Can't take this food at all because we reach limits on this good nutrient
        if (max_amount_before_limits <= 0)
        {
            *amount_to_reach_quota = 0;
            *good_grams = 0;
            return 1000000000;
        }
        
        /*if (f - &all_foods_[0] == 171910)
        {
            std::cerr << "m=" << m << ", min_amount_to_reach_quota=" << min_amount_to_reach_quota << std::endl;
        }*/
    }

    // No reason to take this food at all because it does not increase good nutrient consumption
    if (max_amount_to_reach_quota <= 0)
    {
        *amount_to_reach_quota = 0;
        *good_grams = 0;
        return 1000000000;
    }
    
    /*if (f - &all_foods_[0] == 167512)
    {
        std::cerr << "*amount_to_reach_quota=" << *amount_to_reach_quota << std::endl;
    }*/
    
    // Determine how many bad grams in 100 grams
    // By default we use the weight of the food as a bad nutrient with the limit of 5kg
    *bad_grams = weight_limit_K;//2;
    for (auto &n_i : bad_nutrients)
    {
        int nutrient_id = n_i.first;
        float nutrient_quota = n_i.second;
        float nutrient_amount_per_100g = f->get_nutrient_amount_fast(nutrient_id);
        float nutrient_current_value = nutrient_current_values[nutrient_id];
        
        /*if (f - &all_foods_[0] == 171406)
        {
            std::cerr << "nutrient_id=" << nutrient_id << ", nutrient_amount_per_100g=" <<
            nutrient_amount_per_100g << ", nutrient_current_value=" << nutrient_current_value << ", nutrient_quota=" << nutrient_quota << std::endl;
        }*/
        
        /*if (nutrient_amount_per_100g && nutrient_current_value +
            nutrient_amount_per_100g * *amount_to_reach_quota / 100.0 >= nutrient_quota)
        {
            *amount_to_reach_quota = 0;
            return 1000000000;
            //continue;
        }*/
        
        if (nutrient_amount_per_100g)
        {
            float m = 100.0 * (nutrient_quota - nutrient_current_value) / nutrient_amount_per_100g;
            if (m < max_amount_before_limits)
                max_amount_before_limits = m;
            
            /*if (f - &all_foods_[0] == 335604)
            {
                std::cerr << "(2) m=" << m << ", max_amount_before_limits=" << max_amount_before_limits << std::endl;
            }*/
        }

        // Can't take this food at all because with any amount of that we reach the limit on this bad nutrient
        if (max_amount_before_limits <= 0)
        {
            *amount_to_reach_quota = 0;
            *good_grams = 0;
            return 1000000000;
        }
    }
    
    // Check the total meal weight limit
    if (total_meal_weight + max_amount_before_limits > total_meal_weight_limit)
        max_amount_before_limits = std::fmax(0, total_meal_weight_limit - total_meal_weight);
        
    // Try to take the maximum amount of this food that makes sense to take to maximize good nutrients, but
    //  no more than we can take it before reach limits either on good or bad nutrients
    *amount_to_reach_quota = std::fmin(max_amount_before_limits, max_amount_to_reach_quota);
    if (*amount_to_reach_quota <= 0)
    {
        *amount_to_reach_quota = 0;
        *good_grams = 0;
        return 1000000000;
    }

    /*if (f - &all_foods_[0] == 335604)
    {
        std::cerr << "amount_to_reach_quota=" << *amount_to_reach_quota << "max_amount_before_limits=" << max_amount_before_limits << ", max_amount_to_reach_quota=" << max_amount_to_reach_quota << std::endl;
    }*/

    // Determine the potential good and bad impacts of this food
    // total_percent wil contain the ratio from 0 to 1 where 1 means that this food be taken
    //  in amount *amount_to_reach_quota can fulfill the quota for all needed good nutrients
    // We need to understand how bad this food is. It will be measured by the maximum percent amount all nutrients
    //  towards to the upper limit
    float total_good_percent = 0;
    float max_bad_percent = 0;
    
    for (auto &n_i : good_nutrients)
    {
        int nutrient_id = n_i.first;
        float nutrient_quota = n_i.second;
        float nutrient_amount_per_100g = f->get_nutrient_amount_fast(nutrient_id);
        if (!nutrient_amount_per_100g)
            continue;
        float nutrient_current_value = nutrient_current_values[nutrient_id];
        
        float nutrient_needed_amount = nutrient_quota - nutrient_current_value;
        if (nutrient_needed_amount <= 0)
            continue;
        
        // Adjust the good percent
        total_good_percent += std::fmin(1.0,
                                   nutrient_amount_per_100g * *amount_to_reach_quota / 100 / nutrient_needed_amount);

        // Determine the upper limit of a nutrient
        auto *nutrient_aug_data = get_nutrient_augmented_data(nutrient_id);
        float nutrient_upper_limit;
        if (!nutrient_aug_data)
            nutrient_upper_limit = nutrient_quota * hard_limit_K;
        else
        {
            nutrient_upper_limit = nutrient_aug_data->max_recommended_amount_;
            if (nutrient_upper_limit == -1)
                nutrient_upper_limit = 1000000000;
            else
                nutrient_upper_limit *= hard_limit_K;
        }
        if (nutrient_current_value >= nutrient_upper_limit)
        {
            *amount_to_reach_quota = 0;
            *good_grams = 0;
            return 1000000000;
        }
        
        // Adjust the bad percent
        float bad_percent = std::fmin(1.0,
                                      nutrient_amount_per_100g * *amount_to_reach_quota / 100
                                        / (nutrient_upper_limit - nutrient_current_value));
        if (bad_percent > max_bad_percent)
            max_bad_percent = bad_percent;
    }
    total_good_percent /= n_good_nutrients_to_fulfill;
    
    for (auto &n_i : bad_nutrients)
    {
        int nutrient_id = n_i.first;
        float nutrient_upper_limit = n_i.second;
        float nutrient_amount_per_100g = f->get_nutrient_amount_fast(nutrient_id);
        float nutrient_current_value = nutrient_current_values[nutrient_id];
        
        // Adjust the bad percent
        float bad_percent = std::fmin(1.0,
                                      nutrient_amount_per_100g * *amount_to_reach_quota / 100
                                        / (nutrient_upper_limit - nutrient_current_value));
        if (bad_percent > max_bad_percent)
            max_bad_percent = bad_percent;
    }
    float bad_percent = std::fmin(1.0, *amount_to_reach_quota / (total_meal_weight_limit - total_meal_weight));
    if (bad_percent > max_bad_percent)
        max_bad_percent = bad_percent;
    
    return max_bad_percent / total_good_percent;
}


float foods::get_food_rank_with_current_values(food *f,
                          std::vector<std::pair<int, float> > &good_nutrients,
                          std::vector<std::pair<int, float> > &bad_nutrients,
                            std::unordered_map<int, float> &nutrient_current_values,
                           float *good_grams,
                           float *bad_grams,
                        float *amount_to_reach_quota,
                                               float hard_limit_K,
                                               float weight_limit_K,
                                               float total_meal_weight,
                                               float total_meal_weight_limit)
{
    float max_amount_before_limits = 1000000000;
    float max_amount_to_reach_quota = 0;
    
    /*
     *  We will save in "max_amount_to_before_limits" the maximum amount of this food that makes sense
     *  to eat to reach quota for all good nutrients that this food has
     *
     *  We will save in "max_amount_to_reach_quota" the maximum amount of this food that we can eat
     *  before we reach the upper limit on at least one good or bad nutrient
     *  The idea behind that is this: we want the food with the best rank as much as possible but no more
     *  than limits on nutrients
     */

    std::unordered_set<int> priority_nutrients = {};//1096, 1098, 1180, 1185};//1096, 1098, 1100, 1102, 1124, 1176, 1177, 1180, 1185, 1316};

    std::unordered_map<int, float> nutrient_upper_limits = {
        {},
    };
    
    // Determine how many good grams in 100 grams of this food
    // A good gram is a gram that makes us closer to the goal of matching nutrient_current_values
    //  with the quota
    *good_grams = 0;
    float priority_good_grams = 0;
    for (auto &n_i : good_nutrients)
    {
        int nutrient_id = n_i.first;
        auto *nutrient_aug_data = get_nutrient_augmented_data(nutrient_id);
        
        float nutrient_quota = n_i.second;
        
        // Determine the upper limit of a nutrient
        float nutrient_upper_limit;
        if (!nutrient_aug_data)
            nutrient_upper_limit = nutrient_quota * hard_limit_K;
        else
        {
            nutrient_upper_limit = nutrient_aug_data->max_recommended_amount_;
            if (nutrient_upper_limit == -1)
                nutrient_upper_limit = 1000000000;
            else
                nutrient_upper_limit *= hard_limit_K;
        }
        
        float nutrient_amount_per_100g = f->get_nutrient_amount_fast(nutrient_id);
        if (!nutrient_amount_per_100g)
            continue;
        
        float nutrient_current_value = nutrient_current_values[nutrient_id];
        
        /*if (f - &all_foods_[0] == 335604)
        {
            std::cerr << "nutrient_id=" << nutrient_id << ", nutrient_amount_per_100g=" <<
            nutrient_amount_per_100g << ", nutrient_current_value=" << nutrient_current_value <<
            ", max_amount_before_limits=" << max_amount_before_limits << ", max_amount_to_reach_quota=" << max_amount_to_reach_quota << ", nutrient_quota=" << nutrient_quota << std::endl;
        }*/
        
        // If this food contains a nutrient for which we've already reached the maximum amount then
        //  don't consider this food at all
        if (nutrient_current_value >= nutrient_upper_limit)
        {
            *amount_to_reach_quota = 0;
            *good_grams = 0;
            return 1000000000;
            //continue;
        }
        
        // "m" is how much this food in grams we can eat to reach the quota with this nutrient
        float m = 100.0 * (nutrient_upper_limit - nutrient_current_value) / nutrient_amount_per_100g;
        if (m < max_amount_before_limits)
            max_amount_before_limits = m;
        m = 100.0 * (nutrient_quota - nutrient_current_value) / nutrient_amount_per_100g;
        if (m > max_amount_to_reach_quota)
            max_amount_to_reach_quota = m;
    
        /*if (f - &all_foods_[0] == 335604)
        {
            std::cerr << "m=" << m << std::endl;
        }*/
        
        // Can't take this food at all because we reach limits on this good nutrient
        if (max_amount_before_limits <= 0)
        {
            *amount_to_reach_quota = 0;
            *good_grams = 0;
            return 1000000000;
        }
        
        /*if (f - &all_foods_[0] == 171910)
        {
            std::cerr << "m=" << m << ", min_amount_to_reach_quota=" << min_amount_to_reach_quota << std::endl;
        }*/
        
        float grams_delta;
        if (nutrient_current_value <= nutrient_quota)
            grams_delta = 100 * std::fmin(nutrient_amount_per_100g / (nutrient_quota - nutrient_current_value), 1.0);
        else
            grams_delta = 0;
        
        *good_grams += grams_delta;
        
        if (priority_nutrients.find(nutrient_id) != priority_nutrients.end())
        {
            /*if (f - &all_foods_[0] == 350849)
            {
                std::cerr << "priority_nutrients.find: grams_delta=" << grams_delta << ", nutrient_id=" << nutrient_id << std::endl;
            }*/
            priority_good_grams += grams_delta;
        }
    }

    // No reason to take this food at all because it does not increase good nutrient consumption
    if (max_amount_to_reach_quota <= 0)
    {
        *amount_to_reach_quota = 0;
        *good_grams = 0;
        return 1000000000;
    }
    
    /*if (f - &all_foods_[0] == 167512)
    {
        std::cerr << "*amount_to_reach_quota=" << *amount_to_reach_quota << std::endl;
    }*/
    
    // Determine how many bad grams in 100 grams
    // By default we use the weight of the food as a bad nutrient with the limit of 5kg
    *bad_grams = weight_limit_K;//2;
    for (auto &n_i : bad_nutrients)
    {
        int nutrient_id = n_i.first;
        float nutrient_quota = n_i.second;
        float nutrient_amount_per_100g = f->get_nutrient_amount_fast(nutrient_id);
        float nutrient_current_value = nutrient_current_values[nutrient_id];
        
        /*if (f - &all_foods_[0] == 171406)
        {
            std::cerr << "nutrient_id=" << nutrient_id << ", nutrient_amount_per_100g=" <<
            nutrient_amount_per_100g << ", nutrient_current_value=" << nutrient_current_value << ", nutrient_quota=" << nutrient_quota << std::endl;
        }*/
        
        /*if (nutrient_amount_per_100g && nutrient_current_value +
            nutrient_amount_per_100g * *amount_to_reach_quota / 100.0 >= nutrient_quota)
        {
            *amount_to_reach_quota = 0;
            return 1000000000;
            //continue;
        }*/
        
        if (nutrient_amount_per_100g)
        {
            float m = 100.0 * (nutrient_quota - nutrient_current_value) / nutrient_amount_per_100g;
            if (m < max_amount_before_limits)
                max_amount_before_limits = m;
            
            /*if (f - &all_foods_[0] == 335604)
            {
                std::cerr << "(2) m=" << m << ", max_amount_before_limits=" << max_amount_before_limits << std::endl;
            }*/
        }

        // Can't take this food at all because with any amount of that we reach the limit on this bad nutrient
        if (max_amount_before_limits <= 0)
        {
            *amount_to_reach_quota = 0;
            *good_grams = 0;
            return 1000000000;
        }

        *bad_grams += 100 * nutrient_amount_per_100g / nutrient_quota;
    }
    
    // Check the total meal weight limit
    if (total_meal_weight + max_amount_before_limits > total_meal_weight_limit)
        max_amount_before_limits = std::fmax(0, total_meal_weight_limit - total_meal_weight);
        
    // Try to take the maximum amount of this food that makes sense to take to maximize good nutrients, but
    //  no more than we can take it before reach limits either on good or bad nutrients
    *amount_to_reach_quota = std::fmin(max_amount_before_limits, max_amount_to_reach_quota);

    /*if (f - &all_foods_[0] == 335604)
    {
        std::cerr << "amount_to_reach_quota=" << *amount_to_reach_quota << "max_amount_before_limits=" << max_amount_before_limits << ", max_amount_to_reach_quota=" << max_amount_to_reach_quota << std::endl;
    }*/
    
    if (priority_good_grams)
        return -priority_good_grams / *bad_grams;
    else
        return *bad_grams / *good_grams;
}

void foods::auto_balance(std::vector<std::pair<int, float> > &foods_to_balance_in,
                         std::vector<std::pair<int, float> > &good_nutrients,
                         std::vector<std::pair<int, float> > &bad_nutrients,
                         recipes *rec,
                            float hard_limit_K,
                         float weight_limit_K,
                         const std::vector<std::string> &healthy_search_words)
{
    std::map<int, float> foods_to_balance(foods_to_balance_in.begin(), foods_to_balance_in.end());

    std::string lower_food_name_temp_, lower_food_category_temp_;

    std::cerr << "healthy_search_words=[";
    for (auto &w : healthy_search_words)
        std::cerr << w << " ";
    std::cerr << "]" << std::endl;
    
    for (int i = 0; i < 100; ++i)
    {
        // Fill nutrient_current_values based on food information
        std::unordered_map<int, float> nutrient_current_values;
        float total_meal_weight = 0;
        for (auto &x : foods_to_balance)
        {
            int food_id = x.first;
            food *f = get_food(food_id);
            if (!f)
                f = rec->get_food(food_id);
            if (!f || f->food_name_.empty())
                continue;
            for (auto &y : good_nutrients)
            {
                nutrient_current_values[y.first] += x.second * f->get_nutrient_amount_fast(y.first) / 100.0;
            }
            for (auto &y : bad_nutrients)
            {
                nutrient_current_values[y.first] += x.second * f->get_nutrient_amount_fast(y.first) / 100.0;
            }
            total_meal_weight += x.second;
        }
        
        // Check if we reached quota for all good nutrients
        bool b = true;
        std::cerr << "good nutrients" << std::endl;
        for (auto &y : good_nutrients)
        {
            if (nutrient_current_values[y.first] >= y.second)
                std::cerr << "+ ";
            else
                std::cerr << "- ";
            std::cerr << "[" << y.first << "]=" << nutrient_current_values[y.first] << ", quota=" << y.second
                << " " << get_nutrient_middle_name(y.first) << std::endl;
            if (nutrient_current_values[y.first] < y.second)
                b = false;
        }
        std::cerr << "bad nutrients" << std::endl;
        for (auto &y : bad_nutrients)
        {
            std::cerr << "[" << y.first << "]=" << nutrient_current_values[y.first] << ", quota=" << y.second
                << " " << get_nutrient_middle_name(y.first) << std::endl;
            if (nutrient_current_values[y.first] < y.second)
                b = false;
        }
        std::cerr << "total_meal_weight=" << total_meal_weight << std::endl;
        
        // Reach the quota at all good nutrients - great!
        if (b)
            break;
        
        // Iterate all foods and search for the best food in terms of getting
        //  the most nutritional value and take the amount if its food that will make up
        //  norm
        int best_food = -1;
        float best_rank = 1000000000;
        float best_amount_to_reach_quota = 1000000000;
        int food_id = 0;
        int total_foods_considered = 0;
        for (auto &f : all_foods_)
        {
            if (f.food_name_.empty() || f.food_category_ == "sub_sample_food")
            {
                ++food_id;
                continue;
            }

            // Check filters for words
            lower_food_name_temp_.clear();
            lower_food_category_temp_.clear();
            if (!healthy_search_words.empty())
            {
                bool is_filtered = true;
                lower_str(f.food_name_, lower_food_name_temp_);
                lower_str(f.food_category_, lower_food_category_temp_);
                
                // Search for presence of AT LEAST ONE OF words
                for (auto &x : healthy_search_words)
                {
                    if (lower_food_name_temp_.find(x) != std::string::npos ||
                        lower_food_category_temp_.find(x) != std::string::npos)
                    {
                        is_filtered = false;
                        break;
                    }
                }
                if (is_filtered)
                {
                    ++food_id;
                    continue;
                }
            } // if (!healthy_search_words.empty())
            
            float good_grams, bad_grams, amount_to_reach_quota;
            float rank =
                get_food_rank_with_current_values2(&f, good_nutrients, bad_nutrients, nutrient_current_values, &good_grams, &bad_grams, &amount_to_reach_quota, hard_limit_K, weight_limit_K, total_meal_weight, 5000);
            
            ++total_foods_considered;
            
            /*if (food_id == 167512)
            {
                std::cerr << "amount_to_reach_quota=" << amount_to_reach_quota << std::endl;
            }*/
                
            // If we need food less than it's portion size then don't count this food
            //if (!f.food_standard_weight_ || amount_to_reach_quota >= f.food_standard_weight_)
            {
                // Take only one portion size - if we need more of this food then it will be
                //  increased on next iterations
              //  if (f.food_standard_weight_)
                //    amount_to_reach_quota = f.food_standard_weight_;
                
                if (best_food == -1 || rank < best_rank)
                {
                    best_food = food_id;
                    best_rank = rank;
                    best_amount_to_reach_quota = amount_to_reach_quota;
                }
            }
            
            ++food_id;
        } // for (auto &f : all_foods_)
        
        std::cerr << "best_food=" << best_food << ", best_rank=" << best_rank << ", best_amount_to_reach_quota=" <<
            best_amount_to_reach_quota << ", total_foods_considered=" << total_foods_considered << std::endl;
        
        if (best_food == -1 || !best_amount_to_reach_quota)
            break;
        
        float food_amount_before = foods_to_balance[best_food];
        
        // Update food amount
        foods_to_balance[best_food] += best_amount_to_reach_quota;
        
        std::cerr << "Added food: " << best_food << ", with total amount " <<
            foods_to_balance[best_food] << ", best_amount_to_reach_quota=" << best_amount_to_reach_quota
            << " best_rank=" << best_rank << ", food_amount_before=" << food_amount_before << std::endl;
        
    } // while (true)
    
    std::cerr << "Foods after auto balance:" << std::endl;
    for (auto &x : foods_to_balance)
    {
        int food_id = x.first;
        food *f = get_food(food_id);
        if (!f)
            f = rec->get_food(food_id);
        std::cerr << "[" << food_id << "] " << f->food_name_
            << " " << (int)x.second << " (portion=" << f->food_standard_weight_ << ")" << std::endl;
    }
    
    for (auto &x : foods_to_balance_in)
    {
        auto it = foods_to_balance.find(x.first);
        if (it != foods_to_balance.end())
        {
            x = *it;
            foods_to_balance.erase(it);
        }
    }
    for (auto &x : foods_to_balance)
        foods_to_balance_in.push_back(x);
}

}
