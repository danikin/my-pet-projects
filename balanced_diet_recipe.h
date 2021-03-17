/*
*       balanced_diet_recipe.h
*
*       (C) Denis Anikin 2020
*
*       Headers for the balanced diet recipes
*
*/

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <map>

#ifndef _balanced_diet_recipe_h_included_
#define _balanced_diet_recipe_h_included_

#include "balanced_diet.h"

namespace balanced_diet
{

// Custom recipe
struct recipe
{
    // The name of the recipe
    std::string recipe_name_;
    
    int recipe_food_id_;
    
    // IDs and amounts of foods for this recipe
    std::vector<std::pair<int, float> > recipe_foods_;
};

// Custom recipes
class recipes
{
public:
    
    recipes(foods &fds,
                std::vector<recipe> &rec);

    // Returns a food by its recipe-food id
    foods::food *get_food(int recipe_food_id);
    
    std::vector<recipe> &get_recipes_vector()
    {
        return rec_;
    }
    
private:
    
    std::map<int, foods::food> foods_;
    std::vector<recipe> rec_;
};

}

#endif

