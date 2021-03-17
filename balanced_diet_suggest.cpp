/*
*       balanced_diet_suggest.cpp
*
*       (C) Denis Anikin 2020
*
*       Impl for the balanced suggest diet
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

/*
std::string foods::get_short_nutrient_name(int nutrient_id)
{
    switch (nutrient_id)
    {
        case 1003: return "Prt";
        case 1004: return "Fat";
        case 1005: return "Crb";
        case 1008: return "Cal";
        case 1099: return "Flo";
        case 1104:
        case 1106:
            return "VtA";
        case 1109: return "VtE";
        case 1110: return "VtD";
        case 1111: return "VD2";
        case 1112: return "VD3";
        case 1114: return "VtD";
        case 1124: return "VtE";
        case 1162: return "VtC";
        case 1166: return "VB2";
        case 1175: return "VB6";
        case 1176: return "VB7";
        case 1178: return "B12";
        case 1183:
        case 1184:
        case 1185:
            return "VtK";
        case 1242: return "VtE";
        case 1246: return "B12";
        case 1272: return "Om3";
        case 1278: return "Om3";
        case 1316: return "Om6";
        case 1404: return "Om3";
        default: return get_nutrient(nutrient_id)->nutrient_name_.substr(0,3);
    }
}

std::string foods::get_nutrient_middle_name(int nutrient_id)
{
    switch (nutrient_id)
    {
        case 1003: return "Protein";
        case 1004: return "Total Fat";
        case 1005: return "Total Carbohydrate";
        case 1008: return "Calories";
        case 1019: return "Alcohol";
        case 1079: return "Total Fiber";
        case 1087: return "Calcium";
        case 1089: return "Iron";
        case 1090: return "Magnesium";
        case 1091: return "Phosphorus";
        case 1092: return "Potassium";
        case 1093: return "Sodium";
        case 1095: return "Zinc";
        case 1096: return "Chromium";
        case 1098: return "Copper";
        case 1099: return "Fluoride";
        case 1100: return "Iodine";
        case 1101: return "Manganese";
        case 1102: return "Molybdenum";
        case 1103: return "Selenium";
        case 1104:
        case 1106:
            return "Vitamin A";
        case 1109: return "Vitamin E";
        case 1110: return "Vitamin D";
        case 1111: return "Vitamin D-2";
        case 1112: return "Vitamin D-3";
        case 1114: return "Vitamin D";
        case 1124: return "Vitamin E";
        case 1162: return "Vitamin C";
        case 1165: return "Vitamin B-1";
        case 1166: return "Vitamin B-2";
        case 1167: return "Vitamin B-3";
        case 1170: return "Vitamin B-5";
        case 1175: return "Vitamin B-6";
        case 1176: return "Vitamin B-7";
        case 1177: return "Folate";
        case 1178: return "Vitamin B-12";
        case 1180: return "Choline";
        case 1183:
        case 1184:
        case 1185:
            return "Vitamin K";
        case 1198: return "Betaine";
        case 1235: return "Added sugar";
        case 1242: return "Vitamin E";
        case 1246: return "Vitamin B-12";
        case 1253: return "Cholesterol";
        case 1257: return "Trans Fat";
        case 1258: return "Saturated fat";
        case 1268: return "Omega-9 18:1";
        case 1272: return "Omega-3 DHA";
        case 1278: return "Omega-3 EPA";
        case 1279: return "Omega-9 22:1";
        case 1292: return "Monunsaturated Fat";
        case 1293: return "Polyunsaturated Fat";
        case 1316: return "Omega-6";
        case 1404: return "Omega-3 ALA";
        case 2000: return "Sugar";
        default: return get_nutrient(nutrient_id)->nutrient_name_;
    }
}*/

struct weigh_data
{
    float in_calorie_intake_;
    float in_metabolism_;
    float in_protein_for_muscles_;
    
    float out_muscle_gain_;
    float out_muscle_loss_component_due_to_deficit_;
    float out_fat_gain_;
    float out_metabolism_;
    float out_calorie_surplus_;
    float out_energy_muscle_gain_;
};

void get_weight_data_slow(weigh_data &wd)
{
    // This is how many calories we need to burn to grow 1 gram of fat tissue and
    //  how many calories we release by burning 1 gram of fat tissue
    const float calorie_burn_by_gram_fat_tissue = 7.5;
    
    // This is how many calories we release by burning 1 gram of muscle tissue
    const float calorie_dispose_by_gram_muscle = 1.5;
    
    // This is the amount of protein that we need a day just to support muscle and for
    //  other needs without building it if we receive less then we just lose muscle.
    //const float protein_amount_to_support = 50;
    
    // This is our muscle tissue growth a day:
    //  protein_for_muscles - protein_amount_to_support_muscle
    // Note: it can be nagative
    
    // This is how many calories we need to build one gram of muscle tissue
    const float calorie_build_one_gram_muscle = 5;
    
    // This is how much of muscle mass we gain per gram of protein above protein_amount_to_support
    const float ratio_protein_to_muscle_mass = 5;
    
    // Ratio of the calorie deficit to take energy from fat
    const float ratio_deficit_burn_fat = 0.9;
    
    // How much enerty fat burns just for its existance
    const float fat_metabolism_per_gram = 5.5 / 1000.0;
    
    // How much energy muscle burns just for its existance
    const float muscle_metabolism_per_gram = 18 / 1000.0;

    // This is just calorie surplus
    //  Note: substruct protein calories from surplus because protein counts differently - see below
    wd.out_calorie_surplus_ = wd.in_calorie_intake_ - wd.in_metabolism_ - wd.in_protein_for_muscles_ * 4.0;

    //std::cerr << "calorie_surplus_no_protein=" << calorie_surplus_no_protein << std::endl;

    // We intake enough protein to build muscles
    // Note: wd.in_protein_for_muscles_ already includes amount to support, so it just has to be
    //  positive
    if (wd.in_protein_for_muscles_ > 0/*protein_amount_to_support*/)
    {
        // That's the maximum amount of muscle that we can build
        wd.out_muscle_gain_ = (wd.in_protein_for_muscles_ - 0/*protein_amount_to_support*/) / ratio_protein_to_muscle_mass;
            
        // It requires this energy
        wd.out_energy_muscle_gain_ = calorie_build_one_gram_muscle * wd.out_muscle_gain_;

            //std::cerr << "energy_to_muscle_gain=" << energy_to_muscle_gain << std::endl;
            
            // If it's not enough then build what we can
            // Note: ^^^ we now asume that it's always enough enerty to buid muscle - we will
            //  take it from fat if needed
            /*if (energy_to_muscle_gain > calorie_surplus_no_protein)
            {
                wd.out_calorie_surplus_ = 0;
                wd.out_muscle_gain_ = calorie_surplus_no_protein / calorie_build_one_gram_muscle;
            }
            else*/
        wd.out_calorie_surplus_ -= wd.out_energy_muscle_gain_;
    }
    // We don't take enough protein to build muscles
    else
    {
        wd.out_energy_muscle_gain_ = 0;
        
        // Losing muscles
        // Note: wd.in_protein_for_muscles_ already includes amount to support, so it just has to be
        //  positive
        wd.out_muscle_gain_ = wd.in_protein_for_muscles_ - 0/*protein_amount_to_support*/;
    }
        
    // We have energy deficit
    if (wd.out_calorie_surplus_ < 0)
    {
        // 90% of the deficit is used to burn fat
        wd.out_fat_gain_ = ratio_deficit_burn_fat * wd.out_calorie_surplus_ / calorie_burn_by_gram_fat_tissue;
        
        // 10% of the decifit is used to burn muscle (even though we built it above)
        wd.out_muscle_loss_component_due_to_deficit_ = (1.0 - ratio_deficit_burn_fat) * wd.out_calorie_surplus_ /
            calorie_dispose_by_gram_muscle;
        wd.out_muscle_gain_ += wd.out_muscle_loss_component_due_to_deficit_;
    }
    // We have enerty surplus - it all goes to fat
    else
    {
        wd.out_fat_gain_ = wd.out_calorie_surplus_ / calorie_burn_by_gram_fat_tissue;
        wd.out_muscle_loss_component_due_to_deficit_ = 0;
    }
    
    wd.out_metabolism_ = wd.in_metabolism_
                + wd.out_muscle_gain_ * muscle_metabolism_per_gram
                + wd.out_fat_gain_ * fat_metabolism_per_gram;
}

struct metabolism_data
{
    int in_mr_data_gender_;
    int in_mr_data_weight_;
    int in_mr_data_height_;
    int in_mr_data_age_;
    int in_mr_data_exercise_;
    int in_mr_data_body_type_;
    
    float out_basal_metabolic_rate_;
    float out_exercise_component_;
    float out_fat_component_;
    float out_protein_component_;
    float out_fat_component_correction_;
    float out_protein_component_correction_;
    
    float out_metabolic_rate_;
    float out_protein_percent_intake_;
    float out_protein_max_intake_;
};

void get_metabolic_rate(std::vector<int> &mr_data,
                            metabolism_data &md)
{
    if (mr_data.empty())
    {
        md.in_mr_data_gender_ = 1;
        md.in_mr_data_weight_ = 65;
        md.in_mr_data_height_ = 170;
        md.in_mr_data_age_ = 41;
        md.in_mr_data_exercise_ = 1;
        md.in_mr_data_body_type_ = 2;
    }
    else
    {
        md.in_mr_data_gender_ = (mr_data[0] == -1 ? 1 : mr_data[0]);
        md.in_mr_data_weight_ = (mr_data[1] == -1 ? 65 : mr_data[1]);
        md.in_mr_data_height_ = (mr_data[2] == -1 ? 170 : mr_data[2]);
        md.in_mr_data_age_ = (mr_data[3] == -1 ? 41 : mr_data[3]);
        md.in_mr_data_exercise_ = (mr_data[4] == -1 ? 1 : mr_data[4]);
        md.in_mr_data_body_type_ = (mr_data[5] == -1 ? 2 : mr_data[5]);
    }
    
    if (md.in_mr_data_gender_ == 0)
        md.out_basal_metabolic_rate_ = 66.4730 + (13.7516 * md.in_mr_data_weight_) +
            (5.0033 * md.in_mr_data_height_) - (6.7550 * md.in_mr_data_age_);
    else
        md.out_basal_metabolic_rate_ = 655.0955 + (9.5634 * md.in_mr_data_weight_) +
            (1.8496 * md.in_mr_data_height_) - (4.6756 * md.in_mr_data_age_);
    
    switch (md.in_mr_data_exercise_)
    {
        default:
        case 0:
            md.out_exercise_component_ = md.out_basal_metabolic_rate_ * 0.2;
            md.out_protein_percent_intake_ = 0.1;
            break;
        case 1: md.out_exercise_component_ = md.out_basal_metabolic_rate_ * 0.375;
            md.out_protein_percent_intake_ = 0.15;
            break;
        case 2: md.out_exercise_component_ = md.out_basal_metabolic_rate_ * 0.465;
            md.out_protein_percent_intake_ = 0.2;
            break;
        case 3: md.out_exercise_component_ = md.out_basal_metabolic_rate_ * 0.55;
            md.out_protein_percent_intake_ = 0.25;
            break;
        case 4: md.out_exercise_component_ = md.out_basal_metabolic_rate_ * 0.725;
            md.out_protein_percent_intake_ = 0.3;
            break;
        case 5: md.out_exercise_component_ = md.out_basal_metabolic_rate_ * 0.9;
            md.out_protein_percent_intake_ = 0.35;
            break;
    }

    switch (md.in_mr_data_body_type_)
    {
        case 0:
            if (md.in_mr_data_gender_ == 0)
            {
                md.out_fat_component_ = 0.035 * md.in_mr_data_weight_ * 5;
                md.out_protein_component_ = 0.25 * md.in_mr_data_weight_ * 18;
            }
            else
            {
                md.out_fat_component_ = 0.115 * md.in_mr_data_weight_ * 5;
                md.out_protein_component_ = 0.19 * md.in_mr_data_weight_ * 18;
            }
            break;
        case 1:
            if (md.in_mr_data_gender_ == 0)
            {
                md.out_fat_component_ = 0.095 * md.in_mr_data_weight_ * 5;
                md.out_protein_component_ = 0.20 * md.in_mr_data_weight_ * 18;
            }
            else
            {
                md.out_fat_component_ = 0.17 * md.in_mr_data_weight_ * 5;
                md.out_protein_component_ = 0.18 * md.in_mr_data_weight_ * 18;
            }
            break;
        case 2:
            if (md.in_mr_data_gender_ == 0)
            {
                md.out_fat_component_ = 0.155 * md.in_mr_data_weight_ * 5;
                md.out_protein_component_ = 0.175 * md.in_mr_data_weight_ * 18;
            }
            else
            {
                md.out_fat_component_ = 0.225 * md.in_mr_data_weight_ * 5;
                md.out_protein_component_ = 0.15 * md.in_mr_data_weight_ * 18;
            }
            break;
        default:
        case 3:
            if (md.in_mr_data_gender_ == 0)
            {
                md.out_fat_component_ = 0.215 * md.in_mr_data_weight_ * 5;
                md.out_protein_component_ = 0.15 * md.in_mr_data_weight_ * 18;
            }
            else
            {
                md.out_fat_component_ = 0.28 * md.in_mr_data_weight_ * 5;
                md.out_protein_component_ = 0.09 * md.in_mr_data_weight_ * 18;
            }
            break;
        case 4:
            if (md.in_mr_data_gender_ == 0)
            {
                md.out_fat_component_ = 0.25  * md.in_mr_data_weight_ * 5;
                md.out_protein_component_ = 0.1 * md.in_mr_data_weight_ * 18;
            }
            else
            {
                md.out_fat_component_ = 0.32 * md.in_mr_data_weight_ * 5;
                md.out_protein_component_ = 0.07 * md.in_mr_data_weight_ * 18;
            }
            break;
    }

    // Note: substruct average fat/protein percent * average weight from its value for each group because
    //  it's already counted in the basal metabolic rate by its formula
    //  88kg - average weight of a man
    //  75kg - average weight of a woman
    if (md.in_mr_data_gender_ == 0)
    {
        md.out_fat_component_correction_ = -0.215 * 88 * 5;
        md.out_protein_component_correction_ = -0.15 * 88 * 18;
    }
    else
    {
        md.out_fat_component_correction_ = -0.28 * 75 * 5;
        md.out_protein_component_correction_ = -0.09 * 75 * 18;
    }
    
    md.out_metabolic_rate_ = md.out_basal_metabolic_rate_ + md.out_exercise_component_ +
        md.out_fat_component_ +
        md.out_protein_component_ +
        md.out_fat_component_correction_ +
        md.out_protein_component_correction_;

    md.out_protein_max_intake_ = md.out_metabolic_rate_ * md.out_protein_percent_intake_ / 4;
}

float determine_calorie_need(float calorie_intake,
                             float protein_for_muscles,
                             float total_thermic_effect,

                             float carbs_intake,
                             float alcohol_intake,
                             float protein_intake,
                             
                             const metabolism_data &md)
{
    if (!calorie_intake)
        return 0;
    
    float calorie_need;
    for (int i = 0;i < 3; ++i)
    {
        float denominator1 = calorie_intake - protein_for_muscles * 4 - total_thermic_effect;
        if (denominator1 < 0.1)
            denominator1 = 0.1;
        float denominator2 = calorie_intake - total_thermic_effect;
        if (denominator2 < 0.1)
            denominator2 = 0.1;
        calorie_need = 0.5 * md.out_metabolic_rate_ * calorie_intake / denominator1
        +
        0.5 * (md.out_metabolic_rate_ +  protein_for_muscles * 4) * calorie_intake / denominator2;
    
        //std::cerr << "i=" << i << ", calorie_need = " << calorie_need << std::endl;
    
        float ratio = calorie_need/calorie_intake;
    
        calorie_intake = calorie_need;
        carbs_intake = carbs_intake * ratio;
        alcohol_intake = alcohol_intake * ratio;
        protein_intake = protein_intake * ratio;
    
        protein_for_muscles = std::fmin(protein_intake, md.out_protein_max_intake_) - 50;
        float excess_protein = protein_intake - (protein_for_muscles + 50);
        total_thermic_effect = carbs_intake * 4 * 0.1 + excess_protein * 4 * 0.2 + alcohol_intake * 4 * 0.2;
    }
    
    return calorie_need;
}

void foods::balance_simple(std::vector<std::pair<int, float> > &food_current_values,
                           std::vector<int> &checked_foods,
                            std::vector<std::pair<int, float> > &nutrient_reference_values,
                            int nutrient_id,
                            float nutrient_old_value,
                            float nutrient_new_value,
                           std::vector<std::pair<std::string, std::vector<int> > > &groups,
                           std::vector<int> &mr_data,
                           recipes *rec,
                           int target_weight,
                           int target_weight_deadline,
                           bool add_healthy_food,
                            std::ostream &output)
{
    // Determine metabolic rate
    metabolism_data md;
    get_metabolic_rate(mr_data, md);
    
    std::vector<std::pair<int, float> > good_nutrients;
    std::vector<std::pair<int, float> > bad_nutrients;
    
    for (auto &nr_i : nutrient_reference_values)
    {
        if (nr_i.second > 0)
            good_nutrients.push_back({std::abs(nr_i.first), nr_i.second});
        else
            bad_nutrients.push_back({std::abs(nr_i.first), -nr_i.second});
    }

    // Add healthy food to the balanced food
    if (add_healthy_food)
        auto_balance(food_current_values, good_nutrients, bad_nutrients, rec, 2.0, 4.0, {});
    
    
    // Note: nutrient_reference_values contains pair of nutrient ids and reference values
    //          There can be three states of a nurient:
    //          a) A nutrient within limits - then first > 0 && second > 0
    //          b) A nutrient is not within limits and it's a good one - then
    //              first < 0 && second > 0
    //          c) A nutrient is not within limits and it's a bad one - then
    //              first < 0 && second < 0

    output << "{\"event\":\"balance_simple\",\"data\":[" << std::endl;
    bool is_first = true;
    if (nutrient_id)
    {
        float K = nutrient_new_value / nutrient_old_value;
        
        if (checked_foods.empty())
        {
            // Change amount of all foods based on the nutrient diff
            for (auto &x : food_current_values)
                x.second *= K;
        }
        else
        {
            // If there are checked foods then try to balance without changing them
            // Determine the total nutrient value for all checked foods and unckecked ones
            float total_nutrient_value_checked = 0;
            float total_nutrient_value_unchecked = 0;

            std::set<int> temp_checked_foods;
            for (auto &x : checked_foods)
            {
                int food_id = x;
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
                
                temp_checked_foods.insert(food_id);
            }
            
            for (auto &x : food_current_values)
            {
                int food_id = x.first;
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
                
                if (temp_checked_foods.find(x.first) != temp_checked_foods.end())
                    total_nutrient_value_checked += x.second * f->get_nutrient_amount2(nutrient_id);
                else
                    total_nutrient_value_unchecked += x.second * f->get_nutrient_amount2(nutrient_id);
            }
            
            //std::cerr << "nutrient_new_value=" << nutrient_new_value << ", total_nutrient_value_checked="
            //<< total_nutrient_value_checked << ", total_nutrient_value_unchecked=" << total_nutrient_value_unchecked << std::endl;

            // If we have not to much of nutrient in all checked food and have at least some amount in the unchecked food
            // then change amounts of the unchecked food to
            //  reach nutrient_new_value in the unchecked food + checked food (i.e. reach amount of
            //  nutrient_new_value - total_nutrient_value_checked in the unchecked food)
            if (nutrient_new_value * 100 >= total_nutrient_value_checked && total_nutrient_value_unchecked)
            {
                // Cap the increase the amount of unchecked food more by 5 times because
                //  it will sound unrealistic for the user to agree with that dramatic change
                float K_for_unchecked = std::fmin(
                            (nutrient_new_value * 100 - total_nutrient_value_checked) / total_nutrient_value_unchecked, 5.0);
                for (auto &x : food_current_values)
                {
                    if (temp_checked_foods.find(x.first) != temp_checked_foods.end())
                        continue;
                    x.second = x.second * K_for_unchecked;
                }
            }
            // else do nothing because we already have to much of a nutrient in the checked food
        }
    }
    
    // Temp map of food values - for groups
    std::unordered_map<int, float> food_values_temp;

    // Output the food
    // Note: we output the food anyway even it's not changed - for consistency
    for (auto &x : food_current_values)
    {
        int food_id = x.first;
        food *f = NULL;
        
        // Get the food either from the recipes or from normal foods
        if (rec)
        {
            f = rec->get_food(food_id);
            if (!f)
                f = get_food(food_id);
        }
        else
            f = get_food(food_id);
        
        if (!is_first) output << "," << std::endl; is_first = false;
        output << "{\"food_id\":" << x.first
        << ",\"food_value\":" << x.second
        << ",\"food_cal\":" << x.second * f->get_nutrient_amount2(1008) / 100.0
        << ",\"food_cal_exc_protein\":" << x.second *
        (f->get_nutrient_amount2(1008) - f->get_nutrient_amount2(1003) * 4) / 100.0;
        
        
        // Output sCal and the best percent
        float prot = f->get_nutrient_amount2(1003);
        float fiber = f->get_nutrient_amount2(1079);
        float cal = f->get_nutrient_amount2(1008);
        float sCal = (cal - (prot + fiber)  * 12.0) * x.second / 100;
        output << ",\"food_scal\":\"<span style=\\\"color: " << (sCal<=0?"green":"red") << ";\\\">" << (int)sCal << "</span>\"";
        output << ",\"food_best_nurient\":\"" << get_short_nutrient_name(f->nutrients_sorted_daily_norm_[0].first) << " " << (int)(f->nutrients_sorted_daily_norm_[0].second * x.second) << "\"";
        
        output << "}";
        
        food_values_temp[x.first] = x.second;
    }
    output << std::endl;

    // Output meal group aggregated values
    for (auto &mf : groups)
    {
        if (!is_first) output << "," << std::endl; is_first = false;
        
        float value = 0;
        for (auto &x : mf.second)
            value += food_values_temp[x];
        
        output << "{\"meal_group_name\":\"" << mf.first << "\",\"meal_group_value\":" << value << "}" << std::endl;
    }
    
    float calorie_intake = nutrient_comsumption_helper(food_current_values, 1008, rec);
    float protein_intake = nutrient_comsumption_helper(food_current_values, 1003, rec);

    // Note: don't count first 50g of protein because it's already counted in BMR
    float protein_for_muscles = std::fmin(protein_intake, md.out_protein_max_intake_) - 50;
    float excess_protein = protein_intake - (protein_for_muscles + 50);
    if (protein_for_muscles < 0)
        protein_for_muscles = 0;
    float alcohol_intake = nutrient_comsumption_helper(food_current_values, 1018, rec);
    float fat_intake = nutrient_comsumption_helper(food_current_values, 1004, rec);
    float carbs_intake = nutrient_comsumption_helper(food_current_values, 1005, rec);
    float sugar_intake = nutrient_comsumption_helper(food_current_values, 2000, rec);
    float exess_sodium_intake = nutrient_comsumption_helper(food_current_values, 1093, rec) -
        2300;

    // Get nutrient values based on food values
    // Note: if foods were changed based on the nutrient diff then nutrients can and will be changed
    //  again, including the one that was in the diff
    for (auto &nr_i : nutrient_reference_values)
    {
        float nutrient_value = nutrient_comsumption_helper(food_current_values, std::abs(nr_i.first), rec);
        if (!is_first) output << "," << std::endl; is_first = false;
        output << "{\"nutrient_id\":" << std::abs(nr_i.first) << ",\"nutrient_value\":" << nutrient_value << "}";
        
        // Update the sign of nutrient's id - it can get back to normal or vice versa
        if (nr_i.second > 0)
        {
            if (nutrient_value > nr_i.second )
                nr_i.first = std::abs(nr_i.first);
            else
                nr_i.first = -std::abs(nr_i.first);
        }
        if (nr_i.second < 0)
        {
            if (nutrient_value < -nr_i.second )
                nr_i.first = std::abs(nr_i.first);
            else
                nr_i.first = -std::abs(nr_i.first);
        }
    }
    
    // Output pair of foods and nutrients
    std::string result, escaped_result;
    for (auto &nr_i : nutrient_reference_values)
    {
        // Find the food with the maximum amount of this nutrient
        float maximum_nutrient_amount = 0;
        food *food_with_max = NULL;
        int food_id_with_max = -1;
        
        int nutrient_id = std::abs(nr_i.first);

        // Only search for the maximum food for nutrients that are NOT within limits (no matter good or bad)
        if (nr_i.first < 0)
        {
        
        // Now this nutrient is above limits and if second > 0 then it's good otherwise - bad
        bool is_bad_nutrient_above_limits = (nr_i.second < 0);
        
        for (auto &f_i : food_current_values)
        {
            int food_id = f_i.first;
            food *f = NULL;
            
            // Get the food either from the recipes or from normal foods
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
            
            float nutrient_amout_per_100g = f->get_nutrient_amount2(nutrient_id);
            
            float nutrient_amount = nutrient_amout_per_100g * f_i.second / 100.0;
            
            // If this is a bad nutrient above limits, but this food is green (food_rank < 1.0)
            //  then just skip it: event it's too bad on the bad nutrient then it is still
            //  is good on another good nutrient and overall it's better to leave it alone
            float good_grams, bad_grams;
            if (is_bad_nutrient_above_limits &&
                get_food_rank(f, nutrient_reference_values, &good_grams, &bad_grams) < 1.0)
            {
              //  std::cerr << "for nutrient " << nutrient_id << ", skip food " << food_id << " because of "
              //      << " is_bad_nutrient_above_limits=" << is_bad_nutrient_above_limits << ", rank="
              //  << get_food_rank(f, nutrient_reference_values, &good_grams, &bad_grams) << std::endl;
                continue;
            }
            
            // Note: don't count food with green leafs (food_rank > 0) in bad nutrients - because
            //  it's better not to touch it and rather to decrease some bad food
            if (!food_with_max || nutrient_amount > maximum_nutrient_amount)
            {
                food_with_max = f;
                food_id_with_max = food_id;
                maximum_nutrient_amount = nutrient_amount;
            }
        } // for (auto &f_i : food_current_values)
        } // if (nr_i.first < 0)
        
        // Now if this nutrient is good then mark the food with its maximum as green, otherwise - red
        // Note: if n_i.second > 0 then this means that this nutrient is within limits and no need to
        //  mark it (so the color is 0)
        if (food_with_max)
        {
            result.clear();
            construct_short_nutrient_info_helper_item(food_with_max,
                                                      maximum_nutrient_amount,
                                                      nutrient_id, (nr_i.first < 0)?(nr_i.second > 0 ? 1 : 2):0, result
            );

            if (!is_first) output << "," << std::endl; is_first = false;
            escaped_result.clear();
            output << "{\"food_nutrient_id\":\"" << food_id_with_max << "_" << nutrient_id << "\",\"value\":\"" <<
                json_escape(result, escaped_result) << "\"}";
        }
        
        // Output other foods for this nutrient
        for (auto &f_i : food_current_values)
        {
            int food_id = f_i.first;
            food *f = NULL;
            
            // Get the food either from the recipes or from normal foods
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
            if (f == food_with_max)
                continue;
            
            float nutrient_amout_per_100g = f->get_nutrient_amount2(nutrient_id);
            
            result.clear();
            construct_short_nutrient_info_helper_item(f,
                                                      nutrient_amout_per_100g * f_i.second / 100.0,
                                                      nutrient_id, 0, result
            );

            if (!is_first) output << "," << std::endl; is_first = false;
            escaped_result.clear();
            output << "{\"food_nutrient_id\":\"" << f_i.first << "_" << nutrient_id << "\",\"value\":\"" <<
                json_escape(result, escaped_result) << "\"}";
        } // for (auto &f_i : food_current_values)
    } // for (auto &n_i : nutrient_current_values)
    
    // Output pairs of meal groups and nutrients
    for (auto &mg : groups)
    {
        float cal = 0, prot = 0, fiber = 0;
        
        for (auto &nr_i : nutrient_reference_values)
        {
            int nutrient_id = std::abs(nr_i.first);
            std::string nutrient_name = get_short_nutrient_name(nutrient_id);

            // Determine total amount of this nutrient in this food group and value per 100g
            float nutrient_value = 0;
            float food_amount_in_group = 0;
            for (auto &food_id : mg.second)
            {
                food *f = NULL;
                // Get the food either from the recipes or from normal foods
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
                food_amount_in_group += food_values_temp[food_id];
                float nutrient_amount = food_values_temp[food_id] * f->get_nutrient_amount2(nutrient_id) / 100.0;
                nutrient_value += nutrient_amount;
                
                if (nutrient_id == 1003) prot += nutrient_amount;
                if (nutrient_id == 1079) fiber += nutrient_amount;
                if (nutrient_id == 1008) cal += nutrient_amount;
            } // for (auto &food_id : mg.second)
            float nutrient_value_per_100g = nutrient_value * 100.0 / food_amount_in_group;

            if (!is_first) output << "," << std::endl; is_first = false;
            output << "{\"meal_group_nutrient_id\":\"" << mg.first << "_" << nutrient_id << "\",\"value\":\"" <<
                "<span style=\\\"color: black;\\\">" << //nutrient_name << " " <<
                (int)nutrient_value_per_100g << "/" <<
                (int)nutrient_value << "</span>\"}";
        } // for (auto &nr_i : nutrient_reference_values)

        // Output scal for this group (considering the group as a single food)
        if (!is_first) output << "," << std::endl; is_first = false;
        float sCal = cal - (prot + fiber) * 12.0;
        output << "{\"meal_group_scal\":\"" << mg.first << "\",\"value\":\"" <<
            "<span style=\\\"color: " << (sCal<=0?"green":"red") << ";\\\">" << (int)sCal << "</span>\"}";

    } // for (auto &mg : groups)
    
    output << std::endl << "], \"weight_data\":" << std::endl;
    
    output << std::endl << "[" << std::endl;

    weigh_data wd;
    
    // Exclude thermic food effect from the intake
    float total_thermic_effect = carbs_intake * 4 * 0.1 + excess_protein * 4 * 0.2 + alcohol_intake * 4 * 0.2;
    wd.in_calorie_intake_ = calorie_intake - total_thermic_effect;
    
    wd.in_metabolism_ = md.out_metabolic_rate_;
    wd.in_protein_for_muscles_ = protein_for_muscles;
    
    float out_muscle_gain_;
    float out_fat_gain_;
    float out_metabolism_;
    // Determine muscle and fat gain over different periods of time
    float total_muscle_gain = 0;
    float total_muscle_loss_due_deficit = 0;
    float total_fat_gain = 0;
    float weight_projection[5];
    for (int i = 0; i < 365; ++i)
    {
        get_weight_data_slow(wd);
        wd.in_metabolism_ = wd.out_metabolism_;
        total_muscle_gain += wd.out_muscle_gain_;
        total_muscle_loss_due_deficit += wd.out_muscle_loss_component_due_to_deficit_;
        total_fat_gain += wd.out_fat_gain_;
        
        if (i < 30 /*i == 0 || i == 6 || i == 13 || i == 29*/ || i == 59 || i == 89 || i == 364)
        {
            switch (i)
            {
                case 6: weight_projection[0] = total_muscle_gain + total_fat_gain + 1000*md.in_mr_data_weight_; break;
                case 13: weight_projection[1] = total_muscle_gain + total_fat_gain + 1000*md.in_mr_data_weight_; break;
                case 29: weight_projection[2] = total_muscle_gain + total_fat_gain + 1000*md.in_mr_data_weight_; break;
                case 59: weight_projection[3] = total_muscle_gain + total_fat_gain + 1000*md.in_mr_data_weight_; break;
                case 89: weight_projection[4] = total_muscle_gain + total_fat_gain + 1000*md.in_mr_data_weight_; break;
            }
            
            if (i) output << "," << std::endl;
        
            output << "{\"day\": " << i + 1
                << ",\"calorie_surplus\":" << wd.out_calorie_surplus_
                << ",\"metabolism\":" << wd.out_metabolism_
                << ",\"muscle_gain\":" << wd.out_muscle_gain_
                << ",\"fat_gain\":" << wd.out_fat_gain_
                << ",\"total_muscle_gain\":" << total_muscle_gain
                << ",\"total_muscle_loss_due_deficit\":" << total_muscle_loss_due_deficit
                << ",\"total_fat_gain\":" << total_fat_gain
                << ",\"energy_muscle_gain\":" << wd.out_energy_muscle_gain_
                << ",\"muscle_loss_component_due_to_deficit\":" << wd.out_muscle_loss_component_due_to_deficit_
                << "}";
        }
    }

    output << std::endl << "]," << std::endl;

    output << "\"metabolic_info\":{" << std::endl
        << "\"in_mr_data_gender\":" << md.in_mr_data_gender_<< "," << std::endl
    << "\"in_mr_data_weight\":" << md.in_mr_data_weight_ << "," << std::endl
    << "\"in_mr_data_height\":" << md.in_mr_data_height_ << "," << std::endl
    << "\"in_mr_data_age\":" << md.in_mr_data_age_ << "," << std::endl
    << "\"in_mr_data_exercise\":" << md.in_mr_data_exercise_ << "," << std::endl
    << "\"in_mr_data_body_type\":" << md.in_mr_data_body_type_ << "," << std::endl
    << "\"out_basal_metabolic_rate\":" << md.out_basal_metabolic_rate_ << "," << std::endl
    << "\"out_exercise_component\":" << md.out_exercise_component_ << "," << std::endl
    << "\"out_fat_component\":" << md.out_fat_component_ << "," << std::endl
    << "\"out_protein_component\":" << md.out_protein_component_ << "," << std::endl
    << "\"out_fat_component_correction\":" << md.out_fat_component_correction_ << "," << std::endl
    << "\"out_protein_component_correction\":" << md.out_protein_component_correction_ << "," << std::endl
    << "\"out_metabolic_rate\":" << md.out_metabolic_rate_ << "," << std::endl
    << "\"out_protein_percent_intake\":" << md.out_protein_percent_intake_ << "," << std::endl
    << "\"out_protein_max_intake\":" << md.out_protein_max_intake_ << "," << std::endl
    << "\"target_weight\":" << target_weight << "," << std::endl
    << "\"target_weight_deadline\":" << target_weight_deadline << std::endl
    
    << "}," << std::endl;

    // If there is not calorie intake at all then just leave the need as the metebolic rate
    float calorie_need1 = md.out_metabolic_rate_;
    float calorie_need2 = md.out_metabolic_rate_;
    
    // If there is a calorie intake then the need is more complex because we have to take into account
    //  protien and thermic effect
    if (calorie_intake)
    {
        float denominator1 = (calorie_intake - wd.in_protein_for_muscles_ * 4 - total_thermic_effect);
        if (denominator1 < 0.1)
            denominator1 = 0.1;
        float denominator2 = calorie_intake - total_thermic_effect;
        if (denominator2 < 0.1)
            denominator2 = 0.1;
        calorie_need1 = 0.5 * md.out_metabolic_rate_ * calorie_intake / denominator1
            +
            0.5 * (md.out_metabolic_rate_ +  wd.in_protein_for_muscles_ * 4) * calorie_intake /
                denominator2;
    
        calorie_need2 = determine_calorie_need(calorie_intake,
                                protein_for_muscles,
                                total_thermic_effect,
                                carbs_intake,
                                alcohol_intake,
                                protein_intake,
                                md);
    }
    
    float calorie_intake_by_nutrients = protein_intake * 4 + fat_intake * 9 + carbs_intake * 4 + alcohol_intake + 7;
    float calorie_intake_protein_percent = protein_intake * 4 / calorie_intake_by_nutrients;
    float calorie_intake_fat_percent = fat_intake * 9 / calorie_intake_by_nutrients;
    float calorie_intake_carbs_percent = carbs_intake * 4 / calorie_intake_by_nutrients;
    float calorie_intake_sugar_percent = sugar_intake * 4 / calorie_intake_by_nutrients;
    float calorie_intake_alcohol_percent = alcohol_intake * 7 / calorie_intake_by_nutrients;

    output << "\"additional_weight_info\":{" << std::endl
        << "\"carbs_intake\":" << carbs_intake  << "," << std::endl
        << "\"exess_sodium_intake\":" << exess_sodium_intake  << "," << std::endl
        << "\"water_retention_carbs\":" << carbs_intake * 4  << "," << std::endl
        << "\"water_retention_sodium\":" << exess_sodium_intake/400.0 * 900 << "," << std::endl
        << "\"excess_protein_intake\":" << excess_protein << "," << std::endl
        << "\"carbs_thermic_effect\":" << carbs_intake * 4 * 0.1 << "," << std::endl
        << "\"protein_thermic_effect\":" << excess_protein * 4 * 0.2 << "," << std::endl
        << "\"alcohol_thermic_effect\":" << alcohol_intake * 4 * 0.2 << "," << std::endl
        << "\"calorie_surplus\":" << wd.out_calorie_surplus_ << "," << std::endl
        << "\"calorie_intake\":" << calorie_intake << "," << std::endl
        << "\"calorie_intake_excl_thermic\":" << wd.in_calorie_intake_ << "," << std::endl
        << "\"protein_need_above_bmr_cal\":" << wd.in_protein_for_muscles_ * 4 << "," << std::endl
        << "\"calorie_need\":" << calorie_need1 << "," << std::endl
        << "\"calorie_need2\":" << calorie_need2 << "," << std::endl;
    
    if (target_weight)
    {
        // Use heuristic - to lose one pound a week we need caloric deficit of 500 calories
        float calorie_deficit_for_target_in_week = 500 * 1000 * (target_weight - md.in_mr_data_weight_)/453.592;
        output << "\"calorie_deficit_for_target_in_week\":" << calorie_deficit_for_target_in_week  << "," << std::endl;
        
        // Determine the deficit with thermic effect
        float calorie_deficit_for_target_in_week_with_thermic;
        float thermic_K;
        if (wd.in_calorie_intake_)
            // We know current ratio of intake vs intake without thermic - use it
            thermic_K = calorie_intake / wd.in_calorie_intake_;
        else
            // We don't know that - use average
            thermic_K = 1.1;
        calorie_deficit_for_target_in_week_with_thermic = calorie_deficit_for_target_in_week * thermic_K;
        output << "\"calorie_deficit_for_target_in_week_with_thermic\":" << calorie_deficit_for_target_in_week_with_thermic  << "," << std::endl;
        
        // The target intake to reach the goal in different periods of time
        output << "\"calorie_target_intake_1w\":" <<
            (calorie_need2 + calorie_deficit_for_target_in_week * thermic_K / 7) << "," << std::endl;
        output << "\"calorie_target_intake_2w\":" <<
            (calorie_need2 + calorie_deficit_for_target_in_week * thermic_K / 14) << "," << std::endl;
        output << "\"calorie_target_intake_1m\":" <<
            (calorie_need2 + calorie_deficit_for_target_in_week * thermic_K / 30) << "," << std::endl;
        output << "\"calorie_target_intake_2m\":" <<
            (calorie_need2 + calorie_deficit_for_target_in_week * thermic_K / 60) << "," << std::endl;
        output << "\"calorie_target_intake_3m\":" <<
            (calorie_need2 + calorie_deficit_for_target_in_week * thermic_K / 90) << "," << std::endl;

        // The weight projection in accordance to the current trend in different periods of time
        output << "\"weight_1w\":" <<
            int(weight_projection[0]/1000.0) << "," << std::endl;
        output << "\"weight_2w\":" <<
            int(weight_projection[1]/1000.0) << "," << std::endl;
        output << "\"weight_1m\":" <<
            int(weight_projection[2]/1000.0) << "," << std::endl;
        output << "\"weight_2m\":" <<
            int(weight_projection[3]/1000.0) << "," << std::endl;
        output << "\"weight_3m\":" <<
            int(weight_projection[4]/1000.0) << "," << std::endl;
    }
    
    output << "\"calorie_intake_by_nutrients\":" << calorie_intake_by_nutrients << "," << std::endl
        << "\"calorie_intake_protein_percent\":" << calorie_intake_protein_percent << "," << std::endl
        << "\"calorie_intake_fat_percent\":" << calorie_intake_fat_percent << "," << std::endl
        << "\"calorie_intake_carbs_percent\":" << calorie_intake_carbs_percent << "," << std::endl
        << "\"calorie_intake_sugar_percent\":" << calorie_intake_sugar_percent << "," << std::endl
        << "\"calorie_intake_alcohol_percent\":" << calorie_intake_alcohol_percent << std::endl
        << "}" << std::endl;

    //calorie_intake - total_thermic_effect - md.out_metabolic_rate_ - wd.in_protein_for_muscles_ * 4.0
    
/*    cal_need = f + thermic * (cal_need / cal_intake)
    
    cal_need * i = f * i + t * cal_need
    cal_need * (i-t) =  f * i
    cal_need = f * i / (i - t);*/
    
    output << std::endl << "}" << std::endl;
}

void foods::get_food_details(std::vector<int> &food_list,
                             std::vector<int> &nutrients_list,
                             std::ostream &output)
{
    output << "{\"event\":\"get_food_details\",\"data\":[" << std::endl;
    bool is_first=true;
    for (auto food_id : food_list)
    {
        food *f = get_food(food_id);
        if (f && !f->food_name_.empty())
        {
            std::string js_escaped;
            json_escape(f->food_name_, js_escaped);
            if (!is_first)output<<",";is_first=false;
            output << "{" << "\"food_id\":" << food_id << ",\"food_name\":\"" << js_escaped << "\",\"nutrients\":[";
            bool is_first=true;
            for (auto nutrient_id : nutrients_list)
            {
                if (!is_first)output<<",";is_first=false;
                output << "{\"nutrient_id\":" << nutrient_id
                    << ",\"amount\":" << f->get_nutrient_amount2(nutrient_id) << "}" << std::endl;
            }
            output << "]}" << std::endl;
        }
    }
    output << "]}" << std::endl;
}

} // namespace balanced_diet
