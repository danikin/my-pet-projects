/*
*       balanced_diet_test.cpp
*
*       (C) Denis Anikin 2020
*
*       Test for the balanced diet
*
*/

#include <iostream>
#include <fstream>
#include <sstream>

#include "httplib.h"

#include "balanced_diet.h"
#include "balanced_diet_mfp.h"
#include "balanced_diet_recipe.h"

void parse_csv_line(const std::string &line, std::vector<std::string> &entries)
{
    size_t pos = 0;
    while (pos < line.size())
    {
        // Search for a opening '"'
        pos = line.find("\"", pos);
        if (pos == std::string::npos)
            break;
        ++pos;
        
        // Found the beginning of a value
        //  pos points to the next character after the opening '"'
        // Searching for a closing '"'
        std::string value;
        while (pos < line.size())
        {
            size_t pos1 = line.find("\"", pos);
            if (pos1 == std::string::npos)
            {
                value += std::string(line.begin() + pos, line.end());
                entries.push_back(value);
                return;
            }
            
            // pos1 points to the '"' in the middle of the value (it can be a closing '"' or not)
            // If there is a second '"' in a row
            if (pos1 < line.size()-1 && line[pos1+1] == '"')
            {
                // Two quotes in a row - the value still continues
                value += std::string(line.begin() + pos, line.begin() + pos1 + 1);
                
                // Move to the first character after the second '"'
                pos = pos1 + 2;
            }
            else
            {
                // A real closing '"'
                value += std::string(line.begin() + pos, line.begin() + pos1);
                entries.push_back(value);
                
                // Move pos to the character after the closing '"'
                pos = pos1 + 1;
                break;
            }
        }
    }
}

template<class ...Args>
std::string my_sprintf(const char *format, Args ... args)
{
    char str[256];
    snprintf(str, sizeof(str)-1, format, args ...);
    return str;
}

// Assemble recipes
// Format for recipes:
//  "recipe_food_id1","recipe_name1","N_foods","food_id1","food_amount1",...,"food_idN","food_amountN",
//  "recipe_food_id2","recipe_name1","N_foods","food_id1","food_amount1",...,"food_idN","food_amountN"
void assemble_recipes(std::vector<std::string> &entries,
                      const std::string &s_recipes,
                      std::vector<balanced_diet::recipe> &recipe_vector)
{
    if (s_recipes.empty())
        return;
    
    entries.clear();
    parse_csv_line(s_recipes, entries);
    int i = 0;
    while (i+3 < entries.size())
    {
        balanced_diet::recipe r;
        r.recipe_food_id_ = std::stoi(entries[i]);
        r.recipe_name_ = entries[i+1];
        int n_foods = std::stoi(entries[i+2]);
        i += 3;
        for (int k = 0; k < n_foods; ++k)
        {
            if (i+1 >= entries.size())
                break;
            r.recipe_foods_.push_back({std::stoi(entries[i]), std::stof(entries[i+1])});
            i += 2;
        }
        recipe_vector.push_back(r);
    }
}

#define GET_I_PARAM(param) (req.get_param_value(param).empty() ? 0 : std::stoi(req.get_param_value(param)))
#define GET_I_PARAM2(param) (req.get_param_value(param).empty() ? -1 : std::stoi(req.get_param_value(param)))
#define GET_F_PARAM(param) (req.get_param_value(param).empty() ? 0.0 : std::stof(req.get_param_value(param)))

namespace balanced_diet {
    void upload_stat(std::istream &is, std::multimap<float, dish_satiety_info> &dish_satiety_top);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: ./balanced_diet_test port dir_for_csv" << std::endl;
        return 0;
    }
    
    std::string dir_for_csv = argv[2];
    
    balanced_diet::foods fds;
    balanced_diet::mfp_foods mfp_fds;
    
    std::ifstream food_diaries(dir_for_csv + "mfp-diaries.tsv");
    mfp_fds.upload_stat(food_diaries);
    
    std::ifstream food_csv(dir_for_csv + "food.csv");
    
    std::cerr << "Uploading food" << std::endl;
    
    std::string line;
    std::vector<std::string> entries;
    getline(food_csv, line);
    while (getline(food_csv, line))
    {
        try
        {
            //std::cerr << "line=" << line << std::endl;
            entries.clear();
            parse_csv_line(line, entries);
        
            /*std::cerr << "entries";
            for (auto &x : entries)
                std::cerr << " '" << x << "'";
            std::cerr << std::endl;*/
            
            if (!entries[0].empty())
                fds.upload_food(std::stoi(entries[0]), entries[2], entries[1]);
        }
        catch (std::exception &e)
        {
            std::cerr << "food_csv: exception: " << e.what() << ", line=" << line
                << ", entries[0]=" << entries[0] << std::endl;
        }
    }

    std::ifstream nutrient_csv(dir_for_csv + "nutrient.csv");

    std::cerr << "Uploading nutrients" << std::endl;

    getline(nutrient_csv, line);
    while (getline(nutrient_csv, line))
    {
        try
        {
        //std::cout << "line=" << line << std::endl;
        entries.clear();
        parse_csv_line(line, entries);
        
        //std::cout << entries[0] << "," << entries[1] << std::endl;
        if (!entries[0].empty())
            fds.upload_nutrient(std::stoi(entries[0]), entries[1], entries[2]);
        }
        catch (std::exception &e)
        {
            std::cerr << "nutrient_csv: exception: " << e.what() << ", line=" << line
                << ", entries[0]=" << entries[0] << std::endl;
        }
    }

    std::ifstream food_nutrient_csv(dir_for_csv + "food_nutrient.csv");

    std::cerr << "Uploading food_nutrient" << std::endl;

    getline(food_nutrient_csv, line);
    while (getline(food_nutrient_csv, line))
    {
        try
        {
        //std::cout << "line=" << line << std::endl;
        entries.clear();
        parse_csv_line(line, entries);
        
        //std::cout << entries[0] << "," << entries[1] << std::endl;
        if (!entries[1].empty() && !entries[2].empty())
            fds.upload_food_nutrient(std::stoi(entries[1]), std::stoi(entries[2]), std::stof(entries[3]));
        }
        catch (std::exception &e)
        {
            std::cerr << "food_nutrient_csv: exception: " << e.what() << ", line=" << line
                << ", entries[1]=" << entries[1]
                << ", entries[2]=" << entries[2]
                << std::endl;
        }
    }
    fds.sort_nutrients();

    std::cerr << "Uploading measure_unit" << std::endl;
    
    std::map<int, std::string> measure_units_temp_map;
    
    std::ifstream measure_unit_csv(dir_for_csv + "measure_unit.csv");
    getline(measure_unit_csv, line);
    while (getline(measure_unit_csv, line))
    {
        try
        {
            entries.clear();
            parse_csv_line(line, entries);
        
            measure_units_temp_map[std::stoi(entries[0])] = entries[1];
        }
        catch (std::exception &e)
        {
            std::cerr << "measure_unit_csv: exception: " << e.what() << ", line=" << line
                << ", entries[1]=" << entries[1]
                << ", entries[2]=" << entries[2]
                << std::endl;
        }
    }
    
    std::cerr << "Uploading food_portion" << std::endl;
    
    std::ifstream food_portion_csv(dir_for_csv + "food_portion.csv");
    getline(food_portion_csv, line);
    while (getline(food_portion_csv, line))
    {
        try
        {
            entries.clear();
            parse_csv_line(line, entries);
            int food_id = std::stoi(entries[1]);
            std::string food_standard_unit;
            if (!entries[3].empty() || !entries[4].empty() || !entries[5].empty() || !entries[6].empty())
            {
                // Take the unit either from the standard map or from the specified string
                std::string amount = entries[3];
                std::string modifier = entries[6];
                std::string unit;
                if (!entries[4].empty() && entries[4] != "9999")
                    unit = measure_units_temp_map[std::stoi(entries[4])];
                else
                    unit = entries[5];
                
                food_standard_unit = amount;
                if (!food_standard_unit.empty() && !unit.empty()) food_standard_unit += " ";
                food_standard_unit += unit;
                if (!food_standard_unit.empty() && !modifier.empty()) food_standard_unit += " ";
                food_standard_unit += modifier;
            }
            float food_standard_weight = std::stof(entries[7]);
            
            balanced_diet::foods::food *f = fds.get_food(food_id);
            if (f && !f->food_name_.empty())
            {
                f->food_standard_unit_ = food_standard_unit;
                f->food_standard_weight_ = food_standard_weight;
                
                //std::cerr << food_id << ": " << f->food_standard_unit_ << " " << f->food_standard_weight_ << "g" << std::endl;
            }
        }
        catch (std::exception &e)
        {
            std::cerr << "food_portion_csv: exception: " << e.what() << ", line=" << line
                << ", entries[1]=" << entries[1]
                << ", entries[2]=" << entries[2]
                << std::endl;
        }
    }
    
    fds.postupload_steps();
    
    std::cerr << "Starting http server" << std::endl;
    
    httplib::Server svr;
    
    // /ratio?ratios="1003","60","1004","10","1005","5","1079,"10"&filters="1003",">","20","1004","<","5"
    svr.Get("/ratio", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::vector<std::string> entries;
        std::stringstream ss;
        bool is_json;
        try
        {
            parse_csv_line(req.get_param_value("ratios"), entries);
            std::vector<std::pair<int, float> > ratios;
            for (int i = 0; i < entries.size(); i += 2)
                ratios.push_back({std::stoi(entries[i]), std::stof(entries[i+1])});

            std::string s_filters;
            std::vector<std::pair<int, float> > filters;
            if (!req.get_param_value("filters").empty())
            {
                s_filters = req.get_param_value("filters");
                entries.clear();
                parse_csv_line(s_filters, entries);
                for (int i = 0; i < entries.size(); i += 3)
                {
                    filters.push_back({std::stoi(entries[i]),
                    entries[i+1] == "<" ? -std::stof(entries[i+2]) : std::stof(entries[i+2])
                    });
                }
            }
            
            std::vector<std::string> search_string_include;
            if (!req.get_param_value("inc").empty())
                balanced_diet::divide_into_words(req.get_param_value("inc"), search_string_include);
            std::vector<std::string> search_string_exclude;
            if (!req.get_param_value("exc").empty())
                balanced_diet::divide_into_words(req.get_param_value("exc"), search_string_exclude);
            
            int limit_match_coefficient = 1000000000;
            if (!req.get_param_value("limit").empty())
                limit_match_coefficient = std::stoi(req.get_param_value("limit"));
            
            int nutrient_to_sort = -1;
            int limit_after_sort = -1;
            if (!req.get_param_value("sort").empty())
                nutrient_to_sort = std::stoi(req.get_param_value("sort"));
            if (!req.get_param_value("limitsort").empty())
                limit_after_sort = std::stoi(req.get_param_value("limitsort"));
            
            is_json = !req.get_param_value("json").empty();
            fds.search_by_ratio(ratios,
                                filters,
                                search_string_include,
                                search_string_exclude,
                                limit_match_coefficient,
                                ss,
                                is_json,
                                nutrient_to_sort,
                                limit_after_sort);
        }
        catch (std::exception &e)
        {
            std::cerr << "exception on /ratio, e.what()=" << e.what()<< std::endl;
            for (int i = 0; i < entries.size(); ++i)
                std::cerr << "entry: '" << entries[i] << "'" << std::endl;
        }

        res.set_content(ss.str(), (is_json?"application/json; charset=utf-8":"text/html; charset=utf-8"));
    });
    
    // /balance?foods=&nutrients=
    svr.Get("/balance", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::vector<std::string> entries;
        std::stringstream ss;
        try
        {
            parse_csv_line(req.get_param_value("foods"), entries);
            std::vector<std::pair<int, float> > foods_to_balance;
            for (int i = 0; i < entries.size(); i += 2)
                foods_to_balance.push_back({std::stoi(entries[i]), std::stof(entries[i+1])});
            
            // Just save the recipe
            if (!req.get_param_value("recipe").empty())
            {
                fds.save_recipe(req.get_param_value("recipe"), foods_to_balance);
                ss << "{\"event\":\"recipe_saved\"}" << std::endl;
            }
            else
            {

            std::string s_nutrients;
            std::vector<std::pair<int, float> > good_nutrients;
            std::vector<std::pair<int, float> > bad_nutrients;
            if (!req.get_param_value("nutrients").empty())
            {
                s_nutrients = req.get_param_value("nutrients");
                entries.clear();
                parse_csv_line(s_nutrients, entries);
                for (int i = 0; i < entries.size(); i += 3)
                {
                    if (entries[i+1] == ">")
                        good_nutrients.push_back({std::stoi(entries[i]),std::stof(entries[i+2])});
                    else
                        bad_nutrients.push_back({std::stoi(entries[i]),std::stof(entries[i+2])});
                }
            }
                
            // "groups" contains meal groups in one single list like this:
            //  "breakfast","123","456","lunch,"467","789","dinner","111,"15",117"
            std::vector<std::pair<std::string, std::vector<int> > > groups;
            if (!req.get_param_value("groups").empty())
            {
                std::string s_groups = req.get_param_value("groups");
                entries.clear();
                parse_csv_line(s_groups, entries);
                std::vector<int> *current_group = NULL;
                for (int i = 0; i < entries.size(); ++i)
                {
                    if (entries[i][0] >= '0' && entries[i][0] <= '9')
                    {
                        int food_id = std::stoi(entries[i]);
                        current_group->push_back(food_id);
                    }
                    else
                    {
                        groups.push_back({entries[i], {}});
                        current_group = &groups[groups.size()-1].second;
                    }
                }
            }
                
                std::string current_meal_plan_name;
                if (!req.get_param_value("meal_plan").empty())
                    current_meal_plan_name = req.get_param_value("meal_plan");
                
            entries.clear();
            parse_csv_line(req.get_param_value("mr"), entries);
            std::vector<int> mr_data;
            for (int i = 0; i < entries.size(); ++i)
                mr_data.push_back({std::stoi(entries[i])});
                
            std::string s_recipes = req.get_param_value("recipes");
            std::vector<balanced_diet::recipe> recipe_vector;
            assemble_recipes(entries, s_recipes, recipe_vector);
            balanced_diet::recipes rec(fds, recipe_vector);

            int target_weight = GET_I_PARAM("target_weight");
            int target_weight_deadline = GET_I_PARAM("target_weight_deadline");
            int is_imperial = GET_I_PARAM("imperial");
            int add_healthy_food = GET_I_PARAM("add_healthy_food");

            std::vector<std::string> healthy_search_string;
            if (!req.get_param_value("healthy_search_string").empty())
                balanced_diet::divide_into_words(req.get_param_value("healthy_search_string"), healthy_search_string);
                
            fds.balance(foods_to_balance,
                         good_nutrients,
                         bad_nutrients,
                        groups,
                        mr_data,
                        current_meal_plan_name,
                        &rec,
                        target_weight,
                        target_weight_deadline,
                        is_imperial,
                        add_healthy_food,
                        healthy_search_string,
                         ss);
                
            } // else
        }
        catch (std::exception &e)
        {
            std::cerr << "exception on /balance, e.what()=" << e.what()<< std::endl;
            for (int i = 0; i < entries.size(); ++i)
                std::cerr << "entry: '" << entries[i] << "'" << std::endl;
        }
        
        res.set_content(ss.str(), "text/html; charset=utf-8");
    });
    
    // /balance_simple?foods=&nutrients=&nutrient_id=&nutrient_diff=
    svr.Get("/balance_simple", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::vector<std::string> entries;
        std::stringstream ss;
        try
        {
            parse_csv_line(req.get_param_value("foods"), entries);
            std::vector<std::pair<int, float> > food_curent_values;
            for (int i = 0; i < entries.size(); i += 2)
                food_curent_values.push_back({std::stoi(entries[i]), std::stof(entries[i+1])});
            
            std::vector<int> checked_foods;
            if (!req.get_param_value("checked_foods").empty())
            {
                entries.clear();
                parse_csv_line(req.get_param_value("checked_foods"), entries);
                for (int i = 0; i < entries.size(); ++i)
                    checked_foods.push_back(std::stoi(entries[i]));
            }
            
            entries.clear();
            parse_csv_line(req.get_param_value("nutrients"), entries);
            std::vector<std::pair<int, float> > nutrient_curent_values;
            for (int i = 0; i < entries.size(); i += 2)
                nutrient_curent_values.push_back({std::stoi(entries[i]), std::stof(entries[i+1])});
            
            int nutrient_id = GET_I_PARAM("nutrient_id");
            float nutrient_old_value = GET_I_PARAM("nutrient_old_value");
            float nutrient_new_value = GET_I_PARAM("nutrient_new_value");

            // "groups" contains meal groups in one single list like this:
            //  "breakfast","123","456","lunch,"467","789","dinner","111,"15",117"
            std::vector<std::pair<std::string, std::vector<int> > > groups;
            if (!req.get_param_value("groups").empty())
            {
                std::string s_groups = req.get_param_value("groups");
                entries.clear();
                parse_csv_line(s_groups, entries);
                std::vector<int> *current_group = NULL;
                for (int i = 0; i < entries.size(); ++i)
                {
                    if (entries[i][0] >= '0' && entries[i][0] <= '9')
                    {
                        int food_id = std::stoi(entries[i]);
                        current_group->push_back(food_id);
                    }
                    else
                    {
                        groups.push_back({entries[i], {}});
                        current_group = &groups[groups.size()-1].second;
                    }
                }
            }
            
            // Data for MR
            //  Gender: 0 (M), 1 (F)
            //  Weight in kg
            //  Height in cm
            //  Age
            //  Exercise:
            //      0   -   Sedentary: little or no exercise x1.2
            //      1   -   Exercise 1-3 times/week x1.375
            //      2   -   Exercise 4-5 times/week x1.465
            //      3   -   Daily exercise or intense exercise 3-4 times/week x1.55
            //      4   -   Intense exercise 6-7 times/week x1.725
            //      5   -   Very intense exercise daily, or physical job x1.9
            //  Fat percent
            //      0   -   Essential   W 11.5% M 3.5%  (19/25% protein)
            //      1   -   Athletes    W 17%   M 9.5%  (18/20% protein)
            //      2   -   Fitnes      W 22.5% M 15.5% (15/17.5% protein)
            //      3   -   Average     W 28%   M 21.5% (9/15% protein)
            //      4   -   Obese       W 32+%  M 25+%  (7/10% protein)
            //
            //
            //
            //  Men BMR = 66.4730 + (13.7516 x weight in kg) + (5.0033 x height in cm) – (6.7550 x age in years)
            //  Women BMR = 655.0955 + (9.5634 x weight in kg) + (1.8496 x height in cm) – (4.6756 x age in years)
            //  Protein intake 35% of MR
            entries.clear();
            parse_csv_line(req.get_param_value("mr"), entries);
            std::vector<int> mr_data;
            for (int i = 0; i < entries.size(); ++i)
                mr_data.push_back({std::stoi(entries[i])});
            
            std::string s_recipes = req.get_param_value("recipes");
            std::vector<balanced_diet::recipe> recipe_vector;
            assemble_recipes(entries, s_recipes, recipe_vector);
            balanced_diet::recipes rec(fds, recipe_vector);
            
            int target_weight = GET_I_PARAM("target_weight");
            int target_weight_deadline = GET_I_PARAM("target_weight_deadline");
            int add_healthy_food = GET_I_PARAM("add_healthy_food");
            

            fds.balance_simple(food_curent_values,
                               checked_foods,
                                 nutrient_curent_values,
                                 nutrient_id,
                                 nutrient_old_value,
                                nutrient_new_value,
                               groups,
                               mr_data,
                               &rec,
                               target_weight,
                               target_weight_deadline,
                               add_healthy_food,
                                 ss);
        }
        catch (std::exception &e)
        {
            std::cerr << "exception on /balance_simple, e.what()=" << e.what()<< std::endl;
            for (int i = 0; i < entries.size(); ++i)
                std::cerr << "entry: '" << entries[i] << "'" << std::endl;
        }
            
        res.set_content(ss.str(), "application/json; charset=utf-8");
    });
    
    // /food_by_rank&nutrients=&nutrient_limits=&grouped_first=[n]
    svr.Get("/food_by_rank", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::vector<std::string> entries;
        std::stringstream ss;
        try
        {
            std::string s_nutrients;
            std::vector<std::pair<int, float> > good_nutrients;
            std::vector<std::pair<int, float> > bad_nutrients;
            if (!req.get_param_value("nutrients").empty())
            {
                s_nutrients = req.get_param_value("nutrients");
                entries.clear();
                parse_csv_line(s_nutrients, entries);
                for (int i = 0; i < entries.size(); i += 3)
                {
                    if (entries[i+1] == ">")
                        good_nutrients.push_back({std::stoi(entries[i]),std::stof(entries[i+2])});
                    else
                        bad_nutrients.push_back({std::stoi(entries[i]),std::stof(entries[i+2])});
                }
            }

            std::vector<std::pair<int, float> > nutrient_lower_limits;
            std::vector<std::pair<int, float> > nutrient_upper_limits;
            if (!req.get_param_value("nutrient_limits").empty())
            {
                s_nutrients = req.get_param_value("nutrient_limits");
                entries.clear();
                parse_csv_line(s_nutrients, entries);
                for (int i = 0; i < entries.size(); i += 3)
                {
                    if (entries[i+1] == ">")
                        nutrient_lower_limits.push_back({std::stoi(entries[i]),std::stof(entries[i+2])});
                    else
                        nutrient_upper_limits.push_back({std::stoi(entries[i]),std::stof(entries[i+2])});
                }
            }
            
            std::vector<std::string> search_string_include;
            if (!req.get_param_value("inc").empty())
                balanced_diet::divide_into_words(req.get_param_value("inc"), search_string_include);
            std::vector<std::string> search_string_exclude;
            if (!req.get_param_value("exc").empty())
                balanced_diet::divide_into_words(req.get_param_value("exc"), search_string_exclude);

            // inc_exact - search words for exact case sensitive search
            //  Strings go like this: "dddsds","dddd","e sddsds sdsd, sdsd"
            std::vector<std::string> search_string_include_exact;
            if (!req.get_param_value("inc_exact").empty())
            {
                entries.clear();
                parse_csv_line(req.get_param_value("inc_exact"), entries);
                for (int i = 0; i < entries.size(); ++i)
                    search_string_include_exact.push_back(entries[i]);
            }

            // exc_exact - search words for exact case sensitive search
            //  Strings go like this: "dddsds","dddd","e sddsds sdsd, sdsd"
            std::vector<std::string> search_string_exclude_exact;
            if (!req.get_param_value("exc_exact").empty())
            {
                entries.clear();
                parse_csv_line(req.get_param_value("exc_exact"), entries);
                for (int i = 0; i < entries.size(); ++i)
                    search_string_exclude_exact.push_back(entries[i]);
            }
            
            int limit_after_sort = -1;
            float min_food_rank = -1;
            float max_food_rank = -1;
            int catalogue_item_id = -1;
            int show_distance_to = -1;
            float distance_limit = -1;
            std::string popularity_group;
            if (!req.get_param_value("limitsort").empty())
                limit_after_sort = std::stoi(req.get_param_value("limitsort"));
            if (!req.get_param_value("minrank").empty())
                min_food_rank = std::stof(req.get_param_value("minrank"));
            if (!req.get_param_value("maxrank").empty())
                max_food_rank = std::stof(req.get_param_value("maxrank"));
            if (!req.get_param_value("group").empty())
                popularity_group = req.get_param_value("group");
            if (!req.get_param_value("catalogue_item_id").empty())
                catalogue_item_id = std::stoi(req.get_param_value("catalogue_item_id"));
            if (!req.get_param_value("show_distance_to").empty())
                show_distance_to = std::stoi(req.get_param_value("show_distance_to"));
            if (!req.get_param_value("distance_limit").empty())
                distance_limit = std::stof(req.get_param_value("distance_limit"));

            // Format for recipes:
            //  "recipe_food_id1","recipe_name1","N_foods","food_id1","food_amount1",...,"food_idN","food_amountN",
            //  "recipe_food_id2","recipe_name1","N_foods","food_id1","food_amount1",...,"food_idN","food_amountN"
            std::string s_recipes = req.get_param_value("recipes");
            std::vector<balanced_diet::recipe> recipe_vector;
            assemble_recipes(entries, s_recipes, recipe_vector);
            
            float protein_to_calorie_percent = GET_F_PARAM("protein_to_calorie_percent");
            float fiber_to_calorie_percent = GET_F_PARAM("fiber_to_calorie_percent");
            float min_pfind = GET_F_PARAM("min_pfind");
            int sort_by_satiety_per_cal = GET_I_PARAM("sort_by_satiety_per_cal");
            
            int min_satiety_food = GET_I_PARAM("min_satiety_food");
            float satiety_delta = GET_F_PARAM("satiety_delta");
            
            // Fill recipes structures
            balanced_diet::recipes rec(fds, recipe_vector);
            
            fds.food_rank_json(good_nutrients,
                               bad_nutrients,
                               search_string_include,
                               search_string_exclude,
                               search_string_include_exact,
                               search_string_exclude_exact,
                               limit_after_sort,
                               min_food_rank,
                               max_food_rank,
                               nutrient_lower_limits,
                               nutrient_upper_limits,
                               popularity_group.empty() ? NULL : (
                                popularity_group == "any" ? "" : popularity_group.c_str()),
                               catalogue_item_id,
                               NULL,
                               show_distance_to,
                               distance_limit,
                               &rec,
                               protein_to_calorie_percent,
                               fiber_to_calorie_percent,
                               min_pfind,
                               sort_by_satiety_per_cal,
                               
                               min_satiety_food,
                               satiety_delta,
                               ss);
        }
        catch (std::exception &e)
        {
            std::cerr << "exception on /food_by_rank, e.what()=" << e.what()<< std::endl;
            for (int i = 0; i < entries.size(); ++i)
                std::cerr << "entry: '" << entries[i] << "'" << std::endl;
        }
            
        res.set_content(ss.str(), "application/json; charset=utf-8");
    });
    
    svr.Get("/suggest_food_from_stat", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::vector<std::string> entries;
        std::stringstream ss;
        try
        {
            std::vector<std::string> search_string_include;
            if (!req.get_param_value("inc").empty())
                balanced_diet::divide_into_words_lc(req.get_param_value("inc"), search_string_include);
            std::vector<std::string> search_string_exclude;
            if (!req.get_param_value("exc").empty())
                balanced_diet::divide_into_words_lc(req.get_param_value("exc"), search_string_exclude);

            // inc_exact - search words for exact case sensitive search
            //  Strings go like this: "dddsds","dddd","e sddsds sdsd, sdsd"
            std::vector<std::string> search_string_include_exact;
            if (!req.get_param_value("inc_exact").empty())
            {
                entries.clear();
                parse_csv_line(req.get_param_value("inc_exact"), entries);
                for (int i = 0; i < entries.size(); ++i)
                    search_string_include_exact.push_back(entries[i]);
            }

            // exc_exact - search words for exact case sensitive search
            //  Strings go like this: "dddsds","dddd","e sddsds sdsd, sdsd"
            std::vector<std::string> search_string_exclude_exact;
            if (!req.get_param_value("exc_exact").empty())
            {
                entries.clear();
                parse_csv_line(req.get_param_value("exc_exact"), entries);
                for (int i = 0; i < entries.size(); ++i)
                    search_string_exclude_exact.push_back(entries[i]);
            }
            
            int limit_after_sort = -1;
            float min_food_rank = -1;
            float max_food_rank = -1;
            int catalogue_item_id = -1;
            int show_distance_to = -1;
            float distance_limit = -1;
            std::string popularity_group;
            if (!req.get_param_value("limitsort").empty())
                limit_after_sort = std::stoi(req.get_param_value("limitsort"));
            if (!req.get_param_value("minrank").empty())
                min_food_rank = std::stof(req.get_param_value("minrank"));
            if (!req.get_param_value("maxrank").empty())
                max_food_rank = std::stof(req.get_param_value("maxrank"));
            
            float protein_to_calorie_percent = GET_F_PARAM("protein_to_calorie_percent");
            float fiber_to_calorie_percent = GET_F_PARAM("fiber_to_calorie_percent");
            float min_pfind = GET_F_PARAM("min_pfind");
            
            mfp_fds.suggest_food_from_stat(
                               search_string_include,
                               search_string_exclude,
                               search_string_include_exact,
                               search_string_exclude_exact,
                               limit_after_sort,
                               min_food_rank,
                               max_food_rank,
                               protein_to_calorie_percent,
                               fiber_to_calorie_percent,
                               min_pfind,
                               ss);
        }
        catch (std::exception &e)
        {
            std::cerr << "exception on /suggest_food_from_stat, e.what()=" << e.what()<< std::endl;
            for (int i = 0; i < entries.size(); ++i)
                std::cerr << "entry: '" << entries[i] << "'" << std::endl;
        }
            
        res.set_content(ss.str(), "application/json; charset=utf-8");
    });

    svr.Get("/all_meals_with_dish", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::vector<std::string> entries;
        std::stringstream ss;
        try
        {
            int limit_after_sort = -1;
            if (!req.get_param_value("limitsort").empty())
                limit_after_sort = std::stoi(req.get_param_value("limitsort"));
            std::string dish_name = req.get_param_value("dish_name");
            int dish_id = GET_I_PARAM2("dish_id");
            mfp_fds.all_meals_with_dish(dish_id, dish_name, limit_after_sort, ss);
        }
        catch (std::exception &e)
        {
            std::cerr << "exception on /all_meals_with_dish, e.what()=" << e.what()<< std::endl;
        }
            
        res.set_content(ss.str(), "application/json; charset=utf-8");
    });
    
    // Return details of foods
    svr.Get("/food_details", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::vector<std::string> entries;
        std::stringstream ss;
        try
        {
            std::vector<int> foods;
            std::vector<int> nutrients;
            if (!req.get_param_value("foods").empty())
            {
                std::string s_foods = req.get_param_value("foods");
                entries.clear();
                parse_csv_line(s_foods, entries);
                for (int i = 0; i < entries.size(); ++i)
                    foods.push_back(std::stoi(entries[i]));
            }
            if (!req.get_param_value("nutrients").empty())
            {
                std::string s_nutrients = req.get_param_value("nutrients");
                entries.clear();
                parse_csv_line(s_nutrients, entries);
                for (int i = 0; i < entries.size(); ++i)
                    nutrients.push_back(std::stoi(entries[i]));
            }
            fds.get_food_details(foods, nutrients, ss);
        }
        catch (std::exception &e)
        {
            std::cerr << "exception on /food_details, e.what()=" << e.what()<< std::endl;
            for (int i = 0; i < entries.size(); ++i)
                std::cerr << "entry: '" << entries[i] << "'" << std::endl;
        }
        res.set_content(ss.str(), "application/json; charset=utf-8");
    });
    
    svr.Get("/nutrients", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::stringstream ss;
        fds.nutrients_to_json(ss);
        res.set_content(ss.str(), "application/json; charset=utf-8");
    });

    svr.Get("/catalogue", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::stringstream ss;
        fds.catalogue_to_json(ss);
        res.set_content(ss.str(), "application/json; charset=utf-8");
    });

    svr.Get("/suggest_words", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::stringstream ss;
        fds.suggest_words_json(req.get_param_value("w"), ss);
        res.set_content(ss.str(), "application/json; charset=utf-8");
    });
    
    svr.Get("/t", [&](const httplib::Request& req, httplib::Response& res)
    {
        std::ifstream ifs("./t.html");
        std::stringstream ss;
        std::string s;
        while (getline(ifs, s))
            ss << s << std::endl;
        
        res.set_content(ss.str(), "text/html; charset=utf-8");
    });

    svr.Get("/static", [&](const httplib::Request& req, httplib::Response& res)
    {
        if (!req.get_param_value("file").empty())
        {
            std::string filename = req.get_param_value("file");
            for (auto &x : filename)
                if (x == '/')
                    x = 'x';
            
            std::string extention;
            size_t pos = filename.find('.');
            if (pos != std::string::npos && pos < filename.size())
                extention = std::string(filename.begin() + pos + 1, filename.end());
            
            //std::cerr << "extention=" << extention << std::endl;

            filename = "./" + filename;
            
            std::ifstream ifs(filename);
            std::stringstream ss;
            std::string s;
            while (getline(ifs, s))
                ss << s << std::endl;
        
            if (extention == "css")
                res.set_content(ss.str(), "text/css; charset=utf-8");
            else
            if (extention == "js")
                res.set_content(ss.str(), "text/javascript; charset=utf-8");
            else
                res.set_content(ss.str(), "text/html; charset=utf-8");
        }
    });
    
    svr.Get("/", [&](const httplib::Request& req, httplib::Response& res)
    {
            //std::ifstream ifs("./index.html");
            std::ifstream ifs("./stat.html");
            std::stringstream ss;
            std::string s;
            while (getline(ifs, s))
                ss << s << std::endl;
        
            res.set_content(ss.str(), "text/html; charset=utf-8");
    });

    svr.Get("/main", [&](const httplib::Request& req, httplib::Response& res)
    {
            std::ifstream ifs("./index.html");
            std::stringstream ss;
            std::string s;
            while (getline(ifs, s))
                ss << s << std::endl;
        
            res.set_content(ss.str(), "text/html; charset=utf-8");
    });
    
    svr.Get("/stat", [&](const httplib::Request& req, httplib::Response& res)
    {
            std::ifstream ifs("./stat.html");
            std::stringstream ss;
            std::string s;
            while (getline(ifs, s))
                ss << s << std::endl;
        
            res.set_content(ss.str(), "text/html; charset=utf-8");
    });
    
    svr.Get("/food_pics", [&](const httplib::Request& req, httplib::Response& res)
    {
        if (!req.get_param_value("file").empty())
        {
            std::string filename = req.get_param_value("file");
            for (auto &x : filename)
                if (x == '/')
                    x = 'x';
            
            std::string extention;
            size_t pos = filename.find('.');
            if (pos != std::string::npos && pos < filename.size())
                extention = std::string(filename.begin() + pos + 1, filename.end());
            
            //std::cerr << "extention=" << extention << std::endl;

            filename = "./food_pics/" + filename;
            
            std::ifstream ifs(filename);
            std::stringstream ss;
            std::string s;
            while (getline(ifs, s))
                ss << s << std::endl;
        
            if (extention == "jpeg")
                res.set_content(ss.str(), "image/jpeg");
            else
                res.set_content(ss.str(), "text/html; charset=utf-8");
        }
    });
    
    // /natrution?food=<food id>
    svr.Get("/nutrition", [&](const httplib::Request& req, httplib::Response& res)
    {
        int food_id = std::stoi(req.get_param_value("food"));
        
        std::string s_recipes = req.get_param_value("recipes");
        std::vector<balanced_diet::recipe> recipe_vector;
        assemble_recipes(entries, s_recipes, recipe_vector);
        balanced_diet::recipes rec(fds, recipe_vector);
        
        balanced_diet::foods::food *f = NULL;
        
        if (!recipe_vector.empty())
        {
            f = rec.get_food(food_id);
            if (!f)
                f = fds.get_food(food_id);;
        }
        else
            f = fds.get_food(food_id);
        if (!f)
            return;
        
#define N(id) my_sprintf("%.1f", (f->get_nutrient_amount(id)?*f->get_nutrient_amount(id):0.0))
#define N_R(id) my_sprintf("%.1f", (100.0*f->get_nutrient_amount2(id)/fds.get_nutrient_recommended_min_amout(id)))
#define N_R_MAX(id) my_sprintf("%.1f", (100.0*f->get_nutrient_amount2(id)/fds.get_nutrient_recommended_max_amout(id)))
#define NAME f->food_name_
#define F_ID std::to_string(food_id)
        
#define N2(n) my_sprintf("%.1f", (n < main_nutrients_n ? main_nutrients[n].amount_ : 0))
#define N_R2(n) my_sprintf("%.1f", (n < main_nutrients_n ? main_nutrients[n].norm_ : 0))
#define N_N2(n) (n < main_nutrients_n ? fds.get_nutrient_middle_name(main_nutrients[n].id_) : "")

        // Get 8 nutrients with the most amount of daily percent for this food
        struct _name_doesnt_matter_ {int id_; double amount_, norm_;};
        _name_doesnt_matter_ main_nutrients[8];
        int main_nutrients_n = 0;
        for (auto &x : f->nutrients_sorted_daily_norm_)
        {
            // Skip nutrients that will be shown anyway
            int nutrient_id = x.first;
            if (nutrient_id == 1003 || nutrient_id == 1004 || nutrient_id == 1005 || nutrient_id == 1008 ||
                nutrient_id == 1093 || nutrient_id == 1079 || nutrient_id == 1253 || nutrient_id == 1258 ||
                nutrient_id == 1292 || nutrient_id == 1292 || nutrient_id == 2000)
                continue;
            
            main_nutrients[main_nutrients_n++] = {
                .id_ = nutrient_id,
                .amount_ = f->get_nutrient_amount2(nutrient_id),
                .norm_ = 100 * x.second
            };
            
            //std::cerr << "/nutrients: nutrient_id=" << nutrient_id << ", amount=" <<
              //  f->get_nutrient_amount2(nutrient_id) << ", norm=" << x.second
                //<< std::endl;
            
            if (main_nutrients_n >= 8)
                break;
        }
        
        std::string render_text;
        render_text = render_text + R"(
        
        <style>

        table {
          border: 1px solid black;
        }

        td {
          border-style: none none solid none;
          border-color: black;
          border-width: 1px;
        }

        table {
          width: 300px;
        }

        </style>


        <div><div data-attrid="kc:/food/food:energy" data-md="37" lang="en-RU" data-ved="2ahUKEwiTpZ_r3ebrAhWIw4sKHdalBioQkCkwF3oECAgQAA"><!--m-->

        <div  aria-level="3" role="heading" style="font-size: 19px;" >Nutrition Facts</div><div id="kno-nf-nc"><div class="VGZbXd PZPZlf" data-ved="2ahUKEwiTpZ_r3ebrAhWIw4sKHdalBioQ1igwF3oECAgQAg"><div class="BiYF3"><div class="LN9Pb"><select class="Ev4pye kno-nf-fs" style="display:block;line-height:normal;background-color:#fff" title=")" + NAME + R"(" disabled="disabled"><option selected="selected" value="/m/0pl954_">)" + NAME + R"(</option><option value="/m/0pl6c8v">)" + NAME + R"(</option></select><span title=")" + NAME + R"(" class="kno-nf-sbl" aria-hidden="true">)" + NAME + R"(</span><div class="ITcCnf"></div></div></div></div>
        <div><img src="/food_pics?file=)" + F_ID + R"(.jpeg" width="256" height="256" onerror="this.style.display='none'"/></div>
        
        <div class="bYTA7">Sources include:&nbsp;<span data-mid="/m/0pl954_" class="kno-ftr-src"><a href="http://ndb.nal.usda.gov/ndb/search/list?qlookup=12117" data-ved="2ahUKEwiTpZ_r3ebrAhWIw4sKHdalBioQkh8oADAXegQICBAG" ping="/url?sa=t&amp;source=web&amp;rct=j&amp;url=http://ndb.nal.usda.gov/ndb/search/list%3Fqlookup%3D12117&amp;ved=2ahUKEwiTpZ_r3ebrAhWIw4sKHdalBioQkh8oADAXegQICBAG" target="_blank" rel="noopener">USDA</a></span><span data-mid="/m/0pl6c8v" style="display:none" class="kno-ftr-src"><a href="http://ndb.nal.usda.gov/ndb/search/list?qlookup=12118" data-ved="2ahUKEwiTpZ_r3ebrAhWIw4sKHdalBioQkh8oATAXegQICBAH" ping="/url?sa=t&amp;source=web&amp;rct=j&amp;url=http://ndb.nal.usda.gov/ndb/search/list%3Fqlookup%3D12118&amp;ved=2ahUKEwiTpZ_r3ebrAhWIw4sKHdalBioQkh8oATAXegQICBAH" target="_blank" rel="noopener">USDA</a></span></div>




<table class="AYBNrd"><tr class="PZPZlf" data-ved="2ahUKEwiTpZ_r3ebrAhWIw4sKHdalBioQ2SgwF3oECAgQCA"><td class="REbHqc"><div class="BiYF3 HIf9ae"><div class="LN9Pb"><span style="font-weight: bold;">Amount Per</span> <select class="Ev4pye kno-nf-ss" title="100 grams" disabled="disabled"><option value="1 tbsp">1 tbsp (15 g)</option><option selected="selected" value="100 grams">100 grams</option><option value="1 cup">1 cup (240 g)</option></select><span title="100 grams" class="kno-nf-sbl" aria-hidden="true">100 grams</span><div class="ITcCnf"></div></div></div></td></tr><tr class="PZPZlf kno-nf-cq" data-ved="2ahUKEwiTpZ_r3ebrAhWIw4sKHdalBioQ0ygwF3oECAgQDA"><td style="font-weight: bold;"><span class="V6Ytv">Calories</span> <span class="abs">)" + N(1008) +
    
    R"(</span></td></tr></table></div><!--n--></div><div class="mod NFQFxe" data-attrid="kc:/food/food:nutrition" data-md="38" lang="en-RU" data-ved="2ahUKEwiTpZ_r3ebrAhWIw4sKHdalBioQkCkwGHoECBEQAA"><!--m--><div jscontroller="o3UAsc" class="PZPZlf" id="kno-nf-na" jsdata="njrd;;AkilJU" role="list" jsaction="rcuQ6b:npT2md" data-ved="2ahUKEwiTpZ_r3ebrAhWIw4sKHdalBioQ1CgoADAYegQIERAB"><table class="AYBNrd"><col><col style="width:4.8em"><tr class="slVwZ"><td style="text-align: right; font-weight: bold;" colspan="2">&#37; Daily Value<span>*</span></td></tr><tr data-mid="/m/04k8n" class="kno-nf-nr" role="listitem"><td style="font-weight: bold;"><span class="V6Ytv">Total Fat</span> <span class="abs">)" + N(1004) + R"(g</span></td><td style="text-align:right;"><span class="pdv">)" + N_R_MAX(1004) + R"(%</span></td></tr><tr data-mid="/m/01n78x" class="kno-nf-nr" role="listitem"><td style="text-indent: 30px;"><span>Saturated fat</span> <span class="abs">)" + N(1258) + R"(g</span></td><td style="text-align: right;"><span class="pdv">)" + N_R_MAX(1258) +   R"(%</span></td></tr><tr data-mid="/m/05gh50" class="kno-nf-nr" role="listitem"><td style="text-indent: 30px;"><span>Polyunsaturated fat</span> <span class="abs">)" + N(1293) + R"(g</span></td><td class="ellip fooDZe"><span class="pdv"></span></td></tr><tr data-mid="/m/041r51" class="kno-nf-nr" role="listitem"><td style="text-indent: 30px;"><span>Monounsaturated fat</span> <span class="abs">)" + N(1292) +
    R"( g</span></td><td class="ellip fooDZe"><span class="pdv"></span></td></tr><tr data-mid="/m/01w_3" class="kno-nf-nr" role="listitem"><td style="font-weight: bold;"><span class="V6Ytv">Cholesterol</span> <span class="abs">)" + N(1253) + R"( mg</span></td><td style="text-align: right;"><span class="pdv">)" + N_R_MAX(1253)  + R"(%</span></td></tr><tr data-mid="/m/025sf0_" class="kno-nf-nr" role="listitem"><td style="font-weight: bold;"><span class="V6Ytv">Sodium</span> <span class="abs">)" + N(1093) +
    R"( mg</span></td><td style="text-align: right;"><span class="pdv">)" + N_R_MAX(1093) + R"(%</span></td></tr><tr data-mid="/m/025s7j4" class="kno-nf-nr" role="listitem"><td style="font-weight: bold;"><span class="V6Ytv">Potassium</span> <span class="abs">)" + N(1092) + R"( mg</span></td><td style="text-align: right;"><span class="pdv">)" + N_R(1092) + R"(%</span></td></tr><tr data-mid="/m/01sh2" class="kno-nf-nr" role="listitem"><td class="ellip"><span style="font-weight: bold;">Total Carbohydrate</span> <span class="abs">)" + N(1005) +
    R"( g</span></td><td style="text-align: right;"><span class="pdv">)" + N_R_MAX(1005) + R"(%</span></td></tr><tr data-mid="/m/0hkwr" class="kno-nf-nr" role="listitem"><td style="text-indent: 30px;"><span>Dietary fiber</span> <span class="abs">)" + N(1079) + R"( g</span></td><td style="text-align: right;"><span class="pdv">)" + N_R(1079) + R"(%</span></td></tr><tr data-mid="/m/06x4c" class="kno-nf-nr" role="listitem"><td style="text-indent: 30px;"><span>Sugar</span> <span class="abs">)" + N(2000) + R"( g</span></td><td class="ellip fooDZe"><span class="pdv"></span></td></tr><tr data-mid="/m/05wvs" class="kno-nf-nr" role="listitem"><td style="font-weight: bold;"><span class="V6Ytv">Protein</span> <span class="abs">)" + N(1003) +
    R"( g</span></td><td style="text-align: right;"><span class="pdv">)" + N_R(1003) + R"(%</span></td></tr><tr data-mid="/m/01_7l" style="display:none" class="kno-nf-nr" role="listitem"><td class="ellip"><span class="V6Ytv">Caffeine</span> <span class="abs"></span></td><td class="ellip fooDZe"><span class="pdv"></span></td></tr></table><table class="AYBNrd Gbo8Ne"><col><col style="width:4.6em"><col><col style="width:3.8em"><tr><td class="ellip" role="listitem">)" + N_N2(0) + R"(</td><td style="text-align:right;"  data-mid="/m/02p0tjr"><span title=")" + N2(0) + R"( IU" class="pdv">)" + N_R2(0) +
    R"(%</span></td><td style="width: 100px;" role="listitem">)" + N_N2(1) + R"(</td><td style="text-align:right;"  data-mid="/m/07zqy"><span title=")" + N2(1) + R"( mg" class="pdv">)" + N_R2(1) + R"(%</span></td></tr><tr><td class="ellip" role="listitem">)" + N_N2(2) + R"(</td><td style="text-align:right;" data-mid="/m/025tkqy"><span title=")" + N2(2) + R"( mg" class="pdv">)" + N_R2(2) +
    R"(%</span></td><td class="ellip" role="listitem">)" + N_N2(3) + R"(</td><td style="text-align:right;"  data-mid="/m/025rw19"><span title=")" + N2(3) + R"( mg" class="pdv">)" + N_R2(3) +
    R"(%</span></td></tr><tr><td class="ellip" role="listitem">)" + N_N2(4) + R"(</td><td style="text-align:right;"  data-mid="/m/0f4jp"><span title=")" + N2(4) + R"( IU" class="pdv">)" + N_R2(4) + R"(%</span></td><td class="ellip" role="listitem">)" + N_N2(5) + R"(</td><td style="text-align:right;" data-mid="/m/0f4kp"><span title=")" + N2(5) + R"( mg" class="pdv">)" + N_R2(5) + R"(%</span></td></tr><tr><td class="ellip" role="listitem">)" + N_N2(6) + R"(</td><td style="text-align:right;" data-mid="/m/0f4k5"><span title=")" + N2(6) + R"( µg" class="pdv">)" + N_R2(6) + R"(%</span></td><td class="ellip" role="listitem">)" + N_N2(7) + R"(</td><td style="text-align:right;" data-mid="/m/025s0s0"><span title=")" + N2(7) + R"( mg" class="pdv">)" + N_R2(7) + R"(%</span></td></tr></table>
    
        )";
        
        res.set_content(render_text, "text/html; charset=utf-8");
    });

#undef NAME
#undef N
    
    int port = atoi(argv[1]);
    
    svr.listen(NULL, port, AI_PASSIVE);
    
/*    std::vector<std::pair<int, float> > ratios = {
        {1003, 60}, // Protein
        {1004, 10},  // Fat
        {1005, 5},  // Carbs
        {1079, 10}  // Fiber
    };
    
    std::vector<int> result;
    
    fds.search_by_ratio(ratios, result);*/
    
	return 0;
}

