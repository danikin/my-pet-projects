/*
*       balanced_diet_food_rank.cpp
*
*       (C) Denis Anikin 2020
*
*       Impl for the balanced food rank algorithms
*
*/

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <map>
#include <set>
#include <array>
#include <string.h>

#include "balanced_diet.h"
#include "balanced_diet_recipe.h"

namespace balanced_diet
{

struct nutrient_satiety_info
{
    int nutrient_id_;
    float amount_to_satiety_;
    float max_k_;
};

std::vector<nutrient_satiety_info> nutrient_satiety = {
    {0, 4300, 1/0.65}, // food weight
    {1003, 150, 1/0.65}, // Protein
    {1079, 1/0.95}, // Fiber
    // Don't cound sodium because there are a lot of unleathy items with lots of added sodium and they does not seem to work
    //{1093, 4000}, // Sodium
    {1092, 5000, 1/0.85}, // Potassium
    {1103, 200, 1/0.85}, // Selenium (logarithmic)
    {1087, 1800, 1/0.55}, // Calcium
    {1091, 1800, 1/0.65}, // Phosphorus
    {1098, 3.5, 1/0.90}, // Copper
    {1090, 800, 1/0.73}, // Magnesium
    {1095, 30, 1/0.83}, // Zinc
    {1101, 650, 1/0.93}, // Manganese (logarithmic)
    {1167, 50, 1/0.87}, // B3 (logarithmic)
    {1177, 850, 1/0.80}, // Folate
    {1109, 50, 1/0.83}, // E (logarithmic)
    {1110, 7000, 1/0.87}, // D
    {1165, 3.5, 1/0.87}, // B1 (logarithmic)
    {1175, 3, 1/0.91}, // B6 (logarithmic)
    {1178, 65, 1/0.91}, // B12
    {1162, 850, 1/0.97}, // C
    {1104, 42000, 1/0.93} // A (logarithmic)
};

float satiety_helper(float bmr,
                        float calorie_100g,
                        float nutrient_amount_100g,
                        float nutrient_amount_to_satiety,
                        float max_k)
{
    // Determine the weight of the food for bmr
    float food_weight_in_bmr = bmr / calorie_100g;
    
    // Now determine the nutrient amount in that weight
    float nutrient_amount_in_bmr = food_weight_in_bmr * nutrient_amount_100g;
    
    // Return how much this food is filling if it only had this nutrient and if a user ate
    //  this food up to the bmr
    // >1 means good and <1 means bad
    // Note: we don't overestimate influence of a single nutrient to satiety - that's why max_k
    return std::fmin(max_k, nutrient_amount_in_bmr / nutrient_amount_to_satiety);
}

float foods::get_food_satiety_per_calorie(food *f)
{
    float bmr = 2000;
    float cal = f->get_nutrient_amount_fast(1008);
    float satiety = 0;
    for (auto &n : nutrient_satiety)
    {
        float nutrient_amount_100g;
        if (!n.nutrient_id_)
            nutrient_amount_100g = 100; // Food weight
        else
            nutrient_amount_100g = f->get_nutrient_amount_fast(n.nutrient_id_);
        
        float s = satiety_helper(bmr, cal, nutrient_amount_100g, n.amount_to_satiety_, n.max_k_);
        
        /*if (f - &all_foods_[0] == 334219)
        {
            std::cerr << "foods::get_food_satiety_per_calorie: n.nutrient_id_=" << n.nutrient_id_
                << ", nutrient_amount_100g=" << nutrient_amount_100g
                << ", cal=" << cal
                << ", s=" << s
                << ", satiety=" << satiety
                << std::endl;
        }*/
        
        satiety += s;
    }
    return satiety;
}
    /*
    // protein 1003 -> 300g
    // weight -> 8kg
    // fiber 1079 -> 50g (0.8)
    // Sodium 1093 -> 10000mg (0.8)
    // Potassium 1092 -> 9500mg (0.8)
    // Selenium 1103 -> 600ug (0.8)
    // Calcium 1087 -> 5500mg (1.2)
    // Phosphorus 1091 -> 4000mg
    // Copper 1098 -> 8mg (0.7)
    // Magnesium 1090 -> 1800g (0.8)
    // Zinc 1095 -> 95mg (0.7)
    // Manganese 1101 -> 12 mg (0.5)
    // B3 1167 -> 80mg (0.6)
    // Folate 1177 -> 1800mcg (0.8)
    // E 1109 -> 100mg (0.7)
    // D 1110 -> 9500IU (0.6)
    // B1 1165 -> 7.5mg (0.5)
    // B6 1175 -> 17mg (0.5)
    // B12 1178 -> 137ug (0.5)
    // A 1104 -> 57000IU (0.5)
    
    float cal = f->get_nutrient_amount_fast(1008);
    float protein = f->get_nutrient_amount_fast(1003);
    float weight = 100.0;
    float fiber = f->get_nutrient_amount_fast(1079);
    float sodium = f->get_nutrient_amount_fast(1093);
    float potassium = f->get_nutrient_amount_fast(1092);
    float selenium = f->get_nutrient_amount_fast(1103);
    float calcium = f->get_nutrient_amount_fast(1087);
    float phosphorus = f->get_nutrient_amount_fast(1091);
    float copper = f->get_nutrient_amount_fast(1098);
    float magnesium = f->get_nutrient_amount_fast(1090);
    float zinc = f->get_nutrient_amount_fast(1095);
    float manganese = f->get_nutrient_amount_fast(1101);
    float vB3 = f->get_nutrient_amount_fast(1167);
    float folate = f->get_nutrient_amount_fast(1177);
    float vE = f->get_nutrient_amount_fast(1109);
    float vD = f->get_nutrient_amount_fast(1110);
    float vB1 = f->get_nutrient_amount_fast(1165);
    float vB6 = f->get_nutrient_amount_fast(1175);
    float vB12 = f->get_nutrient_amount_fast(1178);
    float vA = f->get_nutrient_amount_fast(1104);

    float satiety =
        satiety_helper(protein, 300, 1) +
    satiety_helper(weight, 8000, 0.8) +
    satiety_helper(fiber, 50, 0.8) +
    satiety_helper(sodium, 10000, 0.8) +
    satiety_helper(potassium, 950, 0.8) +
    satiety_helper(selenium, 600, 0.8) +
    satiety_helper(calcium, 5500, 1.2) +
    satiety_helper(phosphorus, 400, 1) +
    satiety_helper(copper, 8, 0.7) +
    satiety_helper(magnesium, 1800, 0.8) +
    satiety_helper(zinc, 95, 0.7) +
    satiety_helper(manganese, 12, 0.5) +
    satiety_helper(vB3, 80, 0.6) +
    satiety_helper(folate, 1800, 0.8) +
    satiety_helper(vE, 100, 0.7) +
    satiety_helper(vD, 9500, 0.6) +
    satiety_helper(vB1, 7.5, 0.5) +
    satiety_helper(vB6, 17, 0.5) +
    satiety_helper(vB12, 137, 0.5) +
    satiety_helper(vA, 57000, 0.5)
    ;
    
    satiety = satiety * 100 / cal;
    
    return satiety;*/

float foods::get_food_rank_by_sum(food *f,
                          std::vector<std::pair<int, float> > &good_nutrients,
                          std::vector<std::pair<int, float> > &bad_nutrients,
                           float *good_grams,
                           float *bad_grams)
{
    // Determine how many good grams in 100 grams
    *good_grams = 0;
    for (auto &n_i : good_nutrients)
    {
        int nutrient_id = n_i.first;
        float nutrient_quota = n_i.second;
        float nutrient_amount_per_100g = f->get_nutrient_amount_fast(nutrient_id);
        
        *good_grams += 100 * std::fmin(nutrient_amount_per_100g / nutrient_quota, 1.0);
    }

    // Determine how many bad grams in 100 grams
    *bad_grams = 0;
    for (auto &n_i : bad_nutrients)
    {
        int nutrient_id = n_i.first;
        float nutrient_quota = n_i.second;
        float nutrient_amount_per_100g = f->get_nutrient_amount_fast(nutrient_id);
        
        *bad_grams += 100 * nutrient_amount_per_100g / nutrient_quota;
    }
    
    return *bad_grams / *good_grams;
}

float foods::get_food_rank(food *f,
                          std::vector<std::pair<int, float> > &good_nutrients,
                          std::vector<std::pair<int, float> > &bad_nutrients,
                           float *good_grams,
                           float *bad_grams)
{
    // Determine the good grams - how much we have to eat to reach a quota for at
    //  least one good nutrient
    *good_grams = 1000000000;
    for (auto &n_i : good_nutrients)
    {
        int nutrient_id = n_i.first;
        float nutrient_quota = n_i.second;
        
        float nutrient_amount_per_100g = f->get_nutrient_amount_fast(nutrient_id);
        float grams = 100.0 * nutrient_quota / nutrient_amount_per_100g;
        
        /*if (f - &all_foods_[0] == 806099)
        {
            std::cerr << "foods::get_food_rank: nutrient_id=" << nutrient_id <<
             ", nutrient_quota=" << nutrient_quota << ", nutrient_amount_per_100g=" << nutrient_amount_per_100g <<
            ", grams" << grams << std::endl;
        }*/
        
        if (grams < *good_grams)
            *good_grams = grams;
    }

    // Determine the bad grams - how much we can to eat to stay within limits
    *bad_grams = 1000000000;
    for (auto &n_i : bad_nutrients)
    {
        int nutrient_id = n_i.first;
        float nutrient_quota = n_i.second;
        
        float nutrient_amount_per_100g = f->get_nutrient_amount_fast(nutrient_id);
        float grams = 100.0 * nutrient_quota / nutrient_amount_per_100g;
        if (grams < *bad_grams)
            *bad_grams = grams;
    }
    
    return *good_grams / *bad_grams;
}

float foods::get_food_rank(food* f,
                    std::vector<std::pair<int, float> > &nutrients,
                    float *good_grams,
                    float *bad_grams)
{
    std::vector<std::pair<int, float> > good_nutrients;
    std::vector<std::pair<int, float> > bad_nutrients;
    
    for (auto &n : nutrients)
    {
        if (n.second > 0)
            good_nutrients.push_back({abs(n.first), n.second});
        else
            bad_nutrients.push_back({abs(n.first), -n.second});
    }
    
    return get_food_rank(f, good_nutrients, bad_nutrients, good_grams, bad_grams);
}

void serialize(std::vector<std::pair<int, float> > &nutrients, std::string &result)
{
    for (auto &x : nutrients)
        result += std::to_string(x.first) + " " + std::to_string(x.second) + " ";
}

float sq_percent_diff(float a, float b)
{
    if (!a && !b)
        return 0;
    else
        return (a - b) * (a - b) / (std::fmax(a,b) * std::fmax(a,b));
}

float foods::distance_between_foods(int food_id_a,
                                        int food_id_b,
                                        recipes *rec,
                                        std::vector<int> *common_nutrients,
                                        std::vector<int> *divergent_nutrients)
{
	food *food_a = get_food(food_id_a);
	food *food_b = get_food(food_id_b);

    //if (!food_a || !food_b)
      //  std::cerr << "foods::distance_between_foods(" << food_id_a << ", " << food_id_b << ")" << std::endl;
    
    if (!food_a)
        food_a = rec->get_food(food_id_a);
    if (!food_b)
        food_b = rec->get_food(food_id_b);

    if (!food_a || !food_b)
        std::cerr << "foods::distance_between_foods2 (" << food_id_a << ", " << food_id_b << ")" << std::endl;

    // Don't count Protetin, Fat, Carbs, Fiber, Calories, Sodium, Total sugar
    std::array<int, 70> dont_count_list = {1003, 1004, 1005, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1050, 1062, 1063, 1072, 1075, 1078, 1081, 1082, 1084, 1085, 1086, 1079, 1093, 1113, 1116, 1117, 1119, 1120, 1121, 1122, 1123, 1159,  1160, 1161, 1211, 1212, 1213, 1214, 1215, 1216, 1217, 1218, 1219, 1220, 1221, 1222, 1223, 1224, 1225, 1226, 1227, 1228, 1232, 1233, 1234, 1235, 1253, 1257, 1258, 1292, 1293, 1298, 1329, 1330, 1331, 1403, 2000, 2028, 2029, 2033};
    
    float distance = 0;
    int n = 0;

    /*if (food_id_a == 806099 && food_id_b == 589915 ||
        food_id_b == 806099 && food_id_a == 589915)
    {
        std::cerr << "foods::distance_between_foods: food_id_a=" << food_id_a << ", food_id_b=" << food_id_b << std::endl;
    }*/
    
    for (int index_a = 0, index_b = 0; index_a < food_a->nutrients_.size() ||
         index_b < food_b->nutrients_.size(); )
    {
        int nutrient_id_a = (index_a == food_a->nutrients_.size()) ?
            1000000 : food_a->nutrients_[index_a].first;
        int nutrient_id_b = (index_b == food_b->nutrients_.size()) ?
            1000000 : food_b->nutrients_[index_b].first;
        
        /*if (food_id_a == 354098 && food_id_b == 364871 ||
            food_id_b == 354098 && food_id_a == 364871)
        {
            std::cerr << "foods::distance_between_foods: food_id_a=" << food_id_a << ", food_id_b=" << food_id_b <<
                ", nutrient_id_a=" << nutrient_id_a << ", nutrient_id_b=" << nutrient_id_b <<
                ", nutrient amount a=" << food_a->nutrients_[index_a].second <<
                ", nutrient amount b=" << food_b->nutrients_[index_b].second <<
                ", distance=" << distance << ", n=" << n << std::endl;
        }*/

        //std::cerr << "foods::distance_between_foods: nutrient_id_a=" << nutrient_id_a << ", nutrient_id_b=" << nutrient_id_b << std::endl;
        
        if (nutrient_id_a == nutrient_id_b)
        {
            bool not_both_zero = food_a->nutrients_[index_a].second || food_b->nutrients_[index_b].second;
            
            if (std::find(dont_count_list.begin(), dont_count_list.end(), nutrient_id_a) ==
                dont_count_list.end())
            {
                float d = sq_percent_diff(food_a->nutrients_[index_a].second, food_b->nutrients_[index_b].second);
                distance += d;

                // Count a distance only if nutrient values are not both zero
                if (not_both_zero)
                    ++n;
                
                // Save the common and divergent nutrient list
                if (not_both_zero)
                {
                    if (common_nutrients && d < 0.01)
                        common_nutrients->push_back(nutrient_id_a);
                    if (divergent_nutrients && d > 0.25)
                        divergent_nutrients->push_back(nutrient_id_a);
                }
            }

            //std::cerr << "index_a=" << index_a << ", index_b=" << index_b << ", distance=" << distance << std::endl;
            
            ++index_a;
            ++index_b;
        }
        else
        if (nutrient_id_a < nutrient_id_b)
        {
            if (std::find(dont_count_list.begin(), dont_count_list.end(), nutrient_id_a) ==
                dont_count_list.end())
            {
                if (divergent_nutrients)
                    divergent_nutrients->push_back(nutrient_id_a);
                distance += 1.0;
                ++n;
            }
            ++index_a;
        }
        else
        //if (nutrient_id_a > nutrient_id_b)
        {
            if (std::find(dont_count_list.begin(), dont_count_list.end(), nutrient_id_b) ==
                dont_count_list.end())
            {
                if (divergent_nutrients)
                    divergent_nutrients->push_back(nutrient_id_b);
                distance += 1.0;
                ++n;
            }
            ++index_b;
        }
    }

    // If n is zero then this means that the foods match for only zero non-stop-list nutrients
    // In this case we assume that the distance is maximum - i.e. 100% - i.e. 1.0
    if (!n)
        return 1.0;
    else
        return sqrt(distance / n);
}

bool foods::food_rank_json_filter_food(food *f,
                                int food_id,
                                std::vector<std::pair<int, float> > &good_nutrients,
                                std::vector<std::pair<int, float> > &bad_nutrients,
                                std::vector<std::string> &search_string_include,
                                std::vector<std::string> &search_string_exclude,
                                std::vector<std::string> &search_string_include_exact,
                                std::vector<std::string> &search_string_exclude_exact,
                                float min_food_rank,
                                float max_food_rank,
                                std::vector<std::pair<int, float> > &nutrient_lower_limits,
                                std::vector<std::pair<int, float> > &nutrient_upper_limits,
                                const char *popularity_group,
                                int catalogue_item_id,
                                bool sort_by_distance,
                                int show_distance_to,
                                float distance_limit,
                                recipes *rec,
                                       
                                float protein_to_calorie_percent,
                                float fiber_to_calorie_percent,
                                float min_pfind,
                                       
                                float min_satiety,
                                float max_satiety,

                                std::string &lower_food_name_temp_,
                                std::string &lower_food_category_temp_)
{
    // Check popularity groups
    if (popularity_group && *popularity_group)
    {
        const char *this_food_pop_group = get_food_aug_popularity_group(food_id);
        if (!this_food_pop_group || strcmp(this_food_pop_group, popularity_group))
            return false;
    }
    
    // Filter on catalogue
    if (catalogue_item_id != -1 && catalogue_item_id != f->food_catalogue_item_id_)
        return false;
    
    // Filter by name/category
    lower_food_name_temp_.clear();
    lower_food_category_temp_.clear();
    bool is_filtered = false;
    if (!search_string_include.empty())
    {
        is_filtered = false;
        lower_str(f->food_name_, lower_food_name_temp_);
        lower_str(f->food_category_, lower_food_category_temp_);
        
        // Search for presence of ALL words
        for (auto &x : search_string_include)
        {
            // The word is not absent - filter the food
            if (lower_food_name_temp_.find(x) == std::string::npos &&
                lower_food_category_temp_.find(x) == std::string::npos)
            {
                /*if (food_id > 1000000)
                    std::cerr << "filtered out: lower_food_name_temp_=" << lower_food_name_temp_ <<
                    ", x=" << x << std::endl;*/
                is_filtered = true;
                break;
            }
        }
        if (is_filtered)
            return false;
    } // if (!search_string_include.empty())

        
    // Search for exact match
    for (auto &x : search_string_include_exact)
    {
        // The word is not absent - filter the food
        if (f->food_name_.find(x) == std::string::npos &&
            f->food_category_.find(x) == std::string::npos)
        {
            is_filtered = true;
            break;
        }
    }
    if (is_filtered)
        return false;
        
    if (!search_string_exclude.empty())
    {
        if (lower_food_name_temp_.empty())
            lower_str(f->food_name_, lower_food_name_temp_);
        if (lower_food_category_temp_.empty())
            lower_str(f->food_category_, lower_food_category_temp_);
        
        // Search for absence of ALL words
        for (auto &x : search_string_exclude)
        {
            // The word is present - filter the food
            if (lower_food_name_temp_.find(x) != std::string::npos ||
                lower_food_category_temp_.find(x) != std::string::npos)
            {
                is_filtered = true;
                break;
            }
        }
        if (is_filtered)
            return false;
        
    } // if (!search_string_exclude.empty())

    // Search for exact mismatch
    for (auto &x : search_string_exclude_exact)
    {
        // The word is present - filter the food
        if (f->food_name_.find(x) != std::string::npos ||
            f->food_category_.find(x) != std::string::npos)
        {
                is_filtered = true;
                break;
        }
    }
    if (is_filtered)
        return false;

    // Skip zero-calorie products
    if (!f->get_nutrient_amount2(1008))
        return false;

    // Skip foods that are to far from the specified food
    float distance_to = -1;
    if (show_distance_to != -1)
    {
        distance_to = distance_between_foods(food_id, show_distance_to, rec, NULL, NULL);
        if (distance_limit != -1 && distance_to > distance_limit)
            return false;
    }
    
    float good_grams, bad_grams;
    float rank = get_food_rank(f, good_nutrients, bad_nutrients, &good_grams, &bad_grams);
    
    // If we sort by distance then automatically limit results with the rank of the show_distance_to food
    if (sort_by_distance && food_id == show_distance_to)
        max_food_rank = rank;

    // Skip everything below minumum rank - too good to be true :-)
    if (min_food_rank != -1 && rank < min_food_rank)
        return false;
    // Skip everything above maximum rank
    if (max_food_rank != -1 && rank > max_food_rank)
        return false;

    // Skip food with nutrients not within limits
    bool skip_by_nutrient_limits = false;
    for (auto &n_lower_limit : nutrient_lower_limits)
    {
        if (f->get_nutrient_amount_fast(n_lower_limit.first) < n_lower_limit.second)
        {
            skip_by_nutrient_limits = true;
            break;
        }
    }
    if (skip_by_nutrient_limits)
        return false;
    for (auto &n_upper_limit : nutrient_upper_limits)
    {
        if (f->get_nutrient_amount_fast(n_upper_limit.first) > n_upper_limit.second)
        {
            skip_by_nutrient_limits = true;
            break;
        }
    }
    if (skip_by_nutrient_limits)
        return false;
    
    // Skip foods not within satiety limits
    if (min_satiety != 0 || max_satiety != 0)
    {
        float satiety = get_food_satiety_per_calorie(f);
        if (satiety <= min_satiety || satiety > max_satiety)
            return false;
    }
    
    // Skip food not within ratio protein or fiber to calories
    bool prt_good =
        100.0 * 4.0 * f->get_nutrient_amount_fast(1003) / f->get_nutrient_amount_fast(1008) >= protein_to_calorie_percent;
    bool fib_good =
        100.0 * f->get_nutrient_amount_fast(1079) / f->get_nutrient_amount_fast(1008) >= fiber_to_calorie_percent;
    
    if (protein_to_calorie_percent && fiber_to_calorie_percent)
        return prt_good || fib_good;
    else
    if (protein_to_calorie_percent)
        return prt_good;
    else
    if (fiber_to_calorie_percent)
        return fib_good;
    else
    {
        if (!min_pfind)
            return true;
        else
            return (f->get_nutrient_amount_fast(1003) + f->get_nutrient_amount_fast(1079))
                * 12.0 / f->get_nutrient_amount_fast(1008) >= min_pfind;
    }
}
        
void foods::food_rank_json_output_food(food *f,
                                       int food_id,
                                       std::vector<std::pair<int, float> > &good_nutrients,
                                       std::vector<std::pair<int, float> > &bad_nutrients,
                                       int show_distance_to,
                                       recipes *rec,
                                       bool *b_first,
                                       std::ostream &output)
{
    if (!f || f->food_name_.empty())
        return;
    
    float prot = f->get_nutrient_amount2(1003);
    float fat = f->get_nutrient_amount2(1004);
    float carbs = f->get_nutrient_amount2(1005);
    float cal = f->get_nutrient_amount2(1008);
    float fiber = f->get_nutrient_amount2(1079);
    float sodium = f->get_nutrient_amount2(1093);
    
    std::vector<int> common_nutrients;
    std::vector<int> divergent_nutrients;

    // Skip foods that are to far from the specified food
    float distance_to = -1;
    if (show_distance_to != -1)
        distance_to = distance_between_foods(food_id, show_distance_to, rec, &common_nutrients, &divergent_nutrients);
    
    float good_grams, bad_grams;
    float rank = get_food_rank(f, good_nutrients, bad_nutrients, &good_grams, &bad_grams);
    
    if (!*b_first) output << "," << std::endl;
    *b_first = false;
    std::string js_escaped;
    json_escape(f->food_name_, js_escaped);
    float satiety = get_food_satiety_per_calorie(f);
    output << "{\"food\":\""
        << js_escaped << ", "
        //<< f->food_category_
        << "$$$Prt " << (int)prot
        << " Fat " << (int)fat
        << " Crb " << (int)carbs
        << " Fib " << (int)fiber
        << " Cal " << (int)cal
        << " Sod " << (int)sodium
        << " pfInd " << std::setprecision(3) << (prot + fiber) * 12.0 / cal
        << " sCal " << std::setprecision(3) << ((cal - (prot + fiber) * 12.0) / 100) * get_food_aug_standard_weight(food_id)
        << " satiety " << std::setprecision(3) << satiety
    << "\",\"food_id\":" << food_id << ",\"food_rank\":" << rank
    << ",\"good_grams\":" << good_grams
    << ",\"bad_grams\":" << bad_grams
    << ",\"weight\":" << get_food_aug_standard_weight(food_id)
    << ",\"satiety\":" << satiety
    << ", \"usda_cat\":\"" << f->food_category_ << "\""
    ;
        
    if (!get_food_aug_standard_unit(food_id).empty())
    {
        js_escaped.clear();
        json_escape(get_food_aug_standard_unit(food_id), js_escaped);
        output << ",\"standard_unit\":\"" << js_escaped << "\"";
    }
    if (get_food_aug_popularity_group(food_id))
        output << ",\"popularity_group\":\"" << get_food_aug_popularity_group(food_id) << "\"";
    if (get_food_aug_short_name(food_id))
        output << ",\"short_name\":\"" << *get_food_aug_short_name(food_id) << "\"";
    if (f->food_catalogue_item_id_ != -1)
    {
        output << ",\"food_catalogue_item_id\":" << f->food_catalogue_item_id_;
        food_catalogue_item *fci = get_food_catalogue_item(f->food_catalogue_item_id_);
        if (fci)
            output << ",\"food_catalogue_item_example_food_id\":" << fci->example_food_id_;
    }
    if (distance_to != -1)
        output << ",\"distance\":" << distance_to;
    
    // Output common nutrients with show_distance_to food
    if (!common_nutrients.empty())
    {
        output << ",\"common_nutrients\":\"";
        for (int i = 0; i < common_nutrients.size(); ++i)
        {
            if (i) output << " ";
            output << get_short_nutrient_name(common_nutrients[i]);
        }
        output << "\"";
    }

    // Output divergent nutrients with show_distance_to food
    if (!divergent_nutrients.empty())
    {
        output << ",\"divergent_nutrients\":\"";
        for (int i = 0; i < divergent_nutrients.size(); ++i)
        {
            if (i) output << " ";
            output << get_short_nutrient_name(divergent_nutrients[i]);
        }
        output << "\"";
    }
    
    output << "}";
}

void foods::food_rank_json(std::vector<std::pair<int, float> > &good_nutrients,
                           std::vector<std::pair<int, float> > &bad_nutrients,
                           std::vector<std::string> &search_string_include,
                           std::vector<std::string> &search_string_exclude,
                           std::vector<std::string> &search_string_include_exact,
                           std::vector<std::string> &search_string_exclude_exact,
                           int limit_after_sort,
                           float min_food_rank,
                           float max_food_rank,
                           std::vector<std::pair<int, float> > &nutrient_lower_limits,
                           std::vector<std::pair<int, float> > &nutrient_upper_limits,
                           const char *popularity_group_first,
                           int catalogue_item_id,
                           std::vector<int> *only_return_food_ids,
                 	int show_distance_to,
                    float distance_limit,
                    recipes *rec,
                    float protein_to_calorie_percent,
                    float fiber_to_calorie_percent,
                    float min_pfind,
                    bool sort_by_satiety_per_cal,
                           
                    int min_satiety_food,
                    float satiety_delta,
                    
		          std::ostream &output)
{
    // A satiety filter
    float min_satiety = 0, max_satiety = 0;
    if (min_satiety_food)
    {
        food *min_satiety_food_p = get_food(min_satiety_food);
        if (min_satiety_food_p)
        {
            min_satiety = get_food_satiety_per_calorie(min_satiety_food_p);
            max_satiety = min_satiety + satiety_delta;
        }
    }
    
    // If show_distance_to is negative then sort by distance rather than by food rank and in this case
    // we DON'T show food with the rank bigger than that of show_distance_to food
    bool sort_by_distance = false;
    if (show_distance_to < 0 && show_distance_to != -1)
    {
        sort_by_distance = true;
        show_distance_to = -show_distance_to;
    }
    
    // Default sort - for calories and protein
    if (good_nutrients.empty() && bad_nutrients.empty())
    {
        good_nutrients.push_back({1003, get_nutrient_recommended_min_amout(1003)});
        bad_nutrients.push_back({1008, get_nutrient_recommended_max_amout(1008)});
    }
    
    // This string keeps unique ID of the sorting order
    std::string str;
    if (sort_by_satiety_per_cal)
        str = "sort_by_satiety_per_cal";
    else
    if (sort_by_distance)
        str = "show_distance_to:" + std::to_string(show_distance_to);
    else
    {
        serialize(good_nutrients, str);
        serialize(bad_nutrients, str);
    }
    
    // Lookup food_ids in the cache and add it there if they're not there
    std::vector<int> &food_ids = sorted_ids_cache_[str];
    if (food_ids.empty())
    {
        int food_id = 0;
        for (auto &x : all_foods_)
        {
            if (!x.food_name_.empty())
                food_ids.push_back(food_id);
            ++food_id;
        }
        
        // Sort by satiety per calorie
        if (sort_by_satiety_per_cal)
        {
            std::cerr << "sorting food b satiety per calorie" << std::endl;
            std::sort(food_ids.begin(), food_ids.end(), [this](int a, int b){
                /*return get_food_rank_by(&all_foods_[a], good_nutrients, bad_nutrients, &good_grams, &bad_grams) <
                    get_food_rank(&all_foods_[b], good_nutrients, bad_nutrients, &good_grams, &bad_grams);*/
                return get_food_satiety_per_calorie(&all_foods_[a]) >
                    get_food_satiety_per_calorie(&all_foods_[b]);
            });
        }
        else
        // Sort by distance to the show_distance_to food
        if (sort_by_distance)
        {
            // Prefilter before sort - to sort quicker
            std::cerr << "prefilter before sort: food by distance to " << show_distance_to << ", index='" << str << "'" << std::endl;
            food *f_show_distance_to = get_food(show_distance_to);
            if (!f_show_distance_to || f_show_distance_to->food_name_.empty())
            {
                f_show_distance_to = rec->get_food(show_distance_to);
                if (!f_show_distance_to)
                {
                    std::cerr << "prefilter before sort: show_distance_to=" << show_distance_to << " is invalid!" << std::endl;
                    return;
                }
            }
            float good_grams, bad_grams;
            float show_distance_to_rank = get_food_rank(f_show_distance_to, good_nutrients, bad_nutrients, &good_grams, &bad_grams);
            std::vector<int> new_food_ids;
            for (auto food_id : food_ids)
            {
                food *f = get_food(food_id);
                if (!f || f->food_name_.empty())
                {
                    f = rec->get_food(food_id);
                    if (!f)
                        continue;
                }
                
                float rank = get_food_rank(f, good_nutrients, bad_nutrients, &good_grams, &bad_grams);
                if (rank > show_distance_to_rank)
                    continue;
                float distance = distance_between_foods(food_id, show_distance_to, rec, NULL, NULL);
                if (distance > 0.3)
                    continue;
                new_food_ids.push_back(food_id);
            }
            
            // Limit by distance and rank
            std::cerr << "sorting food by distance to " << show_distance_to << ", index='" << str << "'" << std::endl;
            std::sort(new_food_ids.begin(), new_food_ids.end(), [this, show_distance_to, rec](int a, int b){
                    float good_grams;
                    float bad_grams;
                    return distance_between_foods(show_distance_to, a, rec, NULL, NULL) <
                            distance_between_foods(show_distance_to, b, rec, NULL, NULL);
                });
            
            food_ids = new_food_ids;
        }
        // Sort by the rood rank with respect to the specified good/bad nutrients
        else
        {
            std::cerr << "sorting food by rank by sum" << ", index='" << str << "'" << std::endl;
            std::sort(food_ids.begin(), food_ids.end(), [this, & good_nutrients, & bad_nutrients](int a, int b){
                float good_grams;
                float bad_grams;
                /*return get_food_rank_by(&all_foods_[a], good_nutrients, bad_nutrients, &good_grams, &bad_grams) <
                    get_food_rank(&all_foods_[b], good_nutrients, bad_nutrients, &good_grams, &bad_grams);*/
                return get_food_rank_by_sum(&all_foods_[a], good_nutrients, bad_nutrients, &good_grams, &bad_grams) <
                    get_food_rank_by_sum(&all_foods_[b], good_nutrients, bad_nutrients, &good_grams, &bad_grams);
            });
        }
        std::cerr << "sorting done" << std::endl;
    }
    
    //std::cerr << "food_rank_json: food_ids.size()=" << food_ids.size() << std::endl;

    // Lower search strings
    for (auto &x : search_string_include)
        lower_str(x, x);
    for (auto &x : search_string_exclude)
        lower_str(x, x);
    
    std::string lower_food_name_temp_;
    std::string lower_food_category_temp_;
    
    if (!only_return_food_ids)
        output << "{\"event\":\"food_by_rank\",\"data\":[" << std::endl;
    
    bool b_first = true;

    // Search foods in the following order:
    //  1.  Show the food with id == show_distance_to without any filters
    //  2.  Show food-recipes (they're unique for a user that's why there're not sorted)
    //  3.  Show other foods (does not matter if they're from popularity groups or not - same order)
    //  4.  If reach the limit of the number foods to show then stop
    
    // Show-distance-food
    if (show_distance_to != -1)
        food_rank_json_output_food(get_food(show_distance_to),
                                   show_distance_to,
                                    good_nutrients,
                                    bad_nutrients,
                                    show_distance_to,
                                    rec,
                                    &b_first,
                                    output);
    
    // Show min_satiety_food first
    // Note: this is for the client side to select this food back if no substitutions are good
    if (min_satiety_food != 0)
        food_rank_json_output_food(get_food(min_satiety_food),
                                   min_satiety_food,
                                    good_nutrients,
                                    bad_nutrients,
                                    show_distance_to,
                                    rec,
                                    &b_first,
                                    output);
    
    // Recipe-foods
    int n_foods_shown = 0;
    if (rec)
    {
        for (auto &recipe : rec->get_recipes_vector())
        {
            food *f = rec->get_food(recipe.recipe_food_id_);
            if (!f)
                continue;
            
            // Check filters
            if (!food_rank_json_filter_food(f,
                                            recipe.recipe_food_id_,
                                            good_nutrients,
                                            bad_nutrients,
                                            search_string_include,
                                            search_string_exclude,
                                            search_string_include_exact,
                                            search_string_exclude_exact,
                                            min_food_rank,
                                            max_food_rank,
                                            nutrient_lower_limits,
                                            nutrient_upper_limits,
                                            popularity_group_first,
                                            catalogue_item_id,
                                            sort_by_distance,
                                            show_distance_to,
                                            distance_limit,
                                            rec,
                                            
                                            protein_to_calorie_percent,
                                            fiber_to_calorie_percent,
                                            min_pfind,
                                            
                                            min_satiety,
                                            max_satiety,

                                            lower_food_name_temp_,
                                            lower_food_category_temp_))
                continue;
            
            // Output the food
            food_rank_json_output_food(f,
                                       recipe.recipe_food_id_,
                                       good_nutrients,
                                       bad_nutrients,
                                       show_distance_to,
                                       rec,
                                       &b_first,
                                       output);
            
            ++n_foods_shown;
        } // for (auto recipe_food_id : *rec->get_recipes_vector())
    } // if (rec)
    
    // Foods from any popularity group
    // Why first? Because it's easier for an end user to find something she/he familiar with
    if (popularity_group_first)
    {
        for (auto food_id : food_ids)
        {
            // It was already shown
            if (food_id == show_distance_to)
                continue;
            food *f = get_food(food_id);
            if (!f || f->food_name_.empty())
                continue;
        
            // Show only foods with a popularity group
            if (!get_food_aug_popularity_group(food_id))
                continue;
        
            // Check filters
            if (!food_rank_json_filter_food(f,
                                        food_id,
                                        good_nutrients,
                                        bad_nutrients,
                                        search_string_include,
                                        search_string_exclude,
                                        search_string_include_exact,
                                        search_string_exclude_exact,
                                        min_food_rank,
                                        max_food_rank,
                                        nutrient_lower_limits,
                                        nutrient_upper_limits,
                                        popularity_group_first,
                                        catalogue_item_id,
                                        sort_by_distance,
                                        show_distance_to,
                                        distance_limit,
                                        rec,
                                            
                                            protein_to_calorie_percent,
                                            fiber_to_calorie_percent,
                                            min_pfind,
                                            
                                        min_satiety,
                                        max_satiety,

                                        lower_food_name_temp_,
                                        lower_food_category_temp_))
                continue;
                
            // Note: returning only ids - is only for normal food, not for custom supplied recipes
            if (only_return_food_ids)
                only_return_food_ids->push_back(food_id);
            // Output the food
            else
                food_rank_json_output_food(f,
                                       food_id,
                                       good_nutrients,
                                       bad_nutrients,
                                       show_distance_to,
                                       rec,
                                       &b_first,
                                       output);

            ++n_foods_shown;
        
            if (n_foods_shown >= 1000 || (limit_after_sort != -1 && n_foods_shown >= limit_after_sort))
                break;
        } // for (auto food_id : food_ids)
    } // if (popularity_group_first)
    
    // Other foods
    for (auto food_id : food_ids)
    {
        // It was already shown
        if (food_id == show_distance_to)
            continue;
        food *f = get_food(food_id);
        if (!f || f->food_name_.empty())
            continue;
        
        // Skip foods with a popularity group because they have been already shown
        if (popularity_group_first && get_food_aug_popularity_group(food_id))
            continue;

        // Check filters
        if (!food_rank_json_filter_food(f,
                                        food_id,
                                        good_nutrients,
                                        bad_nutrients,
                                        search_string_include,
                                        search_string_exclude,
                                        search_string_include_exact,
                                        search_string_exclude_exact,
                                        min_food_rank,
                                        max_food_rank,
                                        nutrient_lower_limits,
                                        nutrient_upper_limits,
                                        popularity_group_first,
                                        catalogue_item_id,
                                        sort_by_distance,
                                        show_distance_to,
                                        distance_limit,
                                        rec,
                                        
                                        protein_to_calorie_percent,
                                        fiber_to_calorie_percent,
                                        min_pfind,
                                        
                                        min_satiety,
                                        max_satiety,

                                        lower_food_name_temp_,
                                        lower_food_category_temp_))
            continue;
                
        // Note: returning only ids - is only for normal food, not for custom supplied recipes
        if (only_return_food_ids)
            only_return_food_ids->push_back(food_id);
        // Output the food
        else
            food_rank_json_output_food(f,
                                       food_id,
                                       good_nutrients,
                                       bad_nutrients,
                                       show_distance_to,
                                       rec,
                                       &b_first,
                                       output);

        ++n_foods_shown;
        
        if (n_foods_shown >= 1000 || (limit_after_sort != -1 && n_foods_shown >= limit_after_sort))
            break;
    } // for (auto food_id : food_ids)
    
    if (!only_return_food_ids)
        output << "]}" << std::endl;
}

} // namespace balanced_diet
