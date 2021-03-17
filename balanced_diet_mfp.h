/*
*       balanced_diet_mfp.h
*
*       (C) Denis Anikin 2020
*
*       Headers for the balanced diet
*
*/

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#ifndef _balanced_diet_mfp_h_included_
#define _balanced_diet_mfp_h_included_

namespace balanced_diet
{

// Per user stat - a user of MyFitnessPal
struct mfp_user
{
    // Total calories eaten by a user for all days
    float user_total_fact_calories = 0;
    
    // Total goal of a user for al days
    float user_total_goal_calories = 0;
    
    // One day per this user
    struct day
    {
        std::string date_;
        float total_fact_calories_;
        float total_goal_calories_;
        
        // All dish ids per that day of the user
        std::vector<int> dish_ids_;
    };
    
    std::vector<day> days_;
    std::unordered_set<int> appeared_dish_ids_, disappeared_dish_ids_;
};

// Satiety of dishes
// Note: it's aggregated by dish names
struct dish_satiety_info
{
    float influence_ = 0;
    int cardinality_ = 0;

    float cal_ = 0;
    float prot_ = 0;
    float fat_ = 0;
    float fiber_ = 0;
    float carbs_ = 0;
    float sodium_ = 0;
    float sugar_ = 0;
    float potass_ = 0;

    float prot_cal_ratio_ = 0;
    float fat_cal_ratio_ = 0;
    float fiber_cal_ratio_ = 0;
    float sugar_cal_ratio_ = 0;
    float sodium_cal_ratio_ = 0;
    float potass_cal_ratio_ = 0;
    
    float meal_pe_ = 0; // Average PE of meals with this dish
    float meal_fe_ = 0; // Average FE of meals with this dish
    float meal_fbe_ = 0; // Average FE of meals with this dish
    int num_meals_ = 0; // Number of meals with this dish
    int n_dish_good_days_ = 0; // Total good days with this dish
    int n_dish_bad_days_ = 0; // Total bad days with this dish
    int n_total_good_users_ = 0; // Total good users who had this dish
    int n_total_bad_users_ = 0; // Total bad users who has this dish
    // UID -> food influence, number of meals with this food
    std::unordered_map<int, std::pair<float, int> > user_influence_;
    std::string dish_name_;
    
    // All day meal ids which includes this dish
    // Note: we fill this vector only for popular dishes that show up in the search
    //  in order to save RAM
    std::vector<int> day_meal_ids_;
    
    void add(int uid, float influence,
             float cal, float prot, float fat, float fiber, float carbs, float sodium, float sugar, float potass);
    void add_meal_xe(float pe, float fe, float fbe);
    void make_avg(const std::vector<mfp_user> &mfp_users);
    
}; // dish_satiety_info

// A daily meal from a mfp user
struct mfp_day_meal
{
    // Fact and goal calories for this day
    float total_fact_calories_ = 0;
    float total_goal_calories_ = 0;
    
    // All dish ids for this meal
    std::vector<int> day_meal_dish_ids_;
};

// Foods from MyFitnessPal open diary log data
class mfp_foods
{
public:

    // Suggests foods from the mfp stat
    // Foods are sorted in accordance to bad gays divided by good days
    void suggest_food_from_stat(std::vector<std::string> &search_string_include,
                                std::vector<std::string> &search_string_exclude,
                            std::vector<std::string> &search_string_include_exact,
                            std::vector<std::string> &search_string_exclude_exact,
                                int limit_after_sort,
                                float min_food_rank,
                                float max_food_rank,
                            float protein_to_calorie_percent,
                            float fiber_to_calorie_percent,
                            float min_pfind,
                std::ostream &output);
    
    // Outputs all meals with this dish sorted by goal overfulfillment
    void all_meals_with_dish(int dish_id,
                             const std::string &dish_name,
                             int limit_after_sort,
                             std::ostream &output);
    
    // Uploads the stat from mfp
    void upload_stat(std::istream &is);

private:
    
    // Outputs JSON with a dish
    void output_dish(int dish_id,
                     const dish_satiety_info &dish,
                     bool output_user_list,
                     int dish_cardinality,
                     std::ostream &output);
    
    // Returns the dish by its name
    dish_satiety_info &get_dish_by_name(const std::string &dish_name);
    
    // Returns the dish id by its name
    int get_dish_id_by_name(const std::string &dish_name);
    
    // All dishes along with its satiety info
    // Note: index to this vector is the dish ID
    std::vector<dish_satiety_info> all_dishes_;
    
    // dish name -> ID
    std::map<std::string, int> dish_name_to_id_;
    
    // Top of dish IDs sorted from most filling to least filling
    std::multimap<float, int> dish_satiety_top_;
    
    // All meals
    std::vector<mfp_day_meal> all_day_meals_;
    
    // All users with days and dish ids
    std::vector<mfp_user> mfp_users_;
};

}

#endif
