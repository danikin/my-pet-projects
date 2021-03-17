/*
*       balanced_diet_recipe.cpp
*
*       (C) Denis Anikin 2020
*
*       Impl for the balanced diet recipes
*
*/

#include <iostream>

#include "balanced_diet_recipe.h"

namespace balanced_diet
{

recipes::recipes(foods &fds, std::vector<recipe> &rec) : rec_(rec)
{
    std::map<int, float> nutrients_temp_map;
    
    // Create a food entry for each recipe
    for (auto &r : rec)
    {
        // Iterate all foods for this recipe
        nutrients_temp_map.clear();
        float total_recipe_weight = 0;
        for (auto &x : r.recipe_foods_)
        {
            //std::cerr << "recipe: " << r.recipe_name_ << ", food_id=" << x.first << ", food amount=" << x.second << std::endl;
            
            foods::food *f = fds.get_food(x.first);
            if (!f || f->food_name_.empty())
                continue;
            float food_amount = x.second;
            total_recipe_weight += food_amount;
            
            // Iterate all nutrients for this food and fill nutrients_temp_map
            //  with amount of nutrients
            for (auto &y : f->nutrients_)
            {
                nutrients_temp_map[y.first] += y.second * food_amount;
                //std::cerr << "nutrients_temp_map: " << y.first << "->" << y.second * food_amount << std::endl;
            }
        }
        
        if (!total_recipe_weight)
            continue;
        
        //std::cerr << "total_recipe_weight=" << total_recipe_weight << std::endl;
        
        // Now we have amount of each nutrient per each food in this recipe
        //  divide it by total_recipe_weight to get nutrients by 100g in the recipe
        //  and fill f's internal data
        foods::food new_food;
        new_food.food_name_ = r.recipe_name_;
        for (auto &x : nutrients_temp_map)
        {
            x.second /= total_recipe_weight;
            new_food.nutrients_.push_back(x);
            new_food.nutrients_hash_.insert(x);
            switch (x.first)
            {
                case 1003: new_food.fast_access_prot_ = x.second;break;
                case 1008: new_food.fast_access_cal_ = x.second;break;
            }
        }
        
        // Fill nutrients and percent of daily norms sorted by that
        for (auto &x : new_food.nutrients_)
        {
            float norm = fds.get_nutrient_recommended_min_amout(x.first);
            new_food.nutrients_sorted_daily_norm_.push_back({x.first, (norm ? x.second / norm : 0)});
        }
        std::sort(new_food.nutrients_sorted_daily_norm_.begin(), new_food.nutrients_sorted_daily_norm_.end(),
                    [](const std::pair<int, float> &a,
                                    const std::pair<int, float> &b){
            return a.second > b.second;
        });
        
        //std::cerr << "recipes::recipes: new_food.food_name_=" << new_food.food_name_ <<
          //  ", r.recipe_food_id_=" << r.recipe_food_id_ << std::endl;
        
        // Save the food
        foods_[r.recipe_food_id_] = new_food;
    } // for (auto &r : rec)
}

foods::food *recipes::get_food(int recipe_food_id)
{
    auto it = foods_.find(recipe_food_id);
    if (it == foods_.end())
        return NULL;
    else
        return &it->second;
}

} // namespace balanced_diet
