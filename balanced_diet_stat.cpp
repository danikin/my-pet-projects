/*
*       balanced_diet_stat.cpp
*
*       (C) Denis Anikin 2020
*
*       Statistics for the balanced diet
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
#include <unordered_set>
#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "balanced_diet.h"
#include "balanced_diet_mfp.h"

namespace balanced_diet
{

void divide_into_words_tabs(const std::string &str, std::vector<std::string> &words)
{
    auto i = str.begin();
    while (i != str.end())
    {
        auto j = std::find(i, str.end(), '\t');
        if (j == str.end())
        {
            words.push_back(std::string(i, str.end()));
            break;
        }
        else
            words.push_back(std::string(i, j));

        while (*j == '\t')
            ++j;
        i = j;
    }
}

bool is_separator(char c)
{
    return c == ' ' || c == ',' || c == '/' || c == '"' || c == '\'' || c == '-' || c == '(' || c == ')' || c == '*'
        || (c >= '0' && c <= '9') || c == '{' || c == '}' || c == '.' || c == '&' || c == '%';
}

void divide_into_words2(const std::string &str, std::vector<std::string> &words)
{
    auto i = str.begin();
    while (i != str.end())
    {
        auto j = i;
        while (!is_separator(*j) && j != str.end())
            ++j;
        if (j == str.end())
        {
            words.push_back(std::string(i, str.end()));
            break;
        }
        else
            words.push_back(std::string(i, j));

        while (is_separator(*j))
            ++j;
        i = j;
    }
}

void dish_satiety_info::add(int uid, float influence,
                            float cal, float prot, float fat, float fiber, float carbs, float sodium, float sugar, float potass)
{
    influence_ += influence;
    
    // Add influence per a user per this food
    if (user_influence_.find(uid) == user_influence_.end())
        user_influence_.insert({uid, {0, 0}});
    user_influence_[uid].first += influence;
    ++user_influence_[uid].second;
    
    if (influence >= 0)
        ++n_dish_good_days_;
    else
        ++n_dish_bad_days_;
    
    cal_ += cal;
    prot_ += prot;
    fat_ += fat;
    fiber_ += fiber;
    carbs_ += carbs;
    sodium_ += sodium;
    sugar_ += sugar;
    potass_ += potass;

    if (cal)
    {
        prot_cal_ratio_ += 4.0 * prot / cal;
        fat_cal_ratio_ += 9.0 * fat / cal;
        fiber_cal_ratio_ += 4.0 * fiber / cal;
        sugar_cal_ratio_ += 4.0 * sugar / cal;
        sodium_cal_ratio_ += sodium / cal;
        potass_cal_ratio_ += potass / cal;
        ++cardinality_;
    }
}

void dish_satiety_info::add_meal_xe(float pe, float fe, float fbe)
{
    meal_pe_ += pe;
    meal_fe_ += fe;
    meal_fbe_ += fbe;
    ++num_meals_;
}

void dish_satiety_info::make_avg(const std::vector<mfp_user> &mfp_users)
{
    if (!user_influence_.empty())
    {
        // For influence take average from user first and then overall average
        // That's to avoid bias towards users who eat some particular food a lot
        influence_ = 0;
        for (auto &u : user_influence_)
        {
            influence_ += u.second.first / u.second.second;
            
            // Determine amount of good and bad users who had this dish
            // Note: good user is a user who overall reach the goal and the bad one is vice versa
            if (mfp_users[u.first].user_total_fact_calories)
            {
                if (mfp_users[u.first].user_total_fact_calories <= mfp_users[u.first].user_total_goal_calories)
                    ++n_total_good_users_;
                else
                    ++n_total_bad_users_;
            }
        }
        influence_ /= user_influence_.size();
        
        //influence_ /= cardinality_;
        
        cal_ /= cardinality_;
        prot_ /= cardinality_;
        fat_ /= cardinality_;
        fiber_ /= cardinality_;
        carbs_ /= cardinality_;
        sodium_ /= cardinality_;
        sugar_ /= cardinality_;
        potass_ /= cardinality_;

        prot_cal_ratio_ /= cardinality_;
        fat_cal_ratio_ /= cardinality_;
        fiber_cal_ratio_ /= cardinality_;
        sugar_cal_ratio_ /= cardinality_;
        sodium_cal_ratio_ /= cardinality_;
        potass_cal_ratio_ /= cardinality_;
    }
    if (num_meals_)
    {
        meal_pe_ /= num_meals_;
        meal_fe_ /= num_meals_;
        meal_fbe_ /= num_meals_;
    }
}

int mfp_foods::get_dish_id_by_name(const std::string &dish_name)
{
    // Determine the dish id
    int dish_id;
    // If the dish with this name exists then take its id from the map
    auto it = dish_name_to_id_.find(dish_name);
    if (it != dish_name_to_id_.end())
        dish_id = it->second;
    // Otherwise create a new dish with id and update the map
    else
    {
        all_dishes_.push_back({});
        dish_id = all_dishes_.size()-1;
        
        all_dishes_[dish_id].dish_name_ = dish_name;
        
        dish_name_to_id_.insert({dish_name, dish_id});
    }
    
    return dish_id;
}

dish_satiety_info &mfp_foods::get_dish_by_name(const std::string &dish_name)
{
    return all_dishes_[get_dish_id_by_name(dish_name)];
}

void mfp_foods::upload_stat(std::istream &is)
{
    struct dish_word___
    {
        std::string dish_word_;
        int word_cardinality_ = 0;
        int word_good_days_ = 0;
        int word_bad_days_ = 0;
        std::unordered_set<int> dish_word_users_;
        float word_user_prot_cal_ratio_ = 0;
        float word_user_fat_cal_ratio_ = 0;
        float word_user_fiber_cal_ratio_ = 0;
        float word_user_average_fact_calories_ = 0;
    }; // struct dish_word___
    std::unordered_map<std::string, dish_word___> hash_dish_words;
    
    int n_good_days = 0, n_bad_days = 0;

    std::string line;
    std::vector<std::string> words;
    int j = 0;
    while (getline(is, line))
    {
        //std::cerr << "line: " << line << std::endl;
        words.clear();
        divide_into_words_tabs(line, words);
        
        int uid = std::stoi(words[0]);
        std::string date = words[1];
        
        // Total
        float total_fact_cal = 0;
        float total_goal_cal = 0;
        json j_summary = json::parse(words[3]);
        auto &j_total = j_summary["total"];
        auto &j_goal = j_summary["goal"];
        for (auto &j : j_total)
        {
            if (j["name"] == "Calories")
                total_fact_cal += (int)j["value"];
        }
        for (auto &j : j_goal)
        {
            if (j["name"] == "Calories")
                total_goal_cal += (int)j["value"];
        }
        
        float goal_overfulfilment_percent = (total_goal_cal - total_fact_cal) / total_goal_cal;
        
        // Update per user stat and save info per this day per this user
        mfp_users_.resize(uid + 1);
        mfp_users_[uid].user_total_fact_calories += total_fact_cal;
        mfp_users_[uid].user_total_goal_calories += total_goal_cal;
        mfp_users_[uid].days_.push_back({.date_ = date, .total_fact_calories_ = total_fact_cal, .total_goal_calories_ = total_goal_cal});
        
        if (total_goal_cal > 1000 && total_fact_cal > 1000)
        {
            if (goal_overfulfilment_percent >= 0)
                ++n_good_days;
            else
                ++n_bad_days;
        }
        
        // Insert an entry to the day meal
        all_day_meals_.push_back({});
        mfp_day_meal &current_day_meal = all_day_meals_[all_day_meals_.size()-1];
        current_day_meal.total_fact_calories_ = total_fact_cal;
        current_day_meal.total_goal_calories_ = total_goal_cal;
        
        // Iterate all meals for this user for this day
        json j_meals = json::parse(words[2]);
        for (auto &meal : j_meals)
        {
            //std::cerr << "meal: " << meal << std::endl;
            auto &dishes = meal["dishes"];
            
            // Iterate all dishes within a meal
            for (auto &dish : dishes)
            {
                std::string dish_name = dish["name"];
                
                int cal = 0, prot = 0, fat = 0, fiber = 0, carbs = 0, sodium = 0, sugar = 0, potass = 0;
                auto &nutrients = dish["nutritions"];
                for (auto &nutrient : nutrients)
                {
                    if (nutrient["name"] == "Calories")
                        cal = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Protein")
                        prot = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Fat")
                        fat = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Fiber")
                        fiber = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Carbs")
                        carbs = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Sodium")
                        sodium = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Sugar")
                        sugar = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Potass.")
                        potass = std::stoi((std::string)nutrient["value"]);
                }
                
                // Calculate dish satiety - that's average influence
                // Consider only realistic goals and facts
                // Also skip mistakes with calories
                if (total_goal_cal > 1000 && total_fact_cal > 1000 && cal > 0 && cal >= prot * 4 && cal >= fat * 9.1)
                {
                    float dish_influence = goal_overfulfilment_percent * cal / total_fact_cal;

                    if (0
                        //dish_name == "Dean Street Townhouse - Full English Breakfast, 1 plate" ||
                        //dish_name == "Mcdonalds - Pancakes 3 With Syrup, 3 pancakes" ||
                        //dish_name == "Costco - Blueberry Muffin, 1 muffin" ||
                        //dish_name == "Pilsbury - Cinnamon Rolls / Icing, 3 roll" ||
                        //dish_name == "Homemade - Bacon Egg and Cheese Sandwich ( White Bread), 1 sandwich" ||
                        //dish_name == "Homemade - Blueberry Pancakes With Syrup, 4 pancakes"
                        ) // "Breastfeeding - 3-6 Month Old Baby, 1 day")//"Dunkin Donuts - Large Hot Coffee Black, 20 oz")
                    {
                        std::cerr << "DISH [" << uid << "]-" << words[1] << "(" << dish_name
                            << ") : goal_overfulfilment_percent=" << goal_overfulfilment_percent <<
                            ", cal=" << cal << ", prot=" << prot << ", total_fact_cal=" << total_fact_cal << ", dish_influence=" << dish_influence <<
                            ", total_goal_cal=" << total_goal_cal << std::endl;
                    }
                    
                    // Add this dish to the day meal
                    int dish_id = get_dish_id_by_name(dish_name);
                    current_day_meal.day_meal_dish_ids_.push_back(dish_id);
                    
                    mfp_users_[uid].days_[mfp_users_[uid].days_.size() - 1].dish_ids_.push_back(dish_id);
                    
                    // Add info per this dish
                    all_dishes_[dish_id].add(uid, dish_influence, cal, prot, fat, fiber, carbs, sodium, sugar, potass);

                    // Divide the dish name into words and pairs of adjacent words
                    std::vector<std::string> dish_words_temp, dish_words;
                    divide_into_words2(dish_name, dish_words_temp);
                    dish_words = dish_words_temp;
                    for (int i = 0; i < dish_words_temp.size() - 1; ++i)
                        dish_words.push_back(dish_words_temp[i] + " " + dish_words_temp[i+1]);
                    
                    //std::cerr << "dish_name=" << dish_name << std::endl;
                    
                    // Save the info per each word
                    /*for (auto &w : dish_words)
                    {
                        std::string l_str;
                        lower_str(w, l_str);
                        auto &w_info = hash_dish_words[l_str];
                        ++w_info.word_cardinality_;
                        if (dish_influence >= 0)
                            ++w_info.word_good_days_;
                        else
                            ++w_info.word_bad_days_;
                        w_info.dish_word_ = l_str;
                        
                        //std::cerr << "dish_word: " << w << std::endl;
                        w_info.dish_word_users_.insert(uid);
                    }*/
                    
                    //return;
                } // if (total_goal_cal > 1000 && total_fact_cal > 1000 && cal > 0 && cal >= prot * 4 && cal >= fat * 9.1)
                
                //std::cerr << "dish name: " << dish_name << ", cal=" << cal <<
                  //  ", dish_influence: " << std::setprecision(2) << dish_influence * 100 << std::endl;
            }
        } // for (auto &meal : j_meals)
        
        // Determine PE for this meal
        float meal_cal = 0, meal_prot = 0, meal_fat = 0, meal_fiber = 0;
        for (auto &meal : j_meals)
        {
            auto &dishes = meal["dishes"];
            for (auto &dish : dishes)
            {
                int cal = 0, prot = 0, fat = 0, fiber = 0, carbs = 0, sodium = 0, sugar = 0;
                auto &nutrients = dish["nutritions"];
                for (auto &nutrient : nutrients)
                {
                    if (nutrient["name"] == "Calories")
                        cal = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Protein")
                        prot = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Fat")
                        fat = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Fiber")
                        fiber = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Carbs")
                        carbs = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Sodium")
                        sodium = std::stoi((std::string)nutrient["value"]);
                    else
                    if (nutrient["name"] == "Sugar")
                        sugar = std::stoi((std::string)nutrient["value"]);
                }
                if (total_goal_cal > 1000 && total_fact_cal > 1000 && cal > 0 && cal >= prot * 4 && cal >= fat * 9.1)
                {
                    meal_cal += cal;
                    meal_prot += prot;
                    meal_fat += fat;
                    meal_fiber += fiber;
                }
            }
        } // for (auto &meal : j_meals)
        
        // Add meal PE for all dishes within this meal
        float pe = 4.0 * meal_prot / meal_cal;
        float fe = 9.0 * meal_fat / meal_cal;
        float fbe = 4.0 * meal_fiber / meal_cal;
        if (meal_cal)
        {
            for (auto &meal : j_meals)
            {
                auto &dishes = meal["dishes"];
                for (auto &dish : dishes)
                {
                    std::string dish_name = dish["name"];
                    get_dish_by_name(dish_name).add_meal_xe(pe, fe, fbe);
                }
            }
        }
        
        //std::cerr << "total_fact_cal=" << (int)total_fact_cal << ", total_goal_cal=" << (int)total_goal_cal
          //  << ", goal_completion_percent=" << (int)(goal_overfulfilment_percent * 100.0) << std::endl;
        
        //std::cerr << "0: " << words[0] << ", 1: " << words[1] << ", 2: " << words[2] << ", 3: " << words[3] << std::endl;
        //break;
        
        //if (++j >= 100000)
        //    break;
    } // while (getline(is, line))
    
    int total_cardinality = 0;
    for (int dish_id = 0; dish_id < all_dishes_.size(); ++dish_id)
    {
        dish_satiety_info &d = all_dishes_[dish_id];
        
        total_cardinality += d.cardinality_;
        d.make_avg(mfp_users_);
        
        // Consider only representative foods. Like only ones that were consumed by at least five users
        if (d.user_influence_.size() >= 10)
        {
            if (0)//(d.first == "Dunkin Donuts - Large Hot Coffee Black, 20 oz")
            {
                std::cerr << "Dunkin Donuts - Large Hot Coffee Black, 20 oz: influence_=" << d.influence_ <<
                    ", cal_=" << d.cal_ << ", cardinality_=" << d.cardinality_ << std::endl;
            }
            
            float dish_rank = 1.0 * d.n_dish_bad_days_ / (d.n_dish_good_days_ + d.n_dish_bad_days_);
            float user_rank = 1.0 * d.n_total_bad_users_ / d.n_total_good_users_;
            float user_normal_rank = user_rank / (1.0 * 778 / 9116);
            
            //dish_satiety_top_.insert({dish_rank/*d.second.influence_*/, d.second});
            //dish_satiety_top_.insert({d.second.influence_, d.second});
            dish_satiety_top_.insert({user_normal_rank, dish_id});
        }
    } // for (int dish_id = 0; dish_id < all_dishes_.size(); ++dish_id)
    
    std::cerr << "total dishes: " << all_dishes_.size()
        << ", not unique dishes: " << total_cardinality
        << ", total dishes in top: " << dish_satiety_top_.size() << std::endl;

    // Check which users have reached their goals for the second half of data
    int n_improved_users = 0;
    int n_appeared_dishes = 0;
    int n_disappeared_dishes = 0;
    float avg_appeared_pe = 0;
    float avg_disappeared_pe = 0;
    float avg_appeared_fe = 0;
    float avg_disappeared_fe = 0;
    float avg_appeared_fbe = 0;
    float avg_disappeared_fbe = 0;
    std::unordered_map<int, int> all_appeared_dishes, all_disappeared_dishes;
    for (auto &u : mfp_users_)
    {
        float first_goal = 0;
        float first_fact = 0;
        float second_goal = 0;
        float second_fact = 0;
        
        float first_pe = 0, second_pe = 0;
        int n_first = 0, n_second = 0;
        for (int i = 0; i < u.days_.size(); ++i)
        {
            auto &d = u.days_[i];
            if (i <= 6)
            //if (i <= u.days_.size() / 2)
            {
                first_goal += d.total_goal_calories_;
                first_fact += d.total_fact_calories_;
                for (auto dish_id : d.dish_ids_)
                {
                    auto &dish = all_dishes_[dish_id];
                    first_pe += dish.prot_cal_ratio_;
                    ++n_first;
                }
            }
            else
            {
                second_goal += d.total_goal_calories_;
                second_fact += d.total_fact_calories_;
                for (auto dish_id : d.dish_ids_)
                {
                    auto &dish = all_dishes_[dish_id];
                    second_pe += dish.prot_cal_ratio_;
                    ++n_second;
                }
            }
        }
        first_pe /= n_first;
        second_pe /= n_second;
        
        //if (first_fact > first_goal && second_fact <= second_goal)
        
        // Determine the goal not as with fit in calories but as having more portein loaded food
        if (second_pe >= first_pe * 1.2)
        {
            ++n_improved_users;
        
            // Save appeared and disappeared dish ids for an improved user
            std::unordered_set<int> first_dish_ids, second_dish_ids;
            for (int i = 0; i < u.days_.size(); ++i)
            {
                auto &d = u.days_[i];
                if (i <= 6)
                //if (i <= u.days_.size() / 2)
                {
                    for (auto dish_id : d.dish_ids_)
                        first_dish_ids.insert(dish_id);
                }
                else
                {
                    for (auto dish_id : d.dish_ids_)
                        second_dish_ids.insert(dish_id);
                }
            }
            for (auto dish_id : first_dish_ids)
                if (second_dish_ids.find(dish_id) == second_dish_ids.end())
                {
                    ++all_disappeared_dishes[dish_id];
                    u.disappeared_dish_ids_.insert(dish_id);
                }
            for (auto dish_id : second_dish_ids)
                if (first_dish_ids.find(dish_id) == first_dish_ids.end())
                {
                    ++all_appeared_dishes[dish_id];
                    u.appeared_dish_ids_.insert(dish_id);
                }
            n_appeared_dishes += u.appeared_dish_ids_.size();
            n_disappeared_dishes += u.disappeared_dish_ids_.size();
/*
            // Determine average ratios for appeared and dispeared dishes
            for (auto dish_id : u.appeared_dish_ids_)
            {
                auto &dish = all_dishes_[dish_id];
                avg_appeared_pe += dish.prot_cal_ratio_;
                avg_appeared_fe += dish.fat_cal_ratio_;
                avg_appeared_fbe += dish.fiber_cal_ratio_;
            }
            for (auto dish_id : u.disappeared_dish_ids_)
            {
                auto &dish = all_dishes_[dish_id];
                avg_disappeared_pe += dish.prot_cal_ratio_;
                avg_disappeared_fe += dish.fat_cal_ratio_;
                avg_disappeared_fbe += dish.fiber_cal_ratio_;
            }*/
        } // if (first_fact > first_goal && second_fact <= second_goal)
    } // for (auto &u : mfp_users_)

    
    // Determine average ratios for appeared and dispeared dishes
    for (auto &x : all_appeared_dishes)
    {
        int dish_id = x.first;
        auto &dish = all_dishes_[dish_id];
        avg_appeared_pe += dish.prot_cal_ratio_;
        avg_appeared_fe += dish.fat_cal_ratio_;
        avg_appeared_fbe += dish.fiber_cal_ratio_;
    }
    for (auto &x : all_disappeared_dishes)
    {
        int dish_id = x.first;
        auto &dish = all_dishes_[dish_id];
        avg_disappeared_pe += dish.prot_cal_ratio_;
        avg_disappeared_fe += dish.fat_cal_ratio_;
        avg_disappeared_fbe += dish.fiber_cal_ratio_;
    }
/*    avg_appeared_pe /= n_appeared_dishes;
    avg_appeared_fe /= n_appeared_dishes;
    avg_appeared_fbe /= n_appeared_dishes;
    avg_disappeared_pe /= n_disappeared_dishes;
    avg_disappeared_fe /= n_disappeared_dishes;
    avg_disappeared_fbe /= n_disappeared_dishes;*/

    avg_appeared_pe /= all_appeared_dishes.size();
    avg_appeared_fe /= all_appeared_dishes.size();
    avg_appeared_fbe /= all_appeared_dishes.size();
    avg_disappeared_pe /= all_disappeared_dishes.size();
    avg_disappeared_fe /= all_disappeared_dishes.size();
    avg_disappeared_fbe /= all_disappeared_dishes.size();

    std::cerr << "Total users: " << mfp_users_.size()
        << ", improved users: " << n_improved_users
        << ", n_appeared_dishes: " << n_appeared_dishes
        << ", n_disappeared_dishes: " << n_disappeared_dishes
        << ", unique appeared dishes: " << all_appeared_dishes.size()
        << ", unique disappeared dishes: " << all_disappeared_dishes.size()
        << ", avg_appeared_pe: " << avg_appeared_pe
        << ", avg_disappeared_pe: " << avg_disappeared_pe
        << ", avg_appeared_fe: " << avg_appeared_fe
        << ", avg_disappeared_fe: " << avg_disappeared_fe
        << ", avg_appeared_fbe: " << avg_appeared_fbe
        << ", avg_disappeared_fbe: " << avg_disappeared_fbe
        << std::endl;
    
    // Don't count dishes that are both appeared and disappeared
    for (auto it = all_appeared_dishes.begin(); it != all_appeared_dishes.end();)
    {
        auto it2 = all_disappeared_dishes.find(it->first);
        if (it2 != all_disappeared_dishes.end())
        {
            auto save = it;
            ++save;
            all_appeared_dishes.erase(it);
            all_disappeared_dishes.erase(it2);
            it = save;
        }
        else
            ++it;
    }

    // Determine average ratios for appeared and dispeared dishes after clearing both appeared and disappeared dishes
    avg_appeared_pe = 0;
    avg_disappeared_pe = 0;
    avg_appeared_fe = 0;
    avg_disappeared_fe = 0;
    avg_appeared_fbe = 0;
    avg_disappeared_fbe = 0;
    for (auto &x : all_appeared_dishes)
    {
        int dish_id = x.first;
        auto &dish = all_dishes_[dish_id];
        avg_appeared_pe += dish.prot_cal_ratio_;
        avg_appeared_fe += dish.fat_cal_ratio_;
        avg_appeared_fbe += dish.fiber_cal_ratio_;
    }
    for (auto &x : all_disappeared_dishes)
    {
        int dish_id = x.first;
        auto &dish = all_dishes_[dish_id];
        avg_disappeared_pe += dish.prot_cal_ratio_;
        avg_disappeared_fe += dish.fat_cal_ratio_;
        avg_disappeared_fbe += dish.fiber_cal_ratio_;
    }
    avg_appeared_pe /= all_appeared_dishes.size();
    avg_appeared_fe /= all_appeared_dishes.size();
    avg_appeared_fbe /= all_appeared_dishes.size();
    avg_disappeared_pe /= all_disappeared_dishes.size();
    avg_disappeared_fe /= all_disappeared_dishes.size();
    avg_disappeared_fbe /= all_disappeared_dishes.size();

    std::cerr << "After clearing both appeared and disappeared: "
        << " unique appeared dishes: " << all_appeared_dishes.size()
        << ", unique disappeared dishes: " << all_disappeared_dishes.size()
        << ", avg_appeared_pe: " << avg_appeared_pe
        << ", avg_disappeared_pe: " << avg_disappeared_pe
        << ", avg_appeared_fe: " << avg_appeared_fe
        << ", avg_disappeared_fe: " << avg_disappeared_fe
        << ", avg_appeared_fbe: " << avg_appeared_fbe
        << ", avg_disappeared_fbe: " << avg_disappeared_fbe
        << std::endl;
    
    // Build top for often appearing and disappearing dishes
    std::vector<std::pair<int, int> > top_appeared_dishes, top_disappeared_dishes;
    top_appeared_dishes.resize(all_appeared_dishes.size());
    top_disappeared_dishes.resize(all_disappeared_dishes.size());
    std::copy(all_appeared_dishes.begin(), all_appeared_dishes.end(), top_appeared_dishes.begin());
    std::copy(all_disappeared_dishes.begin(), all_disappeared_dishes.end(), top_disappeared_dishes.begin());
    std::sort(top_appeared_dishes.begin(), top_appeared_dishes.end(), [](const std::pair<int, int> &a, const std::pair<int, int> &b){
        return a.second > b.second;
    });
    std::sort(top_disappeared_dishes.begin(), top_disappeared_dishes.end(), [](const std::pair<int, int> &a, const std::pair<int, int> &b){
        return a.second > b.second;
    });
    
    /*
    // Output the top
    std::cerr << "TOP APPEARED DISHES:" << std::endl << std::endl;
    for (auto &x : top_appeared_dishes)
    {
        int dish_id = x.first;
        auto &dish = all_dishes_[dish_id];
        std::cerr << "[" << x.second << "] " << dish.dish_name_ << std::endl;
    }
    std::cerr << "TOP DISAPPEARED DISHES:" << std::endl << std::endl;
    for (auto &x : top_disappeared_dishes)
    {
        int dish_id = x.first;
        auto &dish = all_dishes_[dish_id];
        std::cerr << "[" << x.second << "] " << dish.dish_name_ << std::endl;
    }*/
    
    // Fill day_meal_ids_ for popular foods
    int links_from_dish_to_meal = 0;
    int links_from_meal_to_dish = 0;
    for (int day_meal_id = 0; day_meal_id < all_day_meals_.size(); ++day_meal_id)
    {
        auto &day_meal = all_day_meals_[day_meal_id];
        links_from_meal_to_dish += day_meal.day_meal_dish_ids_.size();
        for (auto dish_id : day_meal.day_meal_dish_ids_)
        {
            dish_satiety_info &d = all_dishes_[dish_id];
            if (d.user_influence_.size() >= 10)
            {
                // Link the day meal to the dish
                d.day_meal_ids_.push_back(day_meal_id);
                ++links_from_dish_to_meal;
            }
        } // for (auto dish_id : day_meal.day_meal_dish_ids_)
    } // for (int day_meal_id = 0; day_meal_id < all_day_meals_.size(); ++day_meal_id)
    
    // Sort all day_meal_ids in order of decrease of the goal overfulfillment
    for (auto &dish : all_dishes_)
    {
        auto &meals = dish.day_meal_ids_;
        if (!meals.empty())
        {
            std::sort(meals.begin(), meals.end(), [this](int day_meal_id1, int day_meal_id2) {
                auto &day_meal1 = all_day_meals_[day_meal_id1];
                auto &day_meal2 = all_day_meals_[day_meal_id2];
                float goal_overfulfilment_percent1 = (day_meal1.total_goal_calories_ - day_meal1.total_fact_calories_) / day_meal1.total_goal_calories_;
                float goal_overfulfilment_percent2 = (day_meal2.total_goal_calories_ - day_meal2.total_fact_calories_) / day_meal2.total_goal_calories_;
                
                return goal_overfulfilment_percent1 > goal_overfulfilment_percent2;
            });
        }
    }
    
    std::cerr << "total day meals: " << all_day_meals_.size()
        << ", total links from dish to meal: " << links_from_dish_to_meal
        << ", links_from_meal_to_dish: " << links_from_meal_to_dish << std::endl;
    
    // Create a map of macro nutrients to dishes
    /*std::multimap<std::string, std::string> temp_macro_nutrient_to_dish_map;
    for (auto &d : dish_satiety_top_)
    {
        char key[256];
        sprintf(key, "%03d-%02d-%02d-%02d-%02d-%02d-%02d",
                ((int)(d.second.cal_ / 10)) * 10,
                ((int)(d.second.prot_ / 2)) * 2,
                ((int)(d.second.fat_ / 4)) * 4,
                ((int)(d.second.carbs_ / 2)) * 2,
                (int)(d.second.fiber_),
                ((int)(d.second.sugar_ / 2)) * 2,
                ((int)(d.second.sodium_ / 10)) * 10
                );
        temp_macro_nutrient_to_dish_map.insert({key, d.second.dish_name_});
    }
    
    std::cerr << "Macro nutrients to dishes:" << std::endl;
    for (auto &m : temp_macro_nutrient_to_dish_map)
        std::cerr << "[" << m.first << "] " << m.second << std::endl;*/
    
    int n_total_good_users = 0;
    int n_total_bad_users = 0;
    for (auto &u : mfp_users_)
    {
        if (u.user_total_fact_calories)
        {
            if (u.user_total_fact_calories <= u.user_total_goal_calories)
                ++n_total_good_users;
            else
                ++n_total_bad_users;
        }
    }
    
    float average_user_rank = 1.0 * n_total_bad_users / n_total_good_users;
    std::cerr << "total good users: " << n_total_good_users
        << ", total bad users: " << n_total_bad_users
        << ", average user rank: " << average_user_rank
        << std::endl;
    
    //std::cerr << std::endl << "WORST:" << std::endl << std::endl;
    
    // Output top dishes
    float avg_pe = 0, avg_avg_meal_pe = 0, avg_avg_meal_fe = 0, avg_avg_meal_fbe = 0;
    
    float avg_plus_pe = 0;
    float avg_minus_pe = 0;
    float avg_plus_fe = 0;
    float avg_minus_fe = 0;
    float avg_plus_fbe = 0;
    float avg_minus_fbe = 0;
    
    float rank_avg_plus_pe = 0;
    float rank_avg_minus_pe = 0;
    float rank_avg_plus_fe = 0;
    float rank_avg_minus_fe = 0;
    float rank_avg_plus_fbe = 0;
    float rank_avg_minus_fbe = 0;

    float user_rank_avg_plus_pe = 0;
    float user_rank_avg_minus_pe = 0;
    float user_rank_avg_plus_fe = 0;
    float user_rank_avg_minus_fe = 0;
    float user_rank_avg_plus_fbe = 0;
    float user_rank_avg_minus_fbe = 0;
    float user_rank_avg_plus_suge = 0;
    float user_rank_avg_minus_suge = 0;
    float user_rank_avg_plus_sode = 0;
    float user_rank_avg_minus_sode = 0;
    float user_rank_avg_plus_pote = 0;
    float user_rank_avg_minus_pote = 0;
    
    float avg_meal_plus_pe = 0;
    float avg_meal_minus_pe = 0;
    float avg_meal_plus_fbe = 0;
    float avg_meal_minus_fbe = 0;
    
    int n_plus = 0, n_minus = 0;
    int i = 0;
    /*for (auto &t : dish_satiety_top)
    {
        std::cerr << "[" << t.second.user_influence_.size() << "] " << t.second.dish_name_ << ", avg cal=" << (int)t.second.cal_ << ", avg prot=" << t.second.prot_
            << ", PE=" << std::setprecision(2) << t.second.prot_cal_ratio_ << " : " << (int)(t.first*100)
            << ", avg meal PE=" << std::setprecision(2) << t.second.meal_pe_
            << ", avg meal FE=" << std::setprecision(2) << t.second.meal_fe_
            << ", avg meal FBE=" << std::setprecision(2) << t.second.meal_fbe_
            << std::endl;
        avg_pe += t.second.prot_cal_ratio_;
        avg_avg_meal_pe += t.second.meal_pe_;
        avg_avg_meal_fe += t.second.meal_fe_;
        avg_avg_meal_fbe += t.second.meal_fbe_;
        if (++i > 30)
            break;
    }
    std::cerr << "AVG PE=" << avg_pe / i
        << ", AVG avg meal PE=" << avg_avg_meal_pe / i
        << ", AVG avg meal FE=" << avg_avg_meal_fe / i
        << ", AVG avg meal FBE=" << avg_avg_meal_fbe / i
        << std::endl;*/
    
    std::cerr << std::endl << "BEST:" << std::endl << std::endl;
    
    i = 0;
    avg_pe = 0;
    avg_avg_meal_pe = 0;
    avg_avg_meal_fe = 0;
    avg_avg_meal_fbe = 0;
    int n_rank_good_dishes = 0;
    int n_user_rank_good_dishes = 0;
    for (auto t = dish_satiety_top_.begin(); t != dish_satiety_top_.end(); ++t)
    {
        dish_satiety_info &dish = all_dishes_[t->second];
        
        float dish_influence = dish.influence_;
        float dish_absolute_rank = 1.0 * dish.n_dish_bad_days_ / (dish.n_dish_good_days_ + dish.n_dish_bad_days_);
        float rank = 100.0 * dish_absolute_rank / 0.27;
        float user_rank = 1.0 * dish.n_total_bad_users_ / dish.n_total_good_users_;

        if ((int)rank <= 100)
        {
            rank_avg_plus_pe += dish.prot_cal_ratio_;
            rank_avg_plus_fe += dish.fat_cal_ratio_;
            rank_avg_plus_fbe += dish.fiber_cal_ratio_;
            ++n_rank_good_dishes;
        }
        else
        {
            rank_avg_minus_pe += dish.prot_cal_ratio_;
            rank_avg_minus_fe += dish.fat_cal_ratio_;
            rank_avg_minus_fbe += dish.fiber_cal_ratio_;
        }
        
        if (user_rank <= average_user_rank)
        {
            user_rank_avg_plus_pe += dish.prot_cal_ratio_;
            user_rank_avg_plus_fe += dish.fat_cal_ratio_;
            user_rank_avg_plus_fbe += dish.fiber_cal_ratio_;
            user_rank_avg_plus_suge += dish.sugar_cal_ratio_;
            user_rank_avg_plus_sode += dish.sodium_cal_ratio_;
            user_rank_avg_plus_pote += dish.potass_cal_ratio_;
            ++n_user_rank_good_dishes;
        }
        else
        {
            user_rank_avg_minus_pe += dish.prot_cal_ratio_;
            user_rank_avg_minus_fe += dish.fat_cal_ratio_;
            user_rank_avg_minus_fbe += dish.fiber_cal_ratio_;
            user_rank_avg_minus_suge += dish.sugar_cal_ratio_;
            user_rank_avg_minus_sode += dish.sodium_cal_ratio_;
            user_rank_avg_minus_pote += dish.potass_cal_ratio_;
        }

        /*std::cerr << "[" << dish.user_influence_.size() << "] " << dish.dish_name_ << ", avg cal=" << (int)dish.cal_ << ", avg prot=" << dish.prot_
        << ", PE=" << std::setprecision(2) << dish.prot_cal_ratio_
        << ", FE=" << std::setprecision(2) << dish.fat_cal_ratio_
        << ", FBE=" << std::setprecision(2) << dish.fiber_cal_ratio_
        << " : " << (int)(dish_influence*100) << "(" << (int)(100.0 * t->first / 0.27) << "%)"
        << ", avg meal PE=" << std::setprecision(2) << dish.meal_pe_
        << ", avg meal FE=" << std::setprecision(2) << dish.meal_fe_
        << ", avg meal FBE=" << std::setprecision(2) << dish.meal_fbe_
        << ", G=" << std::setprecision(2) << dish.n_dish_good_days_
        << ", B=" << std::setprecision(2) << dish.n_dish_bad_days_
        << std::endl;*/
        avg_pe += dish.prot_cal_ratio_;
        avg_avg_meal_pe += dish.meal_pe_;
        avg_avg_meal_fe += dish.meal_fe_;
        avg_avg_meal_fbe += dish.meal_fbe_;
        
        /*if (dish.fat_cal_ratio_ > 1 || dish.prot_cal_ratio_ < 0)
        {
            std::cout << "[" << dish.user_influence_.size() << "] " << dish.dish_name_ << ", avg cal=" << (int)dish.cal_ << ", avg prot=" << dish.prot_
            << ", PE=" << std::setprecision(2) << dish.prot_cal_ratio_
            << ", FE=" << std::setprecision(2) << dish.fat_cal_ratio_
            << ", FBE=" << std::setprecision(2) << dish.fiber_cal_ratio_
            << " : " << (int)(dish_influence*100)
            << ", avg meal PE=" << std::setprecision(2) << dish.meal_pe_
            << ", avg meal FE=" << std::setprecision(2) << dish.meal_fe_
            << ", avg meal FBE=" << std::setprecision(2) << dish.meal_fbe_
            << std::endl;
        }*/
        
        if (dish_influence >= 0)
        {
            avg_plus_pe += dish.prot_cal_ratio_;
            avg_plus_fe += dish.fat_cal_ratio_;
            avg_plus_fbe += dish.fiber_cal_ratio_;
            avg_meal_plus_pe += dish.meal_pe_;
            avg_meal_plus_fbe += dish.meal_fbe_;
            ++n_plus;
        }
        else
        {
            avg_minus_pe += dish.prot_cal_ratio_;
            avg_minus_fe += dish.fat_cal_ratio_;
            avg_minus_fbe += dish.fiber_cal_ratio_;
            avg_meal_minus_pe += dish.meal_pe_;
            avg_meal_minus_fbe += dish.meal_fbe_;
            ++n_minus;
        }
        //if (i > 10000)
          //  break;
        ++i;
    } // for (auto t = dish_satiety_top_.begin(); t != dish_satiety_top_.end(); ++t)
    
    std::cerr << "AVG PE=" << avg_pe / i
        << ", AVG avg meal PE=" << avg_avg_meal_pe / i
        << ", AVG avg meal FE=" << avg_avg_meal_fe / i
        << ", AVG avg meal FBE=" << avg_avg_meal_fbe / i
        << std::endl;

    std::cerr << "AVG +PE=" << avg_plus_pe / n_plus
        << ", AVG -PE=" << avg_minus_pe / n_minus
        << ", AVG avg meal +PE=" << avg_meal_plus_pe / n_plus
        << ", AVG avg meal -PE=" << avg_meal_minus_pe / n_minus
        << ", AVG avg meal +FBE=" << avg_meal_plus_fbe / n_plus
        << ", AVG avg meal -FBE=" << avg_meal_minus_fbe / n_minus
        << std::endl;

    std::cerr << "AVG +FE=" << avg_plus_fe / n_plus
        << ", AVG -FE=" << avg_minus_fe / n_minus
        << ", AVG +FBE=" << avg_plus_fbe / n_plus
        << ", AVG -FBE=" << avg_minus_fbe / n_minus
        << std::endl;

    std::cerr << "RANK AVG +PE=" << rank_avg_plus_pe / n_rank_good_dishes
        << ", RANK AVG -PE=" << rank_avg_minus_pe / (dish_satiety_top_.size() - n_rank_good_dishes)
        << ", RANK AVG +FE=" << rank_avg_plus_fe / n_rank_good_dishes
        << ", RANK AVG -FE=" << rank_avg_minus_fe / (dish_satiety_top_.size() - n_rank_good_dishes)
        << ", RANK AVG +FBE=" << rank_avg_plus_fbe / n_rank_good_dishes
        << ", RANK AVG -FBE=" << rank_avg_minus_fbe / (dish_satiety_top_.size() - n_rank_good_dishes)
        << std::endl;

    std::cerr << "USER RANK AVG +PE=" << user_rank_avg_plus_pe / n_user_rank_good_dishes
        << ", USER RANK AVG -PE=" << user_rank_avg_minus_pe / (dish_satiety_top_.size() - n_user_rank_good_dishes)
        << ", USER RANK AVG +FE=" << user_rank_avg_plus_fe / n_user_rank_good_dishes
        << ", USER RANK AVG -FE=" << user_rank_avg_minus_fe / (dish_satiety_top_.size() - n_user_rank_good_dishes)
        << ", USER RANK AVG +FBE=" << user_rank_avg_plus_fbe / n_user_rank_good_dishes
        << ", USER RANK AVG -FBE=" << user_rank_avg_minus_fbe / (dish_satiety_top_.size() - n_user_rank_good_dishes)
        << ", USER RANK AVG +SUGE=" << user_rank_avg_plus_suge / n_user_rank_good_dishes
        << ", USER RANK AVG -SUGE=" << user_rank_avg_minus_suge / (dish_satiety_top_.size() - n_user_rank_good_dishes)
        << ", USER RANK AVG +SODE=" << user_rank_avg_plus_sode / n_user_rank_good_dishes
        << ", USER RANK AVG -SODE=" << user_rank_avg_minus_sode / (dish_satiety_top_.size() - n_user_rank_good_dishes)
        << ", USER RANK AVG +POTE=" << user_rank_avg_plus_pote / n_user_rank_good_dishes
        << ", USER RANK AVG -POTE=" << user_rank_avg_minus_pote / (dish_satiety_top_.size() - n_user_rank_good_dishes)
    << std::endl;
    
    float g_avg_pe = 0, g_avg_avg_meal_pe = 0, g_avg_avg_meal_fe = 0, g_avg_avg_meal_fbe = 0;
    for (auto &t : dish_satiety_top_)
    {
        dish_satiety_info &dish = all_dishes_[t.second];;
        
        g_avg_pe += dish.prot_cal_ratio_;
        g_avg_avg_meal_pe += dish.meal_pe_;
        g_avg_avg_meal_fe += dish.meal_fe_;
        g_avg_avg_meal_fbe += dish.meal_fbe_;
    } // for (auto &t : dish_satiety_top_)
   
    std::cerr << "Total dishes=" << dish_satiety_top_.size() << std::endl;
    std::cerr << "Total good dishes=" << n_rank_good_dishes << std::endl;

    std::cerr << "GRAND AVG PE=" << g_avg_pe / dish_satiety_top_.size()
        << ", GRAND AVG avg meal PE=" << g_avg_avg_meal_pe / dish_satiety_top_.size()
        << ", GRAND AVG avg meal FE=" << g_avg_avg_meal_fe / dish_satiety_top_.size()
        << ", GRAND AVG avg meal FBE=" << g_avg_avg_meal_fbe / dish_satiety_top_.size()
        << ", Total good days=" << n_good_days
        << ", Total bad days=" << n_bad_days
        << ", Average bad/good ratio=" << 1.0 * n_bad_days / (n_good_days + n_bad_days)
        << std::endl;
    
    std::cerr << std::endl;
    
    /*std::cerr << "WORDS:" << std::endl << std::endl;
    
    // Output the top of words
    std::multimap<float, dish_word___*> word_top;
    for (auto &w : hash_dish_words)
    {
        // Consder only representative dish words
        if (w.second.word_cardinality_ >= 10 && w.second.dish_word_users_.size() >= 30)
        {
            //float word_rank = 1.0 * w.second.word_bad_days_ / (w.second.word_good_days_ + w.second.word_bad_days_);
            float word_rank = 0;
            for (auto &u : w.second.dish_word_users_)
                if (mfp_users_[u].user_total_fact_calories > mfp_users_[u].user_total_goal_calories)
                    ++word_rank;
            word_rank /= w.second.dish_word_users_.size();
            
            // Determine prot/fat/fiber to cal ratio for users who had dishes with this word
            auto &word = w.second;
            int n = 0;
            for (auto u : w.second.dish_word_users_)
            {
                for (auto &d : mfp_users_[u].days_)
                {
                    for (int dish_id : d.dish_ids_)
                    {
                        auto &dish = all_dishes_[dish_id];
                        word.word_user_prot_cal_ratio_ += dish.prot_cal_ratio_;
                        word.word_user_fat_cal_ratio_ += dish.fat_cal_ratio_;
                        word.word_user_fiber_cal_ratio_ += dish.fiber_cal_ratio_;
                        ++n;
                    }
                }
                word.word_user_average_fact_calories_ += mfp_users_[u].user_total_fact_calories / mfp_users_[u].days_.size();
            }
            word.word_user_prot_cal_ratio_ /= n;
            word.word_user_fat_cal_ratio_ /= n;
            word.word_user_fiber_cal_ratio_ /= n;
            word.word_user_average_fact_calories_ /= w.second.dish_word_users_.size();
            
            word_top.insert({word_rank, &w.second});
        }
    }

    for (auto &w : word_top)
    {
        std::cerr << "[" << w.second->word_cardinality_ << "] " << w.second->dish_word_
            << ", rank=" << w.first
            << ", G=" << w.second->word_good_days_
            << ", B=" << w.second->word_bad_days_

            << ", prot_cal_ratio=" << w.second->word_user_prot_cal_ratio_
            << ", fat_cal_ratio=" << w.second->word_user_fat_cal_ratio_
            << ", fiber_cal_ratio=" << w.second->word_user_fiber_cal_ratio_
        
            << ", avg_day_calories=" << w.second->word_user_average_fact_calories_

            << std::endl;
    }
    
    std::cerr << "Total words:" << word_top.size() << std::endl;*/
}

void mfp_foods::output_dish(int dish_id,
                            const dish_satiety_info &dish,
                            bool output_user_list,
                            int dish_cardinality,
                            std::ostream &output)
{
    float dish_absolute_rank = 1.0 * dish.n_dish_bad_days_ / (dish.n_dish_good_days_ + dish.n_dish_bad_days_);
    float rank = 100.0 * dish_absolute_rank / 0.27;
    float dish_influence = dish.influence_;
    float user_rank = (dish.n_total_good_users_)?
        (1.0 * dish.n_total_bad_users_ / dish.n_total_good_users_) : 1000000000;
    
    output << "{";
    output << "\"dish_id\":" << dish_id << ",";
    if (dish_cardinality != -1)
        output << "\"dish_cardinality\":" << dish_cardinality << ",";
    output << "\"unique_users\":" << dish.user_influence_.size() << ",";
    output << "\"rank\":" << rank << ",";
    output << "\"influence\":" << dish_influence << ",";
    output << "\"user_rank\":" << user_rank << ",";
    output << "\"user_normal_rank\":" << user_rank / (1.0 * 778 / 9116) << ",";
    output << "\"cal\":" << dish.cal_ << ",";
    output << "\"prot\":" << dish.prot_ << ",";
    output << "\"fat\":" << dish.fat_ << ",";
    output << "\"carbs\":" << dish.carbs_ << ",";
    output << "\"fiber\":" << dish.fiber_ << ",";
    output << "\"sugar\":" << dish.sugar_ << ",";
    output << "\"sodium\":" << dish.sodium_ << ",";
    
    std::string js_escaped;
    json_escape(dish.dish_name_, js_escaped);
    output << "\"dish_name\":\"" << js_escaped << "\",";
    
    std::stringstream ss;
    ss << "avg cal=" << (int)dish.cal_ << ", avg prot=" << dish.prot_
    << ", PE=" << std::setprecision(2) << dish.prot_cal_ratio_
    << ", FE=" << std::setprecision(2) << dish.fat_cal_ratio_
    << ", FBE=" << std::setprecision(2) << dish.fiber_cal_ratio_
    << " : " << std::setprecision(2) << dish.influence_*100.0 << "(" << (int)(rank) << "%)"
    << ", avg meal PE=" << std::setprecision(2) << dish.meal_pe_
    << ", avg meal FE=" << std::setprecision(2) << dish.meal_fe_
    << ", avg meal FBE=" << std::setprecision(2) << dish.meal_fbe_
    << ", G=" << std::setprecision(2) << dish.n_dish_good_days_
    << ", B=" << std::setprecision(2) << dish.n_dish_bad_days_
    ;
    
    output << "\"dish_info\":\"" << ss.str() << "\"";
    
    if (output_user_list)
    {
        output << ", \"users\":\"";
        for (auto &u : dish.user_influence_)
            output << " " << u.first << "[" << u.second.first << "," << u.second.second << "]";
        output << "\"";
    }
    
    output << "}" << std::endl;
}

void mfp_foods::suggest_food_from_stat(std::vector<std::string> &search_string_include,
                            std::vector<std::string> &search_string_exclude,
                        std::vector<std::string> &search_string_include_exact,
                        std::vector<std::string> &search_string_exclude_exact,
                            int limit_after_sort,
                            float min_food_rank,
                            float max_food_rank,
                        float protein_to_calorie_percent,
                        float fiber_to_calorie_percent,
                        float min_pfind,
            std::ostream &output)
{
    output << "{\"event\":\"suggest_food_from_stat\",\"data\":[" << std::endl;

    int k = 0;
    std::string lower_dish_name_temp_;
    bool first = true;
    if (limit_after_sort == -1)
        limit_after_sort = 1000;
    //for (auto t = dish_satiety_top_.rbegin(); t != dish_satiety_top_.rend(); ++t)
    for (auto t = dish_satiety_top_.begin(); t != dish_satiety_top_.end(); ++t)
    {
        int dish_id = t->second;
        dish_satiety_info &dish = all_dishes_[dish_id];
        const std::string &dish_name = dish.dish_name_;
    
        //if (rank > 100)
        //    continue;

        // Filter
        lower_dish_name_temp_.clear();
        bool is_filtered = false;
        if (!search_string_include.empty())
        {
            is_filtered = false;
            lower_str(dish_name, lower_dish_name_temp_);
            
            // Search for presence of ALL words
            for (auto &x : search_string_include)
            {
                // The word is not absent - filter the food
                if (lower_dish_name_temp_.find(x) == std::string::npos)
                {
                    /*if (food_id > 1000000)
                        std::cerr << "filtered out: lower_food_name_temp_=" << lower_food_name_temp_ <<
                        ", x=" << x << std::endl;*/
                    is_filtered = true;
                    break;
                }
            }
            if (is_filtered)
                continue;
        } // if (!search_string_include.empty())

            
        // Search for exact match
        for (auto &x : search_string_include_exact)
        {
            // The word is not absent - filter the food
            if (dish_name.find(x) == std::string::npos)
            {
                is_filtered = true;
                break;
            }
        }
        if (is_filtered)
            continue;
            
        if (!search_string_exclude.empty())
        {
            if (lower_dish_name_temp_.empty())
                lower_str(dish_name, lower_dish_name_temp_);
            
            // Search for absence of ALL words
            for (auto &x : search_string_exclude)
            {
                // The word is present - filter the food
                if (lower_dish_name_temp_.find(x) != std::string::npos)
                {
                    is_filtered = true;
                    break;
                }
            }
            if (is_filtered)
                continue;
            
        } // if (!search_string_exclude.empty())

        // Search for exact mismatch
        for (auto &x : search_string_exclude_exact)
        {
            // The word is present - filter the food
            if (dish_name.find(x) != std::string::npos)
            {
                    is_filtered = true;
                    break;
            }
        }
        if (is_filtered)
            continue;
        
        // Output the dish
        if (!first) output << ","; first = false;
        output_dish(dish_id, dish, true, -1, output);
        
        if (++k >= limit_after_sort)
            break;
    } // for (auto t = dish_satiety_top_.begin(); t != dish_satiety_top_.end(); ++t)
    
    output << "]}" << std::endl;
}

void mfp_foods::all_meals_with_dish(int dish_id,
                                    const std::string &dish_name,
                                    int limit_after_sort,
                                    std::ostream &output)
{
    output << "{\"event\":\"all_meals_with_dish\",\"data\":[" << std::endl;
    
    if (dish_id == -1)
        dish_id = get_dish_id_by_name(dish_name);
    
    std::cerr << "dish_id=" << dish_id << ", all_day_meals_.size()=" << all_day_meals_.size() <<
    ", dish_name='" << dish_name << "', all_dishes_[dish_id].day_meal_ids_.size()=" <<
        all_dishes_[dish_id].day_meal_ids_.size() << std::endl;
    
    bool first = true;
    int k = 0;
    for (auto day_meal_id : all_dishes_[dish_id].day_meal_ids_)
    {
        auto &day_meal = all_day_meals_[day_meal_id];
        // Output the day meal info
        if (!first) output << ","; first = false;
        float goal_overfulfilment_percent = (day_meal.total_goal_calories_ - day_meal.total_fact_calories_) / day_meal.total_goal_calories_;
        output << "{\"total_fact_calories\": " << day_meal.total_fact_calories_ << ","
            << "\"total_goal_calories\": " << day_meal.total_goal_calories_ << ","
            << "\"goal_overfulfilment_percent\": " << goal_overfulfilment_percent << ","
        << "\"foods\":[";
        
        // Output all foods within this meal
        bool first2 = true;
        for (auto dish_id : day_meal.day_meal_dish_ids_)
        {
            dish_satiety_info &dish = all_dishes_[dish_id];
            if (!first2) output << ","; first2 = false;
            output_dish(dish_id, dish, false, -1, output);
        }
        
        output << "]}" << std::endl;
        
        if (limit_after_sort != -1)
        {
            if (k++ >= limit_after_sort)
                break;
        }
    } // for (auto &meal : all_dishes_[dish_id].day_meal_ids_)

    output << "], \"dish_substitution\":[" << std::endl;
    
    // Check if this dish is disappeared from somebody's menu - and for all those users get all the appeared
    //  dishes, sort them in order of appearance and show them as a substitution top
    std::map<int, int> appeared_top;
    for (auto &u : mfp_users_)
    {
        auto it = u.disappeared_dish_ids_.find(dish_id);
        if (it != u.disappeared_dish_ids_.end())
        {
            // This dish disappeared from this user's menu - add all appeared dishes to the list
            for (auto dish_id : u.appeared_dish_ids_)
                ++appeared_top[dish_id];
        }
    }
    
    // Now sort top by appearance
    std::vector<std::pair<int, int> > sorted_appeared_top;
    sorted_appeared_top.resize(appeared_top.size());
    std::copy(appeared_top.begin(), appeared_top.end(), sorted_appeared_top.begin());
    std::sort(sorted_appeared_top.begin(), sorted_appeared_top.end(), [](const std::pair<int, int> &a, const std::pair<int, int> &b){
        return a.second > b.second;
    });
    
    // Output the top
    first = true;
    k = 0;
    for (auto &x : sorted_appeared_top)
    {
        int dish_id = x.first;
        auto &dish = all_dishes_[dish_id];
        if (!first) output << ","; first = false;
        output_dish(dish_id, dish, false, x.second, output);
        
        if (k++ >= 10)
            break;
    }

    output << "]}" << std::endl;
}

} // namespace balanced_diet
