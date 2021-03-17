/*
*       balanced_diet.cpp
*
*       (C) Denis Anikin 2020
*
*       Impl for the balanced diet
*
*/

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string.h>

#include "balanced_diet.h"
#include "balanced_diet_recipe.h"

namespace balanced_diet
{

struct ratios_to_compare
{
    float ratio_in_food_;
    float ratio_to_desire_;
    int nutrient_id_;
    
    bool operator<(const ratios_to_compare &a) const
    {
        return nutrient_id_ < a.nutrient_id_;
    }
    bool operator<(int a) const
    {
        return nutrient_id_ < a;
    }
};

bool operator<(int a, const ratios_to_compare &b)
{
    return a < b.nutrient_id_;
}

// Norm ratios to sum up in 100
void norm_ratio_helper(std::vector<ratios_to_compare> &ratios)
{
    float sum_in_food = 0, sum_to_desire = 0;;
    for (auto &r : ratios)
    {
        sum_in_food += r.ratio_in_food_;
        sum_to_desire += r.ratio_to_desire_;
    }
    for (auto &r : ratios)
    {
        r.ratio_in_food_ = r.ratio_in_food_ * 100.0 / sum_in_food;
        r.ratio_to_desire_ = r.ratio_to_desire_ * 100.0 / sum_to_desire;
    }
}

/*bool operator<(const std::pair<int,float> &a, const std::pair<int,float> &b)
{
    return a.first < b.first;
}
template std::pair<int,float>
bool operator<(int a, const std::pair<int,float> &b)
{
    return a < b.first;
}
template std::pair<int,float>
bool operator<(const std::pair<int,float> &a, int b)
{
    return a.first < b;
}*/

template<class InputIt1, class InputIt2,
         class OutputIt, class Compare>
OutputIt my_set_intersection(InputIt1 first1, InputIt1 last1,
                          InputIt2 first2, InputIt2 last2,
                          OutputIt d_first, Compare comp)
{
    while (first1 != last1 && first2 != last2) {
        if (comp(*first1, *first2)) {
            ++first1;
        } else {
            if (!comp(*first2, *first1)) {
                // If the nutrient is 0 in the food then skip it like it is absent
                if (first1->second)
                    *d_first++ = {
                        .ratio_in_food_ = first1->second,
                        .ratio_to_desire_ = first2->second,
                        .nutrient_id_ = first1->first
                    };
                ++first1;
            }
            ++first2;
        }
    }
    return d_first;
}

void lower_str(const std::string &str, std::string &result)
{
    result.resize(str.size());
    for (int i = 0; i < str.size(); ++i)
    {
        if (str[i] >= 'A' && str[i] <= 'Z')
            result[i] = str[i] - 'A' + 'a';
        else
            result[i] = str[i];
    }
}

#define BR << "<br/>" << std::endl

#define NJSON if (!is_json) output
#define JSON if (is_json) output

std::string &json_escape(const std::string &str, std::string &result)
{
    result.reserve(str.size());
    for (auto &x : str)
    {
        if (x == '"')
            result.push_back('\\');
        if (x == '\\')
            result.push_back('\\');
        result.push_back(x);
    }
    
    return result;
}

void foods::search_by_ratio(std::vector<std::pair<int, float> > &ratios,
                            std::vector<std::pair<int, float> > &filters,
                            std::vector<std::string> &search_string_include,
                            std::vector<std::string> &search_string_exclude,
                            int limit_match_coefficient,
                            std::ostream &output,
                            bool is_json,
                            int nutrient_to_sort,
                            int limit_after_sort)
{
    NJSON << R"(
    
    <style>
      
    div {
      font-family: helvetica;
    }
    </style>
    
    <div>
    
    )";
    
    // Lower search strings
    for (auto &x : search_string_include)
        lower_str(x, x);
    for (auto &x : search_string_exclude)
        lower_str(x, x);
    
    std::string lower_food_name_temp_;
    std::string lower_food_category_temp_;
    
    // Sort ratios and filters by first
    std::sort(ratios.begin(), ratios.end(), [](const std::pair<int, float> &a,
                                               const std::pair<int, float> &b){
        return a.first < b.first;
    });
    std::sort(filters.begin(), filters.end(), [](const std::pair<int, float> &a,
                                               const std::pair<int, float> &b){
        return a.first < b.first;
    });
    
    NJSON << "foods::search_by_ratio:" BR BR;
    for (auto &r : ratios)
    {
        if (r.first < 0 || r.first >= all_nutrients_.size())
        {
            NJSON << "BAD NUTRIENT ID IN ratios: " << r.first;
            return;
        }
        NJSON << all_nutrients_[r.first].nutrient_name_ << ": " << r.second BR;
    }
    NJSON BR;

    for (auto &flt : filters)
    {
        if (flt.first < 0 || flt.first >= all_nutrients_.size())
        {
            NJSON << "BAD NUTRIENT ID filters: " << flt.first;
            return;
        }
        NJSON << all_nutrients_[flt.first].nutrient_name_ <<
        (flt.second > 0 ? ">" : "<") <<
        std::fabs(flt.second) BR;
    }
    NJSON BR;
    
    if (!search_string_include.empty())
    {
        NJSON << "includes: ";
        for (auto &x : search_string_include)
            NJSON << x << " ";
        NJSON BR;
    }
    if (!search_string_exclude.empty())
    {
        NJSON << "excludes: ";
        for (auto &x : search_string_exclude)
            NJSON << x << " ";
        NJSON BR;
    }
    
    std::vector<ratios_to_compare> temp;
    
    std::vector<std::vector<std::pair<int, float> > > food_results;
    food_results.resize(ratios.size());
    
    // Iterate all foods
    int food_id = 0;
    
    // If there is no rations then output just ALL foods filtered by search words and
    //  nutrient filters
    if (ratios.empty())
    {
        food_results.resize(1);
        for (auto &f : all_foods_)
        {
            if (!f.food_name_.empty())
                food_results[0].push_back({food_id, 0});
            ++food_id;
        }
    }
    else
    // Otherwise sort foods by match to the specified ratio and then again filter them
    for (auto &f : all_foods_)
    {
        if (f.food_name_.empty())
        {
            ++food_id;
            continue;
        }
        
        // Take the slice of nutrients specified in ratios and norm it for 100
        temp.resize(ratios.size());
        temp.erase(my_set_intersection(f.nutrients_.begin(), f.nutrients_.end(),
                                    ratios.begin(), ratios.end(),
                                    temp.begin(),
                                    [](const std::pair<int, float> &a,
                                    const std::pair<int, float> &b){
            return a.first < b.first;
        }),temp.end());
        
        if (temp.empty())
        {
            ++food_id;
            continue;
        }
        
        /*std::cerr << "food_id=" << food_id << ", food_name='" << f.food_name_ << "', " <<
        " total nutrients: " << f.nutrients_.size() << " BEFORE NORM:" << std::endl;
        
        for (auto &t : temp)
        {
            std::cerr << "nutrient_id=" << t.nutrient_id_ << ", ratio_in_food_=" << t.ratio_in_food_
            << "%, ratio_to_desire_=" << t.ratio_to_desire_ << "% ";
        }
        std::cerr << "AFTER NORM:" << std::endl;*/
        
        norm_ratio_helper(temp);
        
        // Now "temp" contains nutrients that exists in both - this food and "ratios", plus
        //  it's normed for 100
        
        // See how different ratios overall in food vs desired
        float total_diff = 0;
        for (auto &t : temp)
        {
        //    std::cerr << "nutrient_id=" << t.nutrient_id_ << ", ratio_in_food_=" << t.ratio_in_food_
          //  << "%, ratio_to_desire_=" << t.ratio_to_desire_ << "% ";
            
            total_diff += std::fabs(t.ratio_in_food_ - t.ratio_to_desire_);
        }
        //std::cerr << " total_diff=" << total_diff << std::endl;
        
        // Put the results in array depending on number of nutrients that present in the food
        food_results[temp.size()-1].push_back({food_id, total_diff});
        
        ++food_id;
    }
    
    JSON << "{\"event\":\"search\",\"data\":[" << std::endl;
    
    bool b_first = true;

    for (int match = food_results.size() - 1; match >= 0; --match)
    {
        std::vector<std::pair<int, float> > &food_result = food_results[match];
        
        //std::cerr << "foods::search_by_ratio: sorting: match=" << match << std::endl;
        
        // If there is a nutrient to sort - then sort by it
        if (nutrient_to_sort != -1)
        {
            std::sort(food_result.begin(),
                      food_result.end(),
                      [this, nutrient_to_sort](const std::pair<int, float> &a,
                        const std::pair<int, float> &b) {
                //std::cerr << "a.second=" << a.second << ", b.second=" << b.second << std::endl;
                return get_food(a.first)->get_nutrient_amount2(nutrient_to_sort) <
                        get_food(b.first)->get_nutrient_amount2(nutrient_to_sort);
            });
        }
        else
        // Otherwise - sort it by diff to the desired ratio
        std::sort(food_result.begin(),
                  food_result.end(),
                  [](const std::pair<int, float> &a,
                    const std::pair<int, float> &b) {
            //std::cerr << "a.second=" << a.second << ", b.second=" << b.second << std::endl;
            return a.second < b.second;
        });
        
        NJSON BR << "foods::search_by_ratio: result for nutrients match: "
            << match+1 << "/" << food_results.size() BR BR;
        
        int k = 0;
        for (auto &p : food_result)
        {
            food &fd = all_foods_[p.first];
            auto &nutrients = fd.nutrients_;
            
            // Check filters on nutrients for this food
            bool is_filtered = false;
            for (auto &fil : filters)
            {
                float *amount = fd.get_nutrient_amount(fil.first);
                // Amount must be at least as big as in the filter
                if (fil.second > 0)
                    is_filtered = !amount || *amount < fil.second;
                // Amount must be no more than in the minus filter
                else
                    is_filtered = amount && *amount > -fil.second;
                if (is_filtered)
                    break;
            }
            if (is_filtered)
                continue;
            
            // Filter by name/category
            lower_food_name_temp_.clear();
            lower_food_category_temp_.clear();
            if (!search_string_include.empty())
            {
                is_filtered = false;
                lower_str(fd.food_name_, lower_food_name_temp_);
                lower_str(fd.food_category_, lower_food_category_temp_);
                
                // Search for presence of ALL words
                for (auto &x : search_string_include)
                {
                    // The word is not absent - filter the food
                    if (lower_food_name_temp_.find(x) == std::string::npos &&
                        lower_food_category_temp_.find(x) == std::string::npos)
                    {
                        is_filtered = true;
                        break;
                    }
                }
                if (is_filtered)
                    continue;
            }
            if (!search_string_exclude.empty())
            {
                if (lower_food_name_temp_.empty())
                    lower_str(fd.food_name_, lower_food_name_temp_);
                if (lower_food_category_temp_.empty())
                    lower_str(fd.food_category_, lower_food_category_temp_);
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
                    continue;
            }
            NJSON << "[<a href=\"/nutrition?food=" << p.first << "\" target=\"blank\">"
                << p.first << "</a>] -> <a href=\"https://www.google.com/search?q=" << all_foods_[p.first].food_name_
                << "\" target=\"blank\">(" << (int)p.second << ")</a> {"
                << all_foods_[p.first].food_category_ << "} \""
                << all_foods_[p.first].food_name_ << "\" ";
            
            // For JSON we print food + category + PFC + Energy
            if (!b_first) JSON << "," << std::endl;
            b_first = false;
            std::string js_escaped;
            json_escape(all_foods_[p.first].food_name_, js_escaped);
            JSON << "{\"food\":\""
                << js_escaped << ", "
                << all_foods_[p.first].food_category_
                << ", Prot " << (int)all_foods_[p.first].get_nutrient_amount2(1003)
                << " Fat " << (int)all_foods_[p.first].get_nutrient_amount2(1004)
                << " Carbs " << (int)all_foods_[p.first].get_nutrient_amount2(1005)
                << " Cal " << (int)all_foods_[p.first].get_nutrient_amount2(1008)
            << "\",\"food_id\":\"" << p.first << "\"}";
            
            // Print amounts of all present nutrients
            temp.resize(ratios.size());
            temp.erase(my_set_intersection(nutrients.begin(), nutrients.end(),
                                        ratios.begin(), ratios.end(),
                                        temp.begin(),
                                        [](const std::pair<int, float> &a,
                                        const std::pair<int, float> &b){
                return a.first < b.first;
            }),temp.end());
            for (auto &t : temp)
            {
                NJSON << all_nutrients_[t.nutrient_id_].nutrient_name_ << ": "
                    << (int)t.ratio_in_food_ << " ";
            }
            
            // Print amounts of all nutrients that were under filters except those
            //  printed above and don't print a filter per one nutrient more than once
            NJSON << ", filters: ";
            for (auto &fil : filters)
            {
                float *amount = fd.get_nutrient_amount(fil.first);
                if (amount)
                {
                    if (!std::binary_search(temp.begin(), temp.end(), fil.first))
                    {
                        NJSON << all_nutrients_[fil.first].nutrient_name_ << ": " << *amount << " ";
                    }
                }
            }
            
            NJSON BR;
            
            ++k;
            
            if (k >= 1000 || (limit_after_sort != -1 && k >= limit_after_sort)
                || p.second >= limit_match_coefficient)
                break;
        }
    } // for (int match = food_results.size() - 1; match >= 0; --match)
    
    JSON << std::endl << "]}";
    
    NJSON BR BR << "All nutrients:" BR BR;
    
    int nutrient_id = 0;
    for (auto &n : all_nutrients_)
    {
        if (!n.nutrient_name_.empty())
            NJSON << "[" << nutrient_id << "] " << n.nutrient_name_ BR;
        ++nutrient_id;
    }
    
    NJSON << R"(
    
    </div>
    
    )";
}

// Returns the TOTAL amount of a nutrient in a meal
float foods::nutrient_comsumption_helper(std::vector<std::pair<int, float> > &foods_to_balance,
                                            int nutrient_id,
                                         recipes *rec)
{
    float total = 0;
    for (auto &x : foods_to_balance)
    {
        int food_id = x.first;
        float grams = x.second;
        food *f = NULL;
        if (rec)
        {
            f = rec->get_food(food_id);
            if (!f)
                f = get_food(food_id);
        }
        else
            f = get_food(food_id);
        if (!f || f->food_name_.empty())
            continue;
        
        float *amount = f->get_nutrient_amount(nutrient_id);
        if (amount && *amount)
            total += grams * *amount / 100.0;
    }
    return total;
}

/*
bool foods::balance_render_result_helper(std::vector<std::pair<int, float> > &foods_to_balance,
                                        std::vector<std::pair<int, float> > &good_nutrients,
                                        std::vector<std::pair<int, float> > &bad_nutrients,
                                        std::vector<int> &problematic_good_nutrients,
                                        std::vector<int> &problematic_bad_nutrients,
                                        std::ostream &output,
                                        bool is_after_amendments,
                                         recipes *rec,
                                        bool is_silent,
                                         recipes *rec)
{
    
    bool is_problem = false;

    for (auto &x : bad_nutrients)
    {
        int nutrient_id = x.first;
        float maximum_amount = x.second;
        nutrient *n = get_nutrient(nutrient_id);
        if (!n || n->nutrient_name_.empty())
            continue;
        
        float real_nutrient_amount = nutrient_comsumption_helper(foods_to_balance, nutrient_id, rec);
        
        if (!is_silent)
            output << "<tr><td>[" << nutrient_id << "]</td><td> " << n->nutrient_name_
            << "</td><td> < </td><td>" << maximum_amount
            << "</td><td>You'll consume: " << (int)std::ceil(real_nutrient_amount);
        
        int percent_delta = (int)(100.0*std::ceil(real_nutrient_amount - maximum_amount)/maximum_amount);
        if (percent_delta > 0)
        {
            is_problem = true;
            if (!is_silent)
                output << "</td><td><span style=\"color: red;\"><b>("
                << percent_delta
                << " % more)</b></span></td></tr>";
            problematic_bad_nutrients.push_back(nutrient_id);
        }
        else
            if (!is_silent)
                output << "</td><td><span style=\"color: green;\"><b>GOOD</b></span></td></tr>";
    }


    for (auto &x : good_nutrients)
    {
        int nutrient_id = x.first;
        float minimum_amount = x.second;
        nutrient *n = get_nutrient(nutrient_id);
        if (!n || n->nutrient_name_.empty())
            continue;
        
        float real_nutrient_amount = nutrient_comsumption_helper(foods_to_balance, nutrient_id, rec);
        
        if (!is_silent)
            output << "<tr><td>[" << nutrient_id << "]</td><td> " << n->nutrient_name_
            << "</td><td> > </td><td>" << minimum_amount
            << "</td><td>You'll consume: " << (int)std::ceil(real_nutrient_amount);
        int percent_delta = (int)(100.0*std::ceil(minimum_amount - real_nutrient_amount)/minimum_amount);
        if (percent_delta > 0)
        {
            is_problem = true;
            if (!is_silent)
                output << "</td><td><span style=\"color: red;\"><b>("
                << percent_delta
                << " % less)</b></span></td></tr>";
            problematic_good_nutrients.push_back(nutrient_id);
        }
        else
            if (!is_silent)
                output << "</td><td><span style=\"color: green;\"><b>GOOD</b></span></td></tr>";
    }

    return is_problem;
}*/

void construct_more_less_string_helper(int id,
                                       bool is_more,
                                       float amount,
                                       std::string &str)
{
    str += "%22" + std::to_string(id) + "%22,%22" + (is_more?"%3E":"%3C") + "%22,%22" +
        std::to_string((int)std::ceil(amount)) + "%22";
}

void construct_balance_food_url_helper(std::vector<std::pair<int, float> > &foods_to_balance,
                                        std::string &url)
{
    bool b_first = true;
    
    if (!foods_to_balance.empty())
    {
        if (!url.empty() && url[url.size()-1] != '?')
            url += "&";
        url += "foods=";
    }

    for (auto &x : foods_to_balance)
    {
        // Note: exclude all foods with zero grams
        if (!(int)std::ceil(x.second))
            continue;
        
        if (!b_first) url += ",";
        b_first = false;
        url += "%22" + std::to_string(x.first) + "%22,%22" +
        std::to_string((int)std::ceil(x.second)) + "%22";
    }
}

std::string &construct_balance_nutrients_url_helper(std::vector<std::pair<int, float> > &good_nutrients,
                                            std::vector<std::pair<int, float> > &bad_nutrients,
                                            std::string &url)
{
    if (!good_nutrients.empty() || !bad_nutrients.empty())
    {
        if (!url.empty() && url[url.size()-1] != '?')
            url += "&";
        url += "nutrients=";
        bool b_first = true;
        for (auto &x : good_nutrients)
        {
            // Exclude nutrients with zero limits
            if (!(int)std::ceil(x.second))
                continue;
            
            if (!b_first) url += ",";
            b_first = false;
            construct_more_less_string_helper(x.first, true, x.second, url);
        }
        for (auto &x : bad_nutrients)
        {
            if (!(int)std::ceil(x.second))
                continue;
            
            if (!b_first) url += ",";
            b_first = false;
            construct_more_less_string_helper(x.first, false, x.second, url);
        }
    }
    
    return url;
}

void construct_balance_url_helper(std::vector<std::pair<int, float> > &foods_to_balance,
                                    std::vector<std::pair<int, float> > &good_nutrients,
                                    std::vector<std::pair<int, float> > &bad_nutrients,
                                    std::string &url)
{
    url = "/balance?";
    
    construct_balance_food_url_helper(foods_to_balance, url);
    construct_balance_nutrients_url_helper(good_nutrients, bad_nutrients, url);
}

void foods::construct_short_nutrient_info_helper_item(foods::food *f,
                                               float nutrient_amount,
                                               int nutrient_id,
                                               int color,
                                               std::string &result)
{
    const char *s_color = "black";
    if (color == 1)
        s_color = "green";
    else
    if (color == 2)
        s_color = "red";
    
    result = result + "<span style=\"color: " + s_color + ";\">"+/*(nutrient_id==1008?"Cal":get_nutrient(nutrient_id)->nutrient_name_.substr(0,3)) + " " +*/
        std::to_string((int)f->get_nutrient_amount2(nutrient_id)) + "/" +
        std::to_string((int)(nutrient_amount)) +
        "</span>";
}

const char *get_nutrient_td_style_to_show(int nutrient_id)
{
    bool output_this_nutrient = (nutrient_id == 1003 || nutrient_id == 1004 || nutrient_id == 1005 || nutrient_id == 1008 ||
                                 nutrient_id == 1093);
    return output_this_nutrient?"table-cell":"none";
}

void foods::construct_short_nutrient_info_helper(foods::food *f,
                                                int food_id,
                                              float food_amount,
                                              std::vector<std::pair<int, float> > &good_nutrients,
                                              std::vector<std::pair<int, float> > &bad_nutrients,
                                              std::string &result)
{
    for (auto &n_i : good_nutrients)
    {
        int nutrient_id = n_i.first;
        result = result + "<td style='display: " + get_nutrient_td_style_to_show(nutrient_id) + "' id=\"food_nutrient_" + std::to_string(food_id) + "_" +
        std::to_string(n_i.first) + "\">";
        construct_short_nutrient_info_helper_item(f,
                                                  f->get_nutrient_amount2(n_i.first) * food_amount / 100.0, n_i.first, 0, result);
        result += "</td>";
    }
    for (auto &n_i : bad_nutrients)
    {
        int nutrient_id = n_i.first;
        result = result + "<td style='display: " + get_nutrient_td_style_to_show(nutrient_id) + "' id=\"food_nutrient_" + std::to_string(food_id) + "_" +
        std::to_string(n_i.first) + "\">";
        construct_short_nutrient_info_helper_item(f,
                                                  f->get_nutrient_amount2(n_i.first) * food_amount / 100.0, n_i.first, 0, result);
        result += "</td>";
    }
}

int range_value_helper(float v, float ref, int range)
{
    if (v <= ref)
        return range * ((v - ref) / ref + 1) / 2;
    else
        return range * v / (v + ref);
}

#define FIRST_COMMA if (!is_first) output << ","; is_first = false;


const char *meal_group_names[] = {
    "Snack before breakfast",
    "Breakfast",
    "Snack after breakfast",
    "Brunch",
    "Snack after brunch",
    "Lunch",
    "Snack after lunch",
    "Second snack after lunch",
    "Dinner",
    "Snack after dinner",
    "Supper",
    "Late night snack",
    "Just snack"
};

void foods::balance(std::vector<std::pair<int, float> > &foods_to_balance,
             std::vector<std::pair<int, float> > &good_nutrients,
             std::vector<std::pair<int, float> > &bad_nutrients,
            std::vector<std::pair<std::string, std::vector<int> > > &groups___,
            std::vector<int> &mr_data,
            const std::string &current_meal_plan_name,
            recipes *rec,
            int target_weight,
            int target_weight_deadline,
            bool is_imperial,
            bool add_healthy_food,
                    const std::vector<std::string> &healthy_search_words,
             std::ostream &output)
{
    // Nutrient limits by default
    if (good_nutrients.empty() && bad_nutrients.empty()/* && foods_to_balance.empty()*/)
    {
        int default_good_nutrients[] = {
            1003, // Protein
            1079, // Fiber
            1087, // Calcium
            1089, // Iron
            1090, // Magnesium
            1091, // Phosphorus
            1092, // Potassium
            1095, //
            1096, //
            1098, //
            1100, //
            1101, //
            1102, //
            1103, //
            1104, //
            1110, //
            1124,
            1162,
            1165,
            1166,
            1167,
            1170,
            1175,
            1176,
            1177,
            1178,
            1180, /*1183, 1184, */
            1185,
            1272,
            1278,
            1316,
            1404
        };
        int default_bad_nutrients[] = {1004, 1005, 1008, 1093, 1253, 1257};
        
        for (int nutrient_id : default_good_nutrients)
            good_nutrients.push_back({nutrient_id, get_nutrient_recommended_min_amout(nutrient_id)});
        for (int nutrient_id : default_bad_nutrients)
            bad_nutrients.push_back({nutrient_id, get_nutrient_recommended_max_amout(nutrient_id)});
    }
    
    // Add healthy food to the balanced food
    if (add_healthy_food)
        auto_balance(foods_to_balance, good_nutrients, bad_nutrients, rec, 4.0, 4.0, healthy_search_words);
    
    // Sort groups accordingly to the time line - to show them well
    std::vector<std::pair<std::string, std::vector<int> > > groups;
    for (int i = 0; i < sizeof(meal_group_names)/sizeof(meal_group_names[0]); ++i)
    {
        for (auto &x : groups___)
        {
            if (x.first == meal_group_names[i])
            {
                groups.push_back(x);
                break;
            }
        }
    }
    
    std::vector<std::pair<int, float> > foods_to_balance_orig = foods_to_balance;
    
    output << std::ifstream("./bf_header.html").rdbuf() << R"x(
    
     var target_weight = ")x" << target_weight << R"x(";
     var target_weight_deadline = ")x" << target_weight_deadline << R"x(";
     var current_meal_plan_name = ")x" << current_meal_plan_name << R"x(";
     var food_referenece_values = new Map([)x";
       
    // Fill JS maps of current and reference values
    bool is_first = true;
    for (auto &x : foods_to_balance)
    {
        FIRST_COMMA
        output << "[" << x.first << "," << x.second << "]";
    }
    
    // Note: meaning is signes in nutrient_reference_values:
    //  + means that this is a good nutrient
    //  - means that this is a bad nutrient
    output << R"x(]);
     var food_current_values = new Map(food_referenece_values);
     var nutrient_reference_values = new Map([)x";
    {
         bool is_first = true;
         for (auto &x : good_nutrients)
         {
             FIRST_COMMA
             output << "[" << x.first << "," << x.second << "]";
         }
         for (auto &x : bad_nutrients)
         {
             FIRST_COMMA
             output << "[" << x.first << "," << -x.second << "]";
         }
    }
    output << R"x(]);
     var nutrient_current_values = new Map([)x";
    {
        bool is_first = true;
        for (auto &x : good_nutrients)
        {
            FIRST_COMMA
            output << "[" << x.first << "," << (int)nutrient_comsumption_helper(foods_to_balance, x.first, rec) << "]";
        }
        for (auto &x : bad_nutrients)
        {
            FIRST_COMMA
            output << "[" << x.first << "," << (int)nutrient_comsumption_helper(foods_to_balance, x.first, rec) << "]";
        }
     }
    
    // Save meal groups in the map for JS
    const char *current_meal_group_name = NULL;
    output << R"x(]);
    var meal_groups = new Map([)x";
    {
        bool is_first = true;
        for (auto &mg : groups)
        {
            current_meal_group_name = mg.first.c_str();
            FIRST_COMMA
            output << "[\"" << mg.first << "\",[";
            {
                bool is_first = true;
                for (auto &f : mg.second)
                {
                    FIRST_COMMA
                    output << f;
                }
            }
            output << "]]";
        }
    }

    // Fill JS arrray with mr_data
    output << R"x(]);
    var mr_data = [)x";
    {
        bool is_first = true;
        for (auto &mr_e : mr_data)
        {
            FIRST_COMMA
            output << mr_e;
        }
    }

    // Output nutrient names - for recipes
    output << R"x(];
     var nutrient_header_names=[)x";
    {
         bool is_first = true;
         for (auto &x : good_nutrients)
         {
             FIRST_COMMA
             output << "\"" << get_short_nutrient_name(x.first) << "\"";
         }
         for (auto &x : bad_nutrients)
         {
             FIRST_COMMA
             output << "\"" << get_short_nutrient_name(x.first) << "\"";
         }
    }
    
    // Save recpies from the URL to the localstorage if they're not already there (this is when the URL
    //  is sending over messangers one perseon to another)
    // Output nutrient names - for recipes
    output << R"x(];
     var recipes_from_url = [
    )x";
    {
         bool is_first = true;
         for (auto &recipe : rec->get_recipes_vector())
         {
             FIRST_COMMA
             output << std::endl << "[" << recipe.recipe_food_id_ << ",'" << recipe.recipe_name_ << "'";
             for (auto &f : recipe.recipe_foods_)
                 output << "," << f.first << "," << f.second;
             output << "]";
         }
    }
    output << R"x(
    ];
    
    // Serching recipes from URL in local storage
    let all_recipes = JSON.parse(window.localStorage.getItem('BF_recipes'));
    for (let recipe_from_url of recipes_from_url)
    {
        let found = false;
        if (all_recipes)
            for (let x of all_recipes)
                if (x[1] === recipe_from_url[1])
                {
                    found = true;
                    break;
                }
        if (!found)
        {
            if (all_recipes === null || all_recipes === undefined)
                all_recipes = new Array();
            all_recipes.push(recipe_from_url);
        }
    }
    window.localStorage.setItem('BF_recipes', JSON.stringify(all_recipes));
    
    )x";
    
    // Save recipe and apply changes buttons
    output << R"x(
    
    </script>
    
    <link rel="stylesheet" href="./static?file=bf.css">
    </head>
    <body onLoad="on_load()">

    <div>
    
    <div id="curve_chart_floating" style="position: fixed; right: 10px; top: 10px; width: 700px; height: 500px; visibility: hidden;"></div>
    
    <center><h2><a class="header-a" href="/">Balanced Foods</a></h2></center>
    <p></p>

    <p>
    1. Are you in a restaurant or in a grocery store right now? Then this is a service for you.<br>
    2. Enter the name of a food (like pasta, pizza, burger, big mac etc) in the box below.<br>
    3. You'll see a list of foods matching the name.<br>
    4. Green foods are food to eat and red foods are foods to avoid.<br>
    5. Eat more green ones and lose weight.<br>
6. Still want to eat a red food! No problem. Pick any food from the list (red or green).<br>
7. Press the round button on the right of the screen in the same line with the food.<br>
8. Go to the box below "Add food" and type any name for the potential substitution (pizza, donut etc).<br>
9. You'll see a list of foods to substitute this food. All substitutions are healthier and more filling than the initially selected food. Which will result in weight loss :-)<br>
    10. Enjoy your meals and lose weight with balanced-foods.com :-)
    
    </p>
 
    <table>)x";
    
    // If there is no food then don't show headers
    if (!foods_to_balance.empty())
    {
        output << R"x(
    
    <tr><td colspan="2"><b>Daily meal plan</b></td>
    <td colspan="1" id="locally_saved_meal_plans"></td>
    <td>
    <input type="submit" id="save_current_meal_plan_button" onclick="save_current_meal_plan()" value="Save">
    <input type="submit" onclick="remove_current_meal_plan()" value="Remove">
    </td>
    <td colspan="3"><!--<input type="text" id="this_meal_plan_name" placeholder="Name this plan" style="width: 100%; height: 35px;"/>--></td>
    </tr>
    <tr><td colspan="15" style="border-top: 1px solid #ddd; height: 15px;"></td></tr>
    
    <tr>
    <!--<td colspan="2"><input type="text" id="new_recipe" placeholder="Recipe name" style="width: 100%; height: 20px;"/></td>-->
    
    <td><select onchange="metric_imperial_change()" id="oz_grams" style="background-color: #f1f1f1; font-size: 14px; height: 25px; border: 1px solid #d4d4d4; border-bottom: none; border-top: none; z-index: 99;">
            <option value="oz">oz</option>
            <option value="grams" selected>grams</option>
        </select></td>
    
       <td><!--<button onclick="save_recipe(document.getElementById('new_recipe').value);">Save</button>--></td>
        <td><input style="font-size: 12px;" title='Click here to reload the page with all the changes you made moving sliders for foods and nutrients' type="submit" onclick="apply_changes_from_sliders()" value="Apply changes">
        <input style="font-size: 12px;" title='Click here to rebalance your meal in order to reach your nutrient limits. This will change amounts of all foods' type="submit" onclick="auto_rebalance()" value="Rebalance meal">
        <input style="font-size: 12px;" title='Click here to add foods to your diet to make it healthier in terms of good nutrients' type="submit" onclick="add_healthy_food()" value="Add healthy food">
        </td>
        
        <td>
        <input style="font-size: 12px;" onclick="document.getElementById('tutorial3').style.display='block';" type="submit" value="Help - left section">
        <input style="font-size: 12px;" onclick="document.getElementById('tutorial5').style.display='block';" type="submit" value="Help - right section">
        </td>
    
    )x";
    
    // Output headers for food/nutrient fields
    // By default show only protein, fat, carbs, calories, sodium
    // Also add "Best" header for the best nutrient in this food in terms of percent of daily norm

    output << "<td><span title='Satiety calories for this food. Positive if this food gives you hunger and negative if this food gives you satiety'>sCal</span></td>";
    output << "<td><span title='The best nutrient in this food in terms of daily norm with its daily norm per 100g and its daily norm per food amount'>Best %</span></td>";

        
    for (auto &ntr : good_nutrients)
    {
        int nutrient_id = ntr.first;
        output << "<td style='display: " << get_nutrient_td_style_to_show(nutrient_id) << "'><span title='Amount of " << get_nutrient(ntr.first)->nutrient_name_ << " in this food per 100g/per your meal'>" << get_short_nutrient_name(nutrient_id) << "</span></td>";
    }
    for (auto &ntr : bad_nutrients)
    {
        int nutrient_id = ntr.first;
        output << "<td style='display: " << get_nutrient_td_style_to_show(nutrient_id) << "'><span title='" << get_nutrient(ntr.first)->nutrient_name_ << " in this food per 100g/per your meal'>" << get_short_nutrient_name(nutrient_id) << "</span></td>";
    }
    output << "<td><span title='How much weight you will gain in 3 months of daily consumption of this food in this amount'>In 3 mon</span></td><td><span title='Click the number below to see the USDA nutrition information about this food'>USDA info</span></td><td><span title='Click a round button below and then go to the New food box to substitute the food'>Subst</span></td>";
    
    output << R"x(
    
       </tr>
       <tr><td colspan="20" style="height: 10px;"></td></tr>
       
    
    )x" << std::endl;
    
    // Temp map of all grouped food - to prevent from showing it in the ungroupped section
    std::unordered_set<int> grouped_food_temp;
    
    // Iterate all meal groups
    for (int i = 0; i <= groups.size(); ++i)
    {
        // If this group if empty then skip it
        if (i != groups.size() && groups[i].second.empty() ||
            i == groups.size() && grouped_food_temp.size() == foods_to_balance.size())
            continue;
        
        // Output the meal group aggregated info
        const std::string &meal_group_name = (i == groups.size()) ? "Other meals" : groups[i].first;
        output << R"x(
        
        <tr><td colspan="3" style="border-bottom: 1px solid #ddd; height: 15px;">)x" << meal_group_name << R"x(</td>
        
        
        )x" << std::endl;

        // Output the slider for the meal group
        output << R"x(<td><div class="range-wrap">
        <input class="range" type="range" min="1" max="10000" value="5000" id="myRange_meal_group_)x"
        << meal_group_name << R"x(" list="rangedatalist_meal_group_)x"
        << meal_group_name << R"x(" />
        <output class="bubble" id="bubble_meal_group_name_)x"
        << meal_group_name << R"x("></output>
        </div>
        <datalist id="rangedatalist_meal_group_name_)x"
        << meal_group_name << R"x("><option value="5000">)x" << 0 << R"x(</option></datalist></td>
        )x" << std::endl;
        
        // Add a short info per good/bad nutrients for this meal group
        // Note: this td is empty and will be filled in the JS
        // Also add the best nutrient td
        output << "<td id=\"meal_group_scal_" << meal_group_name << "\"></td>"; // sCal
        output << "<td id=\"meal_group_nutrient_" << meal_group_name << "_" << "best" << "\"></td>";
        for (auto &n_i : good_nutrients)
        {
            int nutrient_id = n_i.first;
            output << "<td style='display: " << get_nutrient_td_style_to_show(nutrient_id) << "' id=\"meal_group_nutrient_" << meal_group_name << "_" << nutrient_id << "\"></td>";
        }
        for (auto &n_i : bad_nutrients)
        {
            int nutrient_id = n_i.first;
            output << "<td style='display: " << get_nutrient_td_style_to_show(nutrient_id) << "' id=\"meal_group_nutrient_" << meal_group_name << "_" << nutrient_id << "\"></td>";
        }

        output << R"x(</tr>
        )x" << std::endl;
        

        // Output all foods (starting from groups and then from ungrouped other meals)
        for (int x = 0; x < (i == groups.size() ? foods_to_balance.size() : groups[i].second.size()); ++x)
        {
            int food_id = (i == groups.size() ? foods_to_balance[x].first : groups[i].second[x]);
            
            // Save the food id to prevent it from showing in the ungrouped section
            if (i != groups.size())
                grouped_food_temp.insert(food_id);
            else
                if (grouped_food_temp.find(food_id) != grouped_food_temp.end())
                    continue;
            
            // Get food struct pointer + fill recipes param for the USDA info link
            float grams = 0;//x.second;
            food *f = NULL;
            std::string recipes_param;
            if (rec)
            {
                f = rec->get_food(food_id);
                if (!f)
                    f = get_food(food_id);
                else
                {
                    auto &rec_vec = rec->get_recipes_vector();
                    for (auto &recipe : rec_vec)
                        if (recipe.recipe_food_id_ == food_id)
                        {
                            recipes_param = "&recipes=%22" + std::to_string(food_id) +
                                "%22,%22" + f->food_name_ +
                                "%22,%22" + std::to_string(recipe.recipe_foods_.size()) + "%22";
                            
                            for (auto &x : recipe.recipe_foods_)
                                recipes_param += ",%22" + std::to_string(x.first) + "%22,%22" +
                                std::to_string(x.second) + "%22";
                            break;
                        }
                }
            }
            else
                f = get_food(food_id);
            
            if (!f || f->food_name_.empty())
                continue;

            float good_grams, bad_grams;
            float food_rank = get_food_rank(f, good_nutrients, bad_nutrients, &good_grams, &bad_grams);

            // Output the weight and the name
            output << "<tr><td>"
            << R"(
                    <input id="food)" << food_id <<
                    R"(" style="width: 50px; height: 20px;" type="text" value=")"
            << (int)grams << "\"><input title='Click here to exclude this food from the amount changing when moving sliders for nutrients below' type='checkbox' id='checked_food_" << food_id << "'></td>"
            << R"x(<td><img src="/food_pics?file=)x" << food_id << R"x(.jpeg" width="24" height="24" onmouseover="bigImg(this)" onmouseout="normalImg(this)" onerror="this.style.display='none'"/></td>)x"
            << "<td><a href=\"https://www.google.com/search?q=" << f->food_name_
            << "\" target=\"blank\">" << f->food_name_.substr(0,45) << "</a></td>";
        
            // Output the slider
            output << R"x(<td><span title='Move this slider to change amount of this food'><div class="range-wrap">
            <input class="range" type="range" min="1" max="10000" value="5000" id="myRange_food_)x"
            << food_id << R"x(" list="rangedatalist_food_)x"
            << food_id << R"x(" />
            <output class="bubble" id="bubble_food_)x"
            << food_id << R"x("></output>
            </div>
            </span>
            <datalist id="rangedatalist_food_)x"
            << food_id << R"x("><option value="5000">)x" << (int)grams << R"x(</option></datalist></td>
            )x" << std::endl;
            
            // Output sCal
            float prot = f->get_nutrient_amount2(1003);
            float cal = f->get_nutrient_amount2(1008);
            float fiber = f->get_nutrient_amount2(1079);
            
            int sCal = int(
                                    (
                                        (cal - (prot + fiber) * 12.0)
                                     
                                        / 100
                                     )
                                        * get_food_aug_standard_weight(food_id)
                           );
            
            output << "<td id='scal_" << food_id << "' style='color: "<< (sCal>=0?"red":"green") << "'>" << sCal << "</td>";
            
            // Add a short info per the best nutrient
            output << "<td>" /*<< get_short_nutrient_name(f->nutrients_sorted_daily_norm_[0].first)*/ << "<span title='" << get_nutrient_middle_name(f->nutrients_sorted_daily_norm_[0].first) << "' id='best_" << food_id << "'>" << //f->nutrients_sorted_daily_norm_[0].first << " " <<
                (int)(f->nutrients_sorted_daily_norm_[0].second*100.0) <<
                "</span></td>";
            // Add a short info per good/bad nutrients for this food
            std::string nutrients_info;
            construct_short_nutrient_info_helper(f, food_id, grams,
                                                 good_nutrients,
                                                 bad_nutrients,
                                                 nutrients_info);
            output << nutrients_info;
        
            // Kg in 3 months
            output << R"x(<td id="kg_3_month_)x" << (int)food_id << R"x("></td>)x" << std::endl;
            
            // food_id plus a link to USDA info
            output << R"(<td><a title='The USDA id of this food. Click here for the USDA nutrition information' href="/nutrition?food=)" << food_id << recipes_param << R"(" target="blank">)"
        << (recipes_param.empty()?std::to_string(food_id):"Recipe") << "</a></td>" << std::endl;
        
            // If the rank is less than 1.0 then this food is better to eat otherwise it's
            //  better to avoid in order to balance your nutrition

		// Output the substitution radio button anyway. Also this radio button works as a search for foods close in satiety rather than in micro nutrient profile
            /*if (food_rank < 1.0)
                output << R"(<td><img title='This food is considered healthy under nutrient limits that you specified' src="/food_pics?file=green_leaf.jpeg" width="24" height="24"/></td>)";
            else*/
                output << R"(<td><input title='Click here and then go to the New food box below to find a healthier substituion for this food to balance your nutrition' type="radio" id="radio_food_)" << food_id << R"(" name="distance_to" value="" onclick="show_distance_to=)" << food_id << R"("></td>)";
            output << "</tr>" << std::endl;

            // This is to catch press enter on food amounts
            output << R"(
        <script>
        {
            let input = document.getElementById("food)" + std::to_string(food_id) +
            R"(");
            input.addEventListener("keyup", function(event) {
              if (event.keyCode === 13) {
                 let amount = Number(input.value);
                 if (amount === Number.NaN)
                    amount = 0;
                let food_id = )" << food_id << R"(;
                console.log(`Change food amount: food_id=${food_id}, amount=${amount}`);
                if (amount === 0)
                {
                    food_current_values.delete(food_id);
                    let i = 0;
                    let mg = meal_groups.get(")" << meal_group_name << R"(");
                    for (let x of mg)
                    {
                        if (x == food_id)
                        {
                            mg.splice(i, 1);
                            break;
                        }
                        ++i;
                    }
                            
                    apply_changes_from_sliders();
                }
                else
                {
                    if (is_imperial) amount *= 28.3495;
                    food_current_values.set(food_id, amount);
                    apply_changes_from_sliders(food_id, amount);
                }
              }
            });
        }
        </script>
        )";
        } // for (auto &x : foods_to_balance)
        
        // Divide meal groups
        output << R"x(<tr><td colspan="20" style="height: 15px;"></td></tr>)x" << std::endl;
    
    } // for (auto &fg : groups)
        
    } // if (!foods_to_balance.empty())

    // Add new food
    // New food in URL is a food with ID=999999999 and amount 100 gr
    // JS will substitute it with a real id
    // Note: ^^^ is a placeholder for a weight of a newly added food
    std::string new_food_url = "/balance?";
    construct_balance_nutrients_url_helper(good_nutrients, bad_nutrients, new_food_url);
    construct_balance_food_url_helper(foods_to_balance, new_food_url);
    bool need_comma = foods_to_balance.size() >= 1;
    if (new_food_url[new_food_url.size()-1] == '?')
        new_food_url += "foods=";
    else
    if (!need_comma)
        new_food_url += "&foods=";
    
    std::string suggest_url;
    output << R"(
    <tr><td colspan="4" style="height: 15px;"></td></tr>
    <tr><td colspan="15" style="border-top: 1px solid #ddd; height: 15px;"><b>Add food</b></td></tr>
    <tr><td colspan="4" style="height: 15px;"></td></tr>
    <tr><td colspan="4">
    <div class="autocomplete" style="width:700px;">
      <input id="myInputNewFood" type="text" placeholder="New food" autocomplete="off">
    </div>
    <script>
        autocomplete("/food_by_rank?limitsort=50&)" <<
        construct_balance_nutrients_url_helper(good_nutrients, bad_nutrients, suggest_url)
        << R"(", 999999999, ")" << new_food_url << (need_comma?",":"") <<
        R"(%22999999999%22,%22^^^%22", document.getElementById("myInputNewFood"), arr);
    </script>
    <input onclick="document.getElementById('tutorial2').style.display='block';" type="submit" value="Help?">
    </td>
    )";

    // Meal group select box + suggest from the catalogue
    output << R"(
    <td colspan="4">
    
    
    
    <select style="background-color: #f1f1f1; font-size: 16px; height: 40px; border: 1px solid #d4d4d4; border-bottom: none; border-top: none; z-index: 99;" id="myInputMealGroup" autocomplete="off">)";
    
    // By default show only 4 basic meal groups
    if (foods_to_balance.empty())
    {
        output << "<option selected>Breakfast</option><option>Lunch</option><option>Dinner</option><option>Just snack</option>";
    }
    else
    for (int i = 0; i < sizeof(meal_group_names)/sizeof(meal_group_names[0]); ++i)
    output << R"(
        <option )" << (!current_meal_group_name || strcmp(meal_group_names[i], current_meal_group_name)?"":"selected")
            << R"( value=")" << meal_group_names[i] << R"(">)" << meal_group_names[i] << R"(</option>)";
    output << R"(<option value="to_the_recipe">To the recipe below</option>)";
    
    
    output << R"(
    </select>
    </td>
    
    <td colspan="4">
    <div class="autocomplete" style="width:300px;">
      <input id="myInputCatalogue" type="text" placeholder="any category" autocomplete="off">
    </div>
    <script>
        autocomplete("/catalogue", 111111111, "", document.getElementById("myInputCatalogue"), arr);
    </script>
    </td></tr>
    )";
    
    output << R"(
    <tr>
    <td colspan="4">
    <select style="background-color: #f1f1f1; font-size: 16px; height: 40px; border: 1px solid #d4d4d4; border-bottom: none; border-top: none; z-index: 99;" id="pre_filters" autocomplete="off">
        <option  selected value="">Choose a healthy filter</option>
        <option  value='"1003",">","10","1079",">","5","2000",">","10","1004",">","5"'>High protein, high fiber sweets</option>
        <option  value='"1005",">","20","2000",">","15","1003","<","10","1004",">","7","1008","<","200"'>Low calorie sweets, bakery, ice cream</option>
        <option  value='"1003",">","7","1005",">","30","2000",">","5","1008","<","200","1079",">","3"'>Low calorie bread</option>
        <option  value='"1005",">","10","1003",">","12","1004",">","7","1008","<","200","1079",">","3"'>Low calorie burgers & sandwiches</option>
        <option  value='"1005",">","10","1003",">","5","1004",">","7","1008","<","200","1079",">","2"'>Low calorie pizza & pasta</option>
        <option  value='"1004",">","3","1003",">","20","1004",">","15","1005","<","3","1008","<","250"'>Low calorie cheeses & meats</option>
        <option  value='"1005",">","5","2000","<","10","1079",">","1","1008","<","50","1093","<","100","1003","<","3","1004","<","2","2000",">","2","1008",">","20"'>Low calorie fruits</option>
        <option  value='"1018",">","1","1005","<","1"'>Low carb alcohol</option>
        <option  value='"1008","<","1"'>Zero calories</option>
        <option  value='"1051",">","89"'>High water content</option>
        <option  value='"1093","<","200"'>Low sodium</option>
        <option  value='"1087",">","300"'>High Calcium</option>
        <option  value='"1089",">","3"'>High Iron</option>
        <option  value='"1092",">","1000"'>High Potassium</option>
        <option  value='"1090",">","80"'>High Magnesium</option>
        <option  value='"1095",">","2"'>High Zinc</option>
        <option  value='"1096",">","20"'>High Chromium</option>
        <option  value='"1079",">","10"'>High Fiber</option>
        <option  value='"1097",">","0.4"'>High Cobalt</option>
        <option  value='"1100",">","30000"'>High Iodine</option>
        <option  value='"1104",">","200"'>High Vitamin A</option>
        <option  value='"1110",">","150"'>High Vitamin D</option>
        <option  value='"1162",">","20"'>High Vitamin C</option>
        <option  value='"1175",">","20"'>High Vitamin B-6</option>
    </select>
    
    
        <input type="checkbox" id="filter_high_fat"> <span style="font-size: 11px">Fatty</span>
        <input type="checkbox" id="filter_high_carbs"> <span style="font-size: 11px">High carb</span>
        <input type="checkbox" id="filter_high_sugar"> <span style="font-size: 11px">High sugar</span>
        <input type="checkbox" id="filter_high_sodium"> <span style="font-size: 11px">High sodium</span>
        <input type="checkbox" id="filter_high_rank"> <span style="font-size: 11px">Unhealthy</span>
        <input type="checkbox" id="filter_high_protein_or_fiber"> <span style="font-size: 11px">High protein or fiber</span>
    </td>
    <td colspan="8">
        <input style="width: 70px; font-size: 12px;" type="text" id="filter_protein" placeholder="Protein">
        <input style="width: 45px; font-size: 12px;" type="text" id="filter_fat" placeholder="Fat">
        <input style="width: 55px; font-size: 12px;" type="text" id="filter_carbs" placeholder="Carbs">
        <input style="width: 60px; font-size: 12px;" type="text" id="filter_sugar" placeholder="Sugar">
        <!--<input style="width: 55px" type="text" id="filter_inulin" placeholder="Inulin">-->
        <input style="width: 70px; font-size: 12px;" type="text" id="filter_calories" placeholder="Calories">
        <input style="width: 70px; font-size: 12px;" type="text" id="filter_variance" placeholder="Variance">
        <input onclick="document.getElementById('recipe_table').style.display=(document.getElementById('recipe_table').style.display=='none')?'block':'none'" style="font-size: 12px;" type="submit" value="Show/hide recipes &darr;"></td>
    </tr>
    </table>
    
    
    
    
    
    <table style="display: )" << (foods_to_balance.empty()?"none":"block") << R"(" id="recipe_table"></table>
    
    
    <table>
    <tr><td colspan="9" style="width: 100%; border-bottom: 1px solid #ddd; height: 15px;"></td>
    </tr><tr>
        <td><b>Set goal</b></td>
        <td>
        <script>
            function show_hide_goal_details()
            {
                if (document.getElementById('your_goal').value=='lose_weight')
                {
                    document.getElementById('your_goal_weight').style.display = 'block';
                    document.getElementById('your_goal_units').style.display = 'block';
                    document.getElementById('your_goal_in').style.display = 'block';
                    document.getElementById('your_goal_deadline').style.display = 'block';
                }
                else
                {
    document.getElementById('your_goal_weight').style.display = 'none';
    document.getElementById('your_goal_units').style.display = 'none';
    document.getElementById('your_goal_in').style.display = 'none';
    document.getElementById('your_goal_deadline').style.display = 'none';
                }
            }
            function submit_goal()
            {
                if (is_imperial)
                    target_weight = document.getElementById('your_goal_weight').value * 0.454;
                else
                    target_weight = document.getElementById('your_goal_weight').value;
                target_weight_deadline = document.getElementById('your_goal_deadline').value;
    
                // Apply all local changes and reload the page
                apply_changes_from_sliders();
            }
        </script>
        <select onchange="show_hide_goal_details();" style="background-color: #f1f1f1; font-size: 16px; height: 40px; border: 1px solid #d4d4d4; border-bottom: none; border-top: none; z-index: 99;" id="your_goal" autocomplete="off">
    
        <option )" << (target_weight?"":"selected") << R"(>Be healthy and maintain my current weight</option>
        <option )" << (target_weight?"selected":"") << R"( value="lose_weight">Be healthy and weigh </option>
    
        </select>
        </td>
        <td>
            <input style="display: )" << (target_weight?"block":"none") << R"(;" id="your_goal_weight" type="text" value=')" << (target_weight?std::to_string((int)std::round(is_imperial?target_weight/0.454:target_weight)):"") << R"(' placeholder="this many">
        </td>
        <td>
            <span style="display: )" << (target_weight?"block":"none") << R"(;" id="your_goal_units" > )" << (is_imperial?"lbs":"kgs") << R"( </span>
        </td>
        <td>
            <span style="display: )" << (target_weight?"block":"none") << R"(;" id="your_goal_in" > in </span>
        </td>
    <td>
    <select style="display: )" << (target_weight?"block":"none") << R"(; background-color: #f1f1f1; font-size: 16px; height: 40px; border: 1px solid #d4d4d4; border-bottom: none; border-top: none; z-index: 99;" id="your_goal_deadline" autocomplete="off">
    <option )" << (target_weight_deadline==7?"selected":"") << R"( value="7">one week</option>
    <option )" << (target_weight_deadline==14?"selected":"") << R"( value="14">two weeks</option>
    <option )" << (target_weight_deadline==30?"selected":"") << R"( value="30">one month</option>
    <option )" << (target_weight_deadline==60?"selected":"") << R"( value="60">two months</option>
    <option )" << (target_weight_deadline==90?"selected":"") << R"( value="90">three months</option>
    </select>
    </td>
    <td>
        <input onclick="submit_goal();" type="submit" value="Submit goal">
    </td>
    <td>
        <input onclick="document.getElementById('nutrient_table').style.display=(document.getElementById('nutrient_table').style.display=='none')?'block':'none'" type="submit" value="Show/hide details &darr;">
    <input onclick="document.getElementById('tutorial7').style.display='block'" type="submit" value="Help?">
    </td>
    
    </tr>
    </table>
    
    <table id="nutrient_table">
    <tr><td colspan="6" style="height: 15px;"></td></tr>)" << std::endl;
    
    for (auto &x : bad_nutrients)
    {
        int nutrient_id = x.first;
        float amount = x.second;
        nutrient *n = get_nutrient(nutrient_id);
        if (!n || n->nutrient_name_.empty())
            continue;
        
        output << "<tr><td>[" << nutrient_id << "]</td><td>" << n->nutrient_name_ << "</td><td> < </td><td>"
        << R"(
        <input id="nutrient)" + std::to_string(nutrient_id) +
        R"(" style="width: 75px; height: 20px;" type="text" value=")"
        << amount << R"("></td>)" << std::endl;
        
        float real_nutrient_amount = nutrient_comsumption_helper(foods_to_balance, nutrient_id, rec);
        output << R"x(<td><div class="range-wrap">
            <input title='Move this slider to change the total consumption of this nutrient: this will affect amounts of all food except those checked in checkboxes on the left of the food' class="range" type="range" min="1" max="10000" value=")x"
            << range_value_helper(real_nutrient_amount, amount, 10000)
            << R"x(" id="myRange_nutrient_)x"
            << nutrient_id << R"x(" list="rangedatalist_nutrient_)x"
            << nutrient_id << R"x(" />
            <output class="bubble" id="bubble_nutrient_)x"
            << nutrient_id << R"x("></output>
        </div>
        <datalist id="rangedatalist_nutrient_)x" << nutrient_id << R"x("><option value="5000">)x" << amount << R"x(</option></datalist></td>
        )x" << std::endl;

        
        // Show info if it's OK with limits
        int percent_delta = (int)(100.0*std::ceil(real_nutrient_amount - amount)/amount);
        if (percent_delta > 0)
            output << "<td><span id=\"nutrient_details_" << nutrient_id
            << "\"><span style=\"color: red;\"><b>(" << percent_delta
            << " % more)</b></span></span></td>";
        else
            output << "<td><span id=\"nutrient_details_" << nutrient_id
                       << "\"><span style=\"color: green;\"><b>GOOD</b></span></span></td>";

        // This is to catch press enter on food amounts
        output << R"(</tr>
        <script>
        {
            let nutrient_id = )" << nutrient_id << R"(;
            let input = document.getElementById("nutrient" + nutrient_id);
            input.addEventListener("keyup", function(event) {
              if (event.keyCode === 13) {
                 let amount = Number(input.value);
                 if (amount === Number.NaN)
                    amount = 0;
        
                // Change nutrient reference amount
                if (amount === 0)
                    nutrient_reference_values.delete(nutrient_id);
                else
                    nutrient_reference_values.set(nutrient_id, -amount);
    
                // Apply changes and reload the page
                apply_changes_from_sliders();
              }
            });
        }
        </script>
        )";
    } // for (auto &x : bad_nutrients)

    // New bad nutrient
    // Note: $$$$ - is a placeholder for the limit of a newly added nutrient
    std::string new_nutrient_url = "/balance?";
    construct_balance_food_url_helper(foods_to_balance, new_nutrient_url);
    construct_balance_nutrients_url_helper(good_nutrients, bad_nutrients, new_nutrient_url);
    need_comma = (good_nutrients.size() + bad_nutrients.size()) >= 1;
    if (new_nutrient_url[new_nutrient_url.size()-1] == '?')
        new_nutrient_url += "nutrients=";
    else
    if (!need_comma)
        new_nutrient_url += "&nutrients=";
    output << R"(
    <tr><td colspan="6"><div class="autocomplete" style="width:300px;">
      <input id="addBadNutrient" type="text" placeholder="Add bad nutrient" autocomplete="off">
    </div>
    <script>
        autocomplete("nutrients", 777777777,
            ")" + new_nutrient_url + (need_comma?",":"") +
        R"(%22777777777%22,%22%3C%22,%22$$$$%22", document.getElementById("addBadNutrient"), arr);
    </script>
    </td></tr>
    )" << std::endl;
    
    for (auto &x : good_nutrients)
    {
        int nutrient_id = x.first;
        float amount = x.second;
        nutrient *n = get_nutrient(nutrient_id);
        if (!n || n->nutrient_name_.empty())
            continue;
        
        float real_nutrient_amount = nutrient_comsumption_helper(foods_to_balance, nutrient_id, rec);
        
        output << "<tr><td>[" << nutrient_id << "]</td><td>" << n->nutrient_name_ << "</td><td> > </td><td>"
        
        << R"(
        <input id="nutrient)" + std::to_string(nutrient_id) +
        R"(" style="width: 75px; height: 20px;" type="text" value=")"
        << amount << R"("></td>)" << std::endl;

        output << R"x(<td><div class="range-wrap">
            <input class="range" type="range" min="1" max="10000" value=")x"
            << range_value_helper(real_nutrient_amount, amount, 10000)
            << R"x(" id="myRange_nutrient_)x"
            << nutrient_id << R"x(" list="rangedatalist_nutrient_)x"
            << nutrient_id << R"x(" />
            <output class="bubble" id="bubble_nutrient_)x"
            << nutrient_id << R"x("></output>
        </div>
        <datalist id="rangedatalist_nutrient_)x" << nutrient_id << R"x("><option value="5000">)x" << amount << R"x(</option></datalist></td>
        )x" << std::endl;

        // Show info if it's OK with limits
        int percent_delta = (int)(100.0*std::ceil(amount - real_nutrient_amount)/amount);
        if (percent_delta > 0)
            output << "<td><span id=\"nutrient_details_" << nutrient_id
            << "\"><span style=\"color: red;\"><b>(" << percent_delta
            << " % less)</b></span></span></td>";
        else
            output << "<td><span id=\"nutrient_details_" << nutrient_id
            << "\"><span style=\"color: green;\"><b>GOOD</b></span></span></td>";

        bool need_comma = (good_nutrients.size() + bad_nutrients.size()) > 1;
        
        // This is to catch press enter on nutrient amounts
        output << R"(</tr>
        <script>
        {
            let nutrient_id = )" << nutrient_id << R"(;
            let input = document.getElementById("nutrient" + nutrient_id);
            input.addEventListener("keyup", function(event) {
              if (event.keyCode === 13) {
                 let amount = Number(input.value);
                 if (amount === Number.NaN)
                    amount = 0;

            // Change nutrient reference amount
            if (amount === 0)
                nutrient_reference_values.delete(nutrient_id);
            else
                nutrient_reference_values.set(nutrient_id, amount);
        
            // Apply changes and reload the page
            apply_changes_from_sliders();
        
              }
            });
        }
        </script>
        )";
    }

    // New good nutrient
    // Note: $$$$ - is a placeholder for the limit of a newly added nutrient
    output << R"x(
    <tr><td colspan="6"><div class="autocomplete" style="width:300px;">
      <input id="addGoodNutrient" type="text" placeholder="Add good nutrient" autocomplete="off">
    </div>
    <script>
        autocomplete("nutrients", 777777777,
            ")x" << new_nutrient_url << (need_comma?",":"") <<
    
    
        R"x(%22777777777%22,%22%3E%22,%22$$$$%22", document.getElementById("addGoodNutrient"), arr);
    )x" << std::ifstream((foods_to_balance.empty() ? "./bf_short_footer.html": "./bf_footer.html")).rdbuf();
}

void foods::nutrients_to_json(std::ostream &output)
{
    output << "{\"event\":\"nutrients\",\"data\":[" << std::endl;
    bool is_first = true;
    int nutrient_id = 0;
    for (auto &x : all_nutrients_)
    {
        if (x.nutrient_name_.empty())
        {
            ++nutrient_id;
            continue;
        }
        if (!is_first) output << "," << std::endl;
        is_first = false;
        output
        << "{\"nutrient_id\":" << nutrient_id
        << ",\"nutrient_name\":\"" << x.nutrient_name_
        << " " << x.nutrient_unit_name_
        << "\",\"nutrient_lower_limit\":" << get_nutrient_recommended_min_amout(nutrient_id)
        << "\",\"nutrient_upper_limit\":" << get_nutrient_recommended_max_amout(nutrient_id)
        << "}";
        ++nutrient_id;
    }
    output << std::endl << "]}" << std::endl;
}

} // namespace balanced_diet
