/*
*       balanced_diet.h
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

#ifndef _balanced_diet_h_included_
#define _balanced_diet_h_included_

namespace balanced_diet
{

// Wrappers above float for stricker type checking
struct float_gram
{
    float v_;
    
    float_gram() : v_(0) {}
    explicit float_gram(float a) : v_(a) {}
};
struct float_nutrient_unit
{
    float v_;
    
    float_nutrient_unit() : v_(0) {}
    explicit float_nutrient_unit(float a) : v_(a) {}
};
struct float_nutrient_unit_per_100g
{
    float v_;
    
    float_nutrient_unit_per_100g() : v_(0) {}
    explicit float_nutrient_unit_per_100g(float a) : v_(a) {}
};
struct float_nutrient_unit_per_g
{
    float v_;
    
    float_nutrient_unit_per_g() : v_(0) {}
    explicit float_nutrient_unit_per_g(float a) : v_(a) {}
};

float_gram operator+(float_gram a, float_gram b);
float_gram operator-(float_gram a, float_gram b);
float_gram operator*(float_gram a, float b);
float_gram operator*(float a, float_gram b);
float_nutrient_unit operator*(float_gram a, float_nutrient_unit_per_g b);
float_nutrient_unit operator*(float_nutrient_unit_per_g a, float_gram b);
float_gram operator/(float_gram a, float b);

float_nutrient_unit operator+(float_nutrient_unit a, float_nutrient_unit b);
float_nutrient_unit operator-(float_nutrient_unit a, float_nutrient_unit b);
float_nutrient_unit operator*(float_nutrient_unit a, float b);
float_nutrient_unit operator*(float a, float_nutrient_unit b);
float_nutrient_unit operator/(float_nutrient_unit a, float b);
float_nutrient_unit_per_g operator/(float_nutrient_unit a, float_gram b);

void lower_str(const std::string &str, std::string &result);
std::string &json_escape(const std::string &str, std::string &result);
void divide_into_words(const std::string &str, std::vector<std::string> &words);
void divide_into_words_lc(const std::string &str, std::vector<std::string> &words);

// Item from the food catalog
struct food_catalogue_item
{
    // Name - for autocomletion and to show
    std::string catalogue_item_name_;
    
    /*
     *      Search criteria - it's only for uploading
     */
    
    // Filter by including strings
    std::string search_string_include_;
    
    // Filter by excluding strings
    std::string search_string_exclude_;
    
    // Filter by nutrients
    std::vector<std::pair<int, float> > search_nutrient_lower_limits_;
    std::vector<std::pair<int, float> > search_nutrient_upper_limits_;

    // Filter by inc and exc exact strings
    std::vector<std::string> search_string_include_exact_;
    std::vector<std::string> search_string_exclude_exact_;
    
    // Add exact food its to the catalogue
    std::vector<int> exact_food_ids_;

    // Example food if for this catalog to get a picture from
    int example_food_id_;
};

class recipes;

class foods
{
public:
    
    // Data that was added to a food on top of USDA
    struct food_augmented_data
    {
        // Weight of a standard portion in grams
        float standard_weight_;
        
        // Description of a standard unit of a food (1 piece, 1 serving, 1/2 cup, 1 tsp, 1 tbsp)
        const char *standard_unit_;

        // Popularity group (Walmart, Fast food)
        const char *popularity_group_;

        // Short name of the food - for convenience (can be empty)
        std::string food_short_name_;
    };
    
    // Data that was added to a nutrient on top of USDA
    struct nutrient_augmented_data
    {
        // A minumum recommended amount of intake or -1 if there is no such a norm
        float min_recommended_amount_;
        
        // A maximum recommended amount of intake or -1 if there is no such a norm
        float max_recommended_amount_;
        
        // Which kind of foods contain this nutrient
        std::string foods_contain_this_;
        
        // A short description - why we need this nutrient
        std::string why_we_need_this_;
        
        // Nutrient short name
        std::string nutrient_short_name_;
        
        // Nutrient middle name
        std::string nutrient_middle_name_;
    };
    
    food_catalogue_item xxx_;

    struct food
    {
        std::string food_name_;
        std::string food_category_;
        
        // The description of a standard unit of a food
        std::string food_standard_unit_;
        // The weight of a standard unit of a food
        float food_standard_weight_ = 100;

        // The catalog entry id (or -1 if this food is not catalogized yet)
        int food_catalogue_item_id_ = -1;
        
        // Nutrients of this food
        // first - nutrient id
        // second - amount of nutrient in grams per 100 grams
        // The vector is sorted by first
        std::vector<std::pair<int, float> > nutrients_;
        std::unordered_map<int, float> nutrients_hash_;
        
        // Nutrients and percent of daily norm of each sorted by decrease of
        //  its daily norm
        std::vector<std::pair<int, float> > nutrients_sorted_daily_norm_;

        float fast_access_prot_;
        float fast_access_cal_;
        //float fast_access_cal_lt_2000_grams_;
        //float fast_access_prot_gt_100_grams_;
        //float fast_access_prot_gt_100_cal_lt_2000_rank_;
        
        float *get_nutrient_amount(int nutrient_id)
        {
            auto l = std::lower_bound(nutrients_.begin(), nutrients_.end(),
                                      nutrient_id,
                             [](const std::pair<int, float> &a,
                                     int b) {
                             return a.first < b;
            });
            if (l == nutrients_.end() || l->first != nutrient_id)
                return NULL;
            else
                return &l->second;
        }
        
        float get_nutrient_amount2(int nutrient_id)
        {
            float *f = get_nutrient_amount(nutrient_id);
            return f ? *f : 0;
        }

        float get_nutrient_amount_fast(int nutrient_id)
        {
            switch (nutrient_id)
            {
                case 1003: return fast_access_prot_;
                case 1008: return fast_access_cal_;
            }
            auto l = nutrients_hash_.find(nutrient_id);
            if (l == nutrients_hash_.end())
                return 0;
            else
                return l->second;
        }
    };
    
    foods();
    
    struct nutrient
    {
        std::string nutrient_name_;
        std::string nutrient_unit_name_;
    };

    void upload_food(int food_id, const std::string &food_name, const std::string &food_category);
    void upload_nutrient(int nutrient_id,
                         const std::string &nutrient_name,
                         const std::string &nutrient_unit_name);
    void upload_food_nutrient(int food_id, int nutrient_id, float amount);
    void sort_nutrients();
    void postupload_steps();
    
    // Returns foods close to specified ratios of nutrients
    // In ratois: first - nutrient id, second - ratio of this nutrient
    void search_by_ratio(std::vector<std::pair<int, float> > &ratios,
                            std::vector<std::pair<int, float> > &filters,
                            std::vector<std::string> &search_string_include,
                            std::vector<std::string> &search_string_exclude,
                            int limit_match_coefficient,
                            std::ostream &output,
                            bool is_json,
                            int nutrient_to_sort,
                            int limit_after_sort);

    float get_food_rank_with_current_values2(food *f,
                              std::vector<std::pair<int, float> > &good_nutrients,
                              std::vector<std::pair<int, float> > &bad_nutrients,
                                std::unordered_map<int, float> &nutrient_current_values,
                               float *good_grams,
                               float *bad_grams,
                            float *amount_to_reach_quota,
                                                   float hard_limit_K,
                                                   float weight_limit_K,
                                                   float total_meal_weight,
                                                    float total_meal_weight_limit);
    
    float get_food_rank_with_current_values(food *f,
                              std::vector<std::pair<int, float> > &good_nutrients,
                              std::vector<std::pair<int, float> > &bad_nutrients,
                                std::unordered_map<int, float> &nutrient_current_values,
                               float *good_grams,
                               float *bad_grams,
                                            float *amount_to_reach_quota,
                                            float hard_limit_K,
                                            float weight_limit_K,
                                            float total_meal_weight,
                                            float total_meal_weight_limit);

    void auto_balance(std::vector<std::pair<int, float> > &foods_to_balance,
                             std::vector<std::pair<int, float> > &good_nutrients,
                             std::vector<std::pair<int, float> > &bad_nutrients,
                      recipes *rec,
                                float hard_limit_K,
                      float weight_limit_K,
                      const std::vector<std::string> &healthy_search_words);

    // Balances the meal
    //  foods           -   a list of foods that a person desires to it along with
    //                          desired amounts in grams per each food
    //  good_nutrients  -   a list of good nutrients (like protein or fiber) that a person
    //                          wants to consume AT LEAST as it's specified in this vector
    //  bad_nutrients   -   a list of bad nutritnes (like energy or sodium) that a person
    //                          wants to consume NO MORE than it's specified in this vector
    // As a result it prints on the output:
    //  1.  The input data
    //  2.  Actual amount per each nutrient and its difference with the desired value
    //  3.  Suggested ammendments per each food to fit good and bad nutrients within given
    //          limits
    //  4.  Resulting amount per each nutrient and its difference with the desired value
    void balance(std::vector<std::pair<int, float> > &foods_to_balance,
                 std::vector<std::pair<int, float> > &good_nutrients,
                 std::vector<std::pair<int, float> > &bad_nutrients,
                 std::vector<std::pair<std::string, std::vector<int> > > &groups,
                 std::vector<int> &mr_data,
                 const std::string &current_meal_plan_name,
                 recipes *rec,
                 int target_weight,
                 int target_weight_deadline,
                 bool is_imperial,
                 bool add_healthy_food,
                 const std::vector<std::string> &healthy_search_words,
                 std::ostream &output);

    // Balances the meal based on what a user did on sliders
    void balance_simple(std::vector<std::pair<int, float> > &food_current_values,
                        std::vector<int> &checked_foods,
                               std::vector<std::pair<int, float> > &nutrient_current_values,
                               int nutrient_id,
                               float nutrient_old_value,
                               float nutrient_new_value,
                            std::vector<std::pair<std::string, std::vector<int> > > &groups,
                            std::vector<int> &mr_data,
                            recipes *rec,
                        int target_weight,
                        int target_weight_deadline,
                        bool add_healthy_food,
                               std::ostream &output);

    // Another algorithm for the food rank
    float get_food_rank_by_sum(food *f,
                              std::vector<std::pair<int, float> > &good_nutrients,
                              std::vector<std::pair<int, float> > &bad_nutrients,
                               float *good_grams,
                                      float *bad_grams);
    
    // Returns the rank of the food against the specified nutrients
    // The rank is good grams divided by bad grams, the less the better
    // The ideal rank is zero - means that bad grams is zero
    // The meaning of ranks close to zero is either of these
    //  a) eat whatever amount you want - almost no limits because of very little amount of bad nutrients
    //  b) eat just a little bit - and you're good on good nutrients - so you don't have to eat more
    // Bad grams is how much you maximum can eat of this food to fit the specified limits for
    //  bad nutrients - the more the better
    // Good grams is how much you have to each of this food to fulfill your quota for at
    //  least one good nutrient - the less the better
    // Note: the meaning for second if good and bad nutrients is different:
    //  good: how much we have to have of this nutrient to reach quota
    //  bad: total limit per a period of time (say, per day)
    float get_food_rank(food *f,
                        std::vector<std::pair<int, float> > &good_nutrients,
                        std::vector<std::pair<int, float> > &bad_nutrients,
                        float *good_grams,
                        float *bad_grams);

    // The same as above, but all good and bad are in one vector
    //  sign of second '+' for goods and sign of second '-' for bads
    //  also there is a sign for first then means if the nutrient is
    //  above limits or not - we just have to abs it and ignore
    float get_food_rank(food *f,
                        std::vector<std::pair<int, float> > &nutrients,
                        float *good_grams,
                        float *bad_grams);
    
    // Returns the satiety of food per calorie based on the amount of protein, fiber, nutrients, vitamins
    float get_food_satiety_per_calorie(food *f);

	// Returns the distance between two foods
    // If common_nutrients is not NULL then place there a list of nutriets with similar values
	float distance_between_foods(int food_id_a, int food_id_b, recipes *rec,
                                 std::vector<int> *common_nutrients,
                                 std::vector<int> *divergent_nutrients);

    // Filters one food from the search
    bool food_rank_json_filter_food(food *f,
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
                                    std::string &lower_food_category_temp_);

    // Outputs one food
    void food_rank_json_output_food(food *f,
                                    int food_id,
                                    std::vector<std::pair<int, float> > &good_nutrients,
                                    std::vector<std::pair<int, float> > &bad_nutrients,
                                    int show_distance_to,
                                    recipes *rec,
                                    bool *b_first,
                                    std::ostream &output);
    
    // Output sorted JSON with food and ranks
    // Note: we need min_food_rank to exclude food with strange data - too good to be true :-)
    // Note: if popularity_group_first is not NULL and is not empty then it first shows foods
    //  from this popularity group and then other foods, also
    //  if popularity_group_first is empty then it first shows foods from popularity groups
    //  and then foods without a popularity group
    void food_rank_json(std::vector<std::pair<int, float> > &good_nutrients,
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
                        
			std::ostream &output);
    
    // Returns details on foods and nutrients
    void get_food_details(std::vector<int> &food_list,
                          std::vector<int> &nutrient_list,
                          std::ostream &output);
    
    
    // Returns json with suggestions for a word
    void suggest_words_json(const std::string &search_string,
                      std::ostream &output);
    
    food *get_food(int food_id)
    {
        if (food_id < 0 || food_id >= all_foods_.size())
            return NULL;
        else
            return &all_foods_[food_id];
    }

    nutrient *get_nutrient(int nutrient_id)
    {
        if (nutrient_id < 0 || nutrient_id >= all_nutrients_.size())
            return NULL;
        else
            return &all_nutrients_[nutrient_id];
    }
    
    std::string get_short_nutrient_name(int nutrient_id)
    {
        auto it = nutrient_augmented_data_.find(nutrient_id);
        if (it != nutrient_augmented_data_.end())
            return it->second.nutrient_short_name_;
        else
            return "";
    }
    std::string get_nutrient_middle_name(int nutrient_id)
    {
        auto it = nutrient_augmented_data_.find(nutrient_id);
        if (it != nutrient_augmented_data_.end())
            return it->second.nutrient_middle_name_;
        else
            return "";
    }

    food_catalogue_item *get_food_catalogue_item(int food_catalogue_item_id)
    {
        if (food_catalogue_item_id < 0 || food_catalogue_item_id >= food_catalogue_.size())
            return NULL;
        else
            return &food_catalogue_[food_catalogue_item_id];
    }
    
    // Save a recipe as a food
    void save_recipe(const std::string &name,
                     std::vector<std::pair<int, float> > &foods_to_balance);
    
    void nutrients_to_json(std::ostream &output);
    void catalogue_to_json(std::ostream &output);
    
    // Returns the recommended amount of a nutrient per day
    float get_nutrient_recommended_min_amout(int nutrient_id)
    {
        auto it = nutrient_augmented_data_.find(nutrient_id);
        if (it != nutrient_augmented_data_.end())
            return it->second.min_recommended_amount_;
        else
            return 0;
    }

    // Returns the maximum recommended amount of a nutrient per day
    float get_nutrient_recommended_max_amout(int nutrient_id)
    {
        auto it = nutrient_augmented_data_.find(nutrient_id);
        if (it != nutrient_augmented_data_.end())
            return it->second.max_recommended_amount_;
        else
            return 0;
    }
    
    // Returns the nutrient augmented data
    nutrient_augmented_data *get_nutrient_augmented_data(int nutrient_id)
    {
        auto it = nutrient_augmented_data_.find(nutrient_id);
        if (it != nutrient_augmented_data_.end())
            return &it->second;
        else
            return NULL;
    }
    
    // Returns standard food unit size in grams - like weight of a Big Mac or of an extra large egg
    float get_food_aug_standard_weight(int food_id)
    {
        auto i = food_augmented_data_.find(food_id);
        if (i == food_augmented_data_.end())
        {
            food *f = get_food(food_id);
            if (f && !f->food_name_.empty())
                return f->food_standard_weight_;
            else
                return 100;
        }
        else
            return i->second.standard_weight_ ? i->second.standard_weight_ : 100;
    }

    // Returns the popolarity group of the food
    const char *get_food_aug_popularity_group(int food_id)
    {
        auto i = food_augmented_data_.find(food_id);
        if (i == food_augmented_data_.end())
            return NULL;
        else
            return i->second.popularity_group_;
    }
    std::string get_food_aug_standard_unit(int food_id)
    {
        auto i = food_augmented_data_.find(food_id);
        if (i == food_augmented_data_.end())
        {
            food *f = get_food(food_id);
            if (f && !f->food_name_.empty())
                return f->food_standard_unit_;
            else
                return "";
        }
        else
            return i->second.standard_unit_;
    }
    std::string *get_food_aug_short_name(int food_id)
    {
        auto i = food_augmented_data_.find(food_id);
        if (i == food_augmented_data_.end())
            return NULL;
        else
            return &i->second.food_short_name_;
    }

private:

    void construct_short_nutrient_info_helper_item(foods::food *f,
                                                   float nutrient_amount,
                                                   int nutrient_id,
                                                   int color,
                                                          std::string &result);

    void construct_short_nutrient_info_helper(foods::food *f,
                                                    int food_id,
                                                  float food_amount,
                                                  std::vector<std::pair<int, float> > &good_nutrients,
                                                  std::vector<std::pair<int, float> > &bad_nutrients,
                                                     std::string &result);
    
    bool balance_render_result_helper(std::vector<std::pair<int, float> > &foods_to_balance,
                                            std::vector<std::pair<int, float> > &good_nutrients,
                                            std::vector<std::pair<int, float> > &bad_nutrients,
                                            std::vector<int> &problematic_good_nutrients,
                                            std::vector<int> &problematic_bad_nutrients,
                                            std::ostream &output,
                                            bool is_after_amendments,
                                            bool is_silent,
                                        recipes *rec);
    
    // Returns the amount of specified nutrient the a person intakes if she/he
    //  consumes specified amount of grams of specified foods
    float nutrient_comsumption_helper(std::vector<std::pair<int, float> > &foods_to_balance,
                                      int nutrient_id,
                                      recipes *rec);
    
    // List of all foods
    // An index to this vector is a food id
    std::vector<food> all_foods_;

    // List of all nutrients
    // An index to this vector is a nutrient id
    std::vector<nutrient> all_nutrients_;
    
    // Cache of sorted IDs for different rank paramaters
    std::unordered_map<std::string, std::vector<int> > sorted_ids_cache_;

    // Recomended amount of each nutrient - for default limits
    std::unordered_map<int, nutrient_augmented_data> nutrient_augmented_data_;

    // Knows weights of foods
    std::unordered_map<int, food_augmented_data> food_augmented_data_;
    
    // Food catalogue
    std::vector<food_catalogue_item> food_catalogue_;
    
    // All words from all foods and nutrient names, lower cased
    std::vector<std::string> all_food_words_;
    std::vector<std::pair<std::string, int> > all_nutrient_words_;
};

}

#endif
