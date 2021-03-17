/*
*       balanced_diet_data.cpp
*
*       (C) Denis Anikin 2020
*
*       Work with the data for the balanced diet
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
#include <fstream>

#include "balanced_diet.h"

namespace balanced_diet
{

foods::foods() :
    /*facts_recommended_nutrient_amout_({
    {1003, 50},   // Protein
    {1004, 60},   // Fat
    {1005, 275},   // Carbs
    {1008, 2000},   // Cal
    {1018, 40}, // Alcohol
    {1079, 38},   // Fiber
    {1087, 1500},   // Calcium
    {1089, 10},   // Iron
    {1090, 410},   // Magnesium
    {1091, 700},    // Phosphorus
    {1092, 4100},   // Potassium
    {1093, 2300},   // Sodium
    {1095, 11},   // Zinc
    {1096, 35},   // Chromium
    {1098, 900},   // Copper
    {1099, 4},   // Fluoride
    {1100, 150000},   // Iodine (UG)
    {1101, 2.3},    // Manganese
    {1102, 45}, // Molybdenum (UG)
    {1103, 55},   // Selenium
    {1104, 900},   // Vitamin A
    {1110, 600},   // Vitamin D
    {1124, 22.5},   // Vitamin E
    {1162, 90},   // Vitamin C
    {1165, 1.2},    // Thiamin (Vitamin B1)
    {1166, 1.3},   // Riboflavin (Vitamin B2)
    {1167, 16},   // Niacin (Vitamin B3)
    {1170, 5}, // Pantothenic acid (Vitamin B5)
    {1175, 1.3},   // Vitamin B-6
    {1176, 30},   // Biotin (Vitamin B7)
    {1177, 400},   // Folate
    {1178, 2.4},   // Vitamin B-12
    {1180, 550},   // Choline
    {1183, 120},   // Vitamin K
    {1184, 120},   // Vitamin K
    {1185, 120},   // Vitamin K
    {1198, 900},   // Copper
    {1235, 36},   // Added sugar
    {1253, 300},   // Cholesterol
    {1257, 2},  // Trans fat
    {1258, 19},   // Saturated fat
        //{1268, },   // 18:1 (kind of Omega-9)
    {1272, 0.25},   // DHA (kind of Omega-3)
    {1278, 0.25},   // EPA (kind of Omega-3)
        //{1279, },   // 22:1 (king of Omega-9)
    {1293, 22},   // Polyunsaturated fat
    {1292, 44},   // Monunsaturated fat
    {1316, 15},   // 18:2 n-6 c,c (Omega-6)
    {1404, 0.25},   // ALA (kind of Omega-3)
    {2000, 100},   // Sugar
    }),*/

nutrient_augmented_data_({
    {1003, {50 , -1, "" , "", "Prt", "Protein"}},
    {1004, {-1 , 60, "" , "", "Fat", "Total Fat"}},   // Fat
    {1005, {-1 , 275, "" , "", "Crb", "Total Carbohydrate"}},   // Carbs
    {1008, {-1 , 2000, ""  , "", "Cal", "Calories"}},   // Cal
    {1018, {-1 , 40, "" , "", "Alc", "Alcohol"}}, // Alcohol
    {1079, {38 , -1, "Plant foods, including oatmeal, lentils, peas, beans, fruits, and vegetables" , "Helps with digestion, lowers LDL ('bad') cholesterol, helps you feel full, and helps maintain blood sugar levels", "Fib", "Total Fiber"}},   // Fiber
    {1087, {1000 , 2500, "Milk, fortified nondairy alternatives like soy milk, yogurt, hard cheeses, fortified cereals, unfortified almond milk, kale" , "Needed for bone growth and strength, blood clotting, muscle contraction", "Clc", "Calcium"}},   // Calcium
    {1089, {10 , 45, "Fortified cereals, beans, lentils, beef, turkey (dark meat), soy beans, spinachi" , "Needed for red blood cells and many enzymes", "Irn", "Iron"}},   // Iron
    {1090, {410 , -1, "Green leafy vegetables, nuts, dairy, soybeans, potatoes, whole wheat, quinoa" , "Helps with heart rhythm, muscle and nerve function, bone strength", "Mgn", "Magnesium"}},   // Magnesium
    {1091, {700 , 3000, "Milk and other dairy products, peas, meat, eggs, some cereals and breads" , "Cells need it to work normally. Helps make energy. Needed for bone growth.", "Phs", "Phosphorus"}},    // Phosphorus
    {1092, {4100 , -1, "Potatoes, bananas, yogurt, milk, yellowfin tuna, soybeans, and a variety of fruits and vegetables" , "Helps control blood pressure, makes kidney stones less likely", "Pts", "Potassium"}},   // Potassium
    {1093, {1500 , 2300, "Foods made with added salt, such as processed and restaurant foods" , "Important for fluid balance", "Sod", "Sodium"}},   // Sodium
    {1095, {11 , 40, "Red meats, some seafood, fortified cereals" , "Supports your immune system and nerve function. Also important for reproduction", "Znc", "Zinc"}},   // Zinc
    {1096, {35 , -1, "Broccoli, potatoes, meats, poultry, fish, some cereals" , "Helps control blood sugar levels", "Chr", "Chromium"}},   // Chromium
    {1098, {0.9 , 10, "Seafood, nuts, seeds, wheat bran cereals, whole grains" , "Helps your body process iron", "Cop", "Copper"}},   // Copper
    {1099, {4 , 10, "Fluoridated water, some sea fish" , "Prevents cavities in teeth, helps with bone growth", "Flr", "Fluoride"}},   // Fluoride
    {1100, {150000 , 1100000, "Seaweed, seafood, dairy products, processed foods, iodized salt" , "Helps make thyroid hormones", "Iod", "Iodine"}},   // (UG)
    {1101, {2.3 , 11, "Nuts, beans and other legumes, tea, whole grains" , "Helps form bones and make some enzymes", "Man", "Manganese"}},    // Manganese
    {1102, {45 , 2000, "Legumes, leafy vegetables, grains, nuts" , "Needed to make some enzymes", "Mol", "Molybdenum"}}, //  (UG)
    {1103, {55 , 400, "Organ meats, seafood, dairy, some plants (if grown in soil with selenium), Brazil nuts" , "Protects cells from damage. Helps manage thyroid hormone", "Sel", "Selenium"}},   // Selenium
    {1104, {900 , 3000, "Sweet potatoes, carrots, spinach, fortified cereals" , "Needed for vision, the immune system, and reproduction", "VtA", "Vitamin A"}},   // Vitamin A
    {1106, {900 , 3000, "Sweet potatoes, carrots, spinach, fortified cereals" , "Needed for vision, the immune system, and reproduction", "VtA", "Vitamin A"}},   // Vitamin A
    {1109, {22.5 , 1000, "Fortified cereals, sunflower seeds, almonds, peanut butter, vegetable oils" , "Helps protect cells against damage", "VtE", "Vitamin E"}},   // Vitamin E
    {1110, {600 , 4000, "Fish liver oils, fatty fish, fortified milk products, fortified cereals" , "Needed for bones, muscles, the immune system, and communication between the brain and the rest of your body", "VtD", "Vitamin D"}},   // Vitamin D
    {1124, {22.5 , 1000, "Fortified cereals, sunflower seeds, almonds, peanut butter, vegetable oils" , "Helps protect cells against damage", "VtE", "Vitamin E"}},   // Vitamin E
    {1162, {90 , 2000, "Red and green peppers, kiwis, oranges and other citrus fruits, strawberries, broccoli, tomatoes" , "Helps protect against cell damage, supports the immune system, and helps your body make collagen", "VtC", "Vitamin C"}},   // Vitamin C
    {1165, {1.2 , -1, "Whole-grain, enriched, fortified products like bread and cereals" , "Helps the body process carbs and some protein", "VB1", "Vitamin B-1"}},    // Thiamin (Vitamin B1)
    {1166, {1.3 , -1, "Milk, bread products, fortified cereals" , "Helps convert food into energy. Also helps make red blood cells", "VB2", "Vitamin B-2"}},   // Riboflavin (Vitamin B2)
    {1167, {16 , -1, "Meat, fish, poultry, enriched and whole grain breads, fortified cereals" , "Helps with digestion and with making cholesterol", "VB3", "Vitamin B-3"}},   // Niacin (Vitamin B3)
    {1170, {5 , -1, "Chicken, beef, potatoes, oats, cereals, tomatoes" , "Helps turn carbs, protein, and fat into energy", "VB5", "Vitamin B-5"}}, // Pantothenic acid (Vitamin B5)
    {1175, {1.3 , 100, "Fortified cereals, fortified soy products, chickpeas, potatoes, organ meats" , "Helps with metabolism, the immune system, and babies' brain development", "VB6", "Vitamin B-6"}},   // Vitamin B-6
    {1176, {30 , -1, "Liver, fruits, meats" , "Helps your body make fats, protein, and other things your cells need", "VB7", "Vitamin B-7"}},   // Biotin (Vitamin B7)
    {1177, {400 , 1000, "Dark, leafy vegetables; enriched and whole grain breads; fortified cereals" , "Helps prevent birth defects, important for heart health and for cell development", "Fol", "Folate"}},   // Folate
    {1178, {2.4 , -1, "Fish, poultry, meat, dairy products, fortified cereals" , "Helps your body make red blood cells", "B12", "Vitamin B-12"}},   // Vitamin B-12
    {1180, {550 , 3500, "Milk, liver, eggs, peanuts" , "Helps make cells", "Chl", "Choline"}},   // Choline
    {1183, {120 , -1, "Green vegetables like spinach, collards, and broccoli; Brussels sprouts; cabbage" , "Important in blood clotting and bone health", "VtK", "Vitamin K"}},   // Vitamin K
    {1184, {120 , -1, "Green vegetables like spinach, collards, and broccoli; Brussels sprouts; cabbage" , "Important in blood clotting and bone health", "VtK", "Vitamin K"}},   // Vitamin K
    {1185, {120 , -1, "Green vegetables like spinach, collards, and broccoli; Brussels sprouts; cabbage" , "Important in blood clotting and bone health", "VtK", "Vitamin K"}},   // Vitamin K
    {1198, {-1 , -1, "" , "", "Bet", "Betaine"}},
    {1235, {-1 , 36, "" , "", "aSg", "Added sugar"}},   // Added sugar
    {1253, {-1 , 300, "" , "", "Cho", "Cholesterol"}},   // Cholesterol
    {1257, {-1 , 2, "" , "", "TrF", "Trans fat"}},  // Trans fat
    {1258, {-1 , 19, "", "SaF", "Saturated fat"}},   // Saturated fat
    {1268, {-1 , -1, "", "Om9", "Omega-9 18:1"}},   // 18:1 (kind of Omega-9)
    {1272, {0.25 , -1, "" , "", "Om3", "Omega-3 DHA"}},   // DHA (kind of Omega-3)
    {1278, {0.25 , -1, "" , "", "Om3", "Omega-3 EPA"}},   // EPA (kind of Omega-3)
    {1279, {-1 , -1, "" , "", "Om9", "Omega-9 22:1"}},   // 22:1 (king of Omega-9)
    {1292, {-1 , 44, "" , "", "MsF", "Monunsaturated Fat"}},   // Monunsaturated fat
    {1293, {-1 , 22, "" , "", "PsF", "Polyunsaturated fat"}},   // Polyunsaturated fat
    {1316, {15 , -1, "" , "", "Om6", "Omega-6"}},   // 18:2 n-6 c,c (Omega-6)
    {1404, {0.25 , -1, "" , "", "Om3", "Omega-3 ALA"}},   // ALA (kind of Omega-3)
    {2000, {-1 , 100 , "" , "", "Sug", "Sugar"}},   // Sugar
}),

    food_augmented_data_({
        
        /*
         *  McDonald's
         */
        
        {170720, {215, "1 sandwich", "McDonald's", "Big Mac"}},   // Big Mac
        {172067, {200, "1 sandwich", "McDonald's", "Big Mac without sauce"}},   // McDONALD'S, BIG MAC (without Big Mac Sauce)
        {172069, {150, "1 sandwich", "McDonald's"}},   // McDONALD'S, Bacon Egg & Cheese Biscuit
        {173307, {140, "1 sandwich", "McDonald's"}},   // McDONALD'S, Egg McMUFFIN
        {173308, {130, "1 sandwich", "McDonald's"}},   // McDONALD'S, Sausage McMUFFIN
        {172068, {175, "1 sandwich", "McDonald's"}},   // McDONALD'S, Sausage McMUFFIN with Egg
        {170719, {205, "1 sandwich", "McDonald's"}},   // McDONALD'S, QUARTER POUNDER with Cheese
        {172066, {275, "1 sandwich", "McDonald's"}},   // McDONALD'S, DOUBLE QUARTER POUNDER with Cheese
        {173297, {16, "1 nugget", "McDonald's"}},   // McDONALD'S, Chicken McNUGGETS
        {170321, {171, "1 sandwich", "McDonald's"}},   // McDONALD'S, QUARTER POUNDER
        {173314, {124, "1 sandwich", "McDonald's"}},   // McDONALD'S, FILET-O-FISH (without tartar sauce)
        {173300, {175, "1 McGriddles", "McDonald's"}}, // McDONALD'S, Bacon, Egg & Cheese McGRIDDLES
        {170320, {120, "1 cheeseburger", "McDonald's"}}, // McDONALD'S, Cheeseburger
        {173306, {135, "1 sandwich", "McDonald's"}}, // McDONALD'S, McCHICKEN Sandwich (without mayonnaise)

        /*
         *  Burger king
         */
        
        {170731, {100, "1 piece", "Burger King"}},
        {170374, {100, "1 piece", "Burger King"}},
        {783472, {100, "1 piece", "Burger King"}},
        {783463, {100, "1 piece", "Burger King"}},
        {170328, {100, "1 piece", "Burger King"}},
        {170729, {100, "1 piece", "Burger King"}},
        {783508, {100, "1 piece", "Burger King"}},
        {170730, {100, "1 piece", "Burger King"}},
        {783440, {100, "1 piece", "Burger King"}},
        {783484, {100, "1 piece", "Burger King"}},
        {783496, {100, "1 piece", "Burger King"}},
        {783426, {100, "1 piece", "Burger King"}},
        {783427, {100, "1 piece", "Burger King"}},
        {783485, {100, "1 piece", "Burger King"}},
        {170727, {100, "1 piece", "Burger King"}},
        {170728, {100, "1 piece", "Burger King"}},
        {170331, {100, "1 piece", "Burger King"}},
        {170330, {100, "1 piece", "Burger King"}},
        {173315, {100, "1 piece", "Burger King"}},
        {173315, {100, "1 piece", "Burger King"}},
        {172076, {100, "1 piece", "Burger King"}},
        {173316, {100, "1 piece", "Burger King"}},
        {407462, {100, "1 piece", "Burger King"}},
        {170700, {100, "1 piece", "Burger King"}},
        {172077, {100, "1 piece", "Burger King"}},
        {170327, {100, "1 piece", "Burger King"}},
        {172078, {75, "1 serving - small", "Burger King"}}, //
        {170771, {91, "1 serving - small", "Burger King"}}, // BURGER KING, Onion Rings
        
        /*
         *  Subway
         */
        
        {170713, {100, "1 piece", "Subway"}},
        {170307, {100, "1 piece", "Subway"}},
        {170705, {100, "6 inch sandwich", "Subway"}},
        {170314, {198, "1 piece", "Subway"}},
        {170711, {100, "1 piece", "Subway"}},
        {170310, {203, "6 inch sandwich", "Subway"}},
        {170712, {100, "1 piece", "Subway"}},
        {170316, {100, "1 piece", "Subway"}},
        {170315, {100, "1 piece", "Subway"}},
        {173315, {100, "1 piece", "Subway"}},
        {170312, {100, "1 piece", "Subway"}},
        {170708, {100, "1 piece", "Subway"}},
            
        /*
         *  Taco Bell
         */
        
        {170732, {100, "1 piece", "Taco Bell"}},
        {793243, {100, "1 piece", "Taco Bell"}},
        {170334, {100, "1 piece", "Taco Bell"}},
        {170335, {100, "1 piece", "Taco Bell"}},
        {170733, {100, "1 piece", "Taco Bell"}},
        {170736, {100, "1 piece", "Taco Bell"}},
        {709432, {100, "1 piece", "Taco Bell"}},
        {170332, {100, "1 piece", "Taco Bell"}},
        {170734, {100, "1 piece", "Taco Bell"}},
        {170735, {100, "1 piece", "Taco Bell"}},
        {404763, {100, "1 piece", "Taco Bell"}},
        {170337, {195, "1 portion", "Taco Bell"}},
        {404876, {100, "1 piece", "Taco Bell"}},
        {170336, {100, "1 piece", "Taco Bell"}},

        /*
         *  KFC
         */
        
        {170732, {100, "1 piece", "KFC"}},
        {170344, {100, "1 piece", "KFC"}},
        {170345, {100, "1 piece", "KFC"}},
        {170740, {100, "1 piece", "KFC"}},
        {170342, {100, "1 piece", "KFC"}},
        {170741, {100, "1 piece", "KFC"}},
        {170732, {100, "1 piece", "KFC"}},
        {170742, {100, "1 piece", "KFC"}},
        {170732, {100, "1 piece", "KFC"}},
        {170743, {100, "1 piece", "KFC"}},
        {170744, {163, "1 breast", "KFC"}},
        {170745, {100, "1 piece", "KFC"}},
        {170348, {100, "1 piece", "KFC"}},
        {170732, {100, "1 piece", "KFC"}},
        {170349, {100, "1 piece", "KFC"}},
        {170341, {100, "1 piece", "KFC"}},
        {170349, {100, "1 piece", "KFC"}},
        {170347, {100, "1 piece", "KFC"}},
        {170346, {100, "1 piece", "KFC"}},
        {170349, {100, "1 piece", "KFC"}},
        {170747, {100, "1 piece", "KFC"}},
        {170746, {100, "1 piece", "KFC"}},
        {170737, {100, "1 piece", "KFC"}},
        {170738, {100, "1 piece", "KFC"}},
        {170343, {100, "1 piece", "KFC"}},
        {170339, {100, "1 piece", "KFC"}},
        {170340, {100, "1 piece", "KFC"}},
        
        // Todo: DENNY'S, Wawa, WENDY'S, DUNKIN', Starbucks
        
        {358257, {122, "1 serving", "IHOP At Home"}},
        {607746, {145, "2 pieces", "IHOP"}},
        
        {442674, {65, "1 doughnut", "Krispy Kreme"}},
        
        {173282, {106, "1 slice", "Domino's"}},
        
        /*
         *  Difference kinds of red wine
         */
        
        {174840, {150, "1 glass", "Red wine"}},
        {171872, {150, "1 glass", "Red wine"}},
        {174838, {150, "1 glass", "Red wine"}},
        {174835, {150, "1 glass", "Red wine"}},
        {174834, {150, "1 glass", "Red wine"}},
        {171925, {150, "1 glass", "Red wine"}},
        {173194, {150, "1 glass", "Red wine"}},
        {174836, {150, "1 glass", "Red wine"}},
        {173191, {150, "1 glass", "Red wine"}},
        {173193, {150, "1 glass", "Red wine"}},
        {173190 , {150, "1 glass", "Red wine"}},
        {173208, {150, "1 glass", "Red wine"}},
        {174839, {150, "1 glass", "Red wine"}},
        {173192, {150, "1 glass", "Red wine"}},
        {174109, {150, "1 glass", "Red wine"}},

        /*
         *  Difference kinds of white wine
         */
        
        {174110, {150, "1 glass", "White wine"}},
        
        /*
         *  Beer
         */
        
        {168746, {330, "1 bottle", "Beer"}},
        
        {358608, {60, "1 piece", "XLarge Egg"}},   // Extra large fresh egg
        
        /*
         *  Most popular foods from Wal-Mart
         */
        
        {173944, {101, "1 item", "Walmart", "Bananas"}},   // Bananas, raw
        {576957, {70, "1/2 cup", "Walmart", "Mini cucumber"}},   // Mini cucumber
        {322892, {250, "1 serving", "Walmart"}},    // Milk whole 3.25% added vitamin D
        {169997, {35, "1/4 cup", "Walmart", "Coriander leaves"}},   // Coriander, leaves
        {487430, {50, "1 piece", "Walmart", "Grade A large egg"}},   // Grade A large egg
        {170027, {148, "1 item", "Walmart", "Russet potatoes"}},   // Russet potatoes
        {600111, {43, "1 bun", "Walmart", "Whole wheat hot dog buns"}},     // Whole wheat hot dog buns
        {747447, {85, "1 cup", "Walmart", "Broccoli"}},     // Broccoli, raw
        {693680, {125, "1 serving", "Walmart"}},         // Whole kernel golden sweet corn
        {514033, {26, "1 slice", "Walmart", "Honey wheat bread"}},   // Honey wheat bread
        {411709, {30, "2 tbsp", "Walmart"}},    // HALF AND HALF ULTRA-PASTEURIZED MILK AND CREAM
        {593747, {26, "1 slice", "Walmart"}},   // WHOLE WHEAT SANDWICH BREAD
        {173110, {112, "1 serving", "Walmart"}},// Beef, ground, 93% lean meat / 7% fat, raw
        {719700, {95, "1 bagel", "Walmart"}},   // PLAIN BAGELS
        {507298, {83, "1 piece", "Walmart"}},
        {561913, {8, "2 tsp", "Walmart"}},  // Suggar $0.53 lb
        
        {170427, {149, "1 serving", "Walmart - vegetables"}}, // Green pepper $0.72 each
        {169248, {72, "1 serving", "Walmart - vegetables"}}, // Iceberg $1.28 each (weight?)
        {168568, {81, "3 oz", "Walmart - vegetables"}}, // Baby carrots 0.98 lb
        {169988, {28, "1 serving", "Walmart - vegetables"}}, // Celery $1.28 each
        {169291, {220, "1 serving - 1/2 lb", "Walmart - vegetables"}}, // Zucchini $1.38 lb
        {787803, {30, "1/4 cup", "Walmart - vegetables"}}, // Green onion $0.50 bunch
        {787774, {125, "1 serving", "Walmart - vegetables"}}, // Aspsragus $2.97 lb
        {790646, {160, "1 serving", "Walmart - vegetables"}}, // Yellow onions $0.88 lb
        {170108, {149, "1 serving", "Walmart - vegetables"}}, // Red bell pepper $1.38 each
        {710692, {180, "1 serving", "Walmart - vegetables"}}, // Tomatoes on the vine $2.48 lb
        {523755, {148, "1 serving", "Walmart - vegetables"}}, // Grape tomatoes $2.48 1 pint

        {786790, {144, "1 serving", "Walmart - fruits"}}, // Strawberries $2.00 lb
        {577920, {50, "1 serving", "Walmart - fruits"}}, // Mini avocados $0.68 each
        {512448, {130, "1 1/2 cup", "Walmart - fruits"}}, // Fresh Red Seedless Grapes $1.38 each
        {168195, {109, "1 serving", "Walmart - fruits"}}, // Clmentines $1.29 lb
        {586821, {101, "1 cup", "Walmart - fruits"}}, // Blueberries  $0.33 oz
        {474199, {60, "1 serving", "Walmart - fruits"}}, // Lemons $0.57 each
        {512449, {130, "1 1/2 cup", "Walmart - fruits"}}, // Fresh Green Seedless Grapes $1.36 lb
        {578071, {245, "1 serving", "Walmart - fruits"}}, // Gala Apples (red) $0.92 lb
        {168155, {67, "1 serving", "Walmart - fruits"}}, // Limes $0.28 each
        {786661, {134, "1 serving", "Walmart - fruits"}}, // Cantaloupe $2.18 each
        {786754, {280, "1 serving", "Walmart - fruits"}}, // Watermelon $5.88 each
        {169911, {170, "1 serving", "Walmart - fruits"}}, // Melon Honeydew $2.98 each
        
        /*
         
         Try to search more frutis by this
         
         http://localhost:9999/food_by_rank?nutrient_limits=%221005%22,%22%3E%22,%229%22,%221005%22,%22%3C%22,%2217%22,%222000%22,%22%3E%22,%222%22,%222000%22,%22%3C%22,%2215%22,%221003%22,%22%3C%22,%222%22,%221004%22,%22%3C%22,%223%22,%221093%22,%22%3C%22,%22100%22
         
         http://localhost:9999/food_by_rank?nutrient_limits=%221005%22,%22%3E%22,%229%22,%221005%22,%22%3C%22,%2217%22,%222000%22,%22%3E%22,%222%22,%222000%22,%22%3C%22,%2215%22,%221003%22,%22%3C%22,%222%22,%221004%22,%22%3C%22,%223%22,%221093%22,%22%3C%22,%22100%22&exc=soda%20drink%20blend%20puree%20STRAWBERRIES%20juice%20BEVERAGE%20SMOOTHIE%20Bottle%20SMOIOTHIE%20STRAWBERRY%20Sauce%20ROASTED%20UNSWEETENED%20STARTER%20WELLNESS%20COFFEE%20milk%20FROZEN%20ESPRESSO
         
         */

        {494841, {28, "2 tbsp", "Walmart - cheese"}}, // Cream cheese $0.16 oz
        {373175, {28, "2 tbsp", "Walmart - cheese"}}, // Cream cheese light
        {358225, {28, "2 tbsp", "Walmart - cheese"}}, // Cream cheese fat free
        {394470, {19, "1 slice", "Walmart - cheese"}}, // American single cheese slices $0.23 oz
        {380110, {28, "1/3 cup", "Walmart - cheese"}}, // Great Value Finely Shredded Colby & Monterey Jack Cheese, $0.25 oz

        {494886, {240, "1 cup", "Walmart - milk"}}, // Fat free milk $0.023 oz
        {494846, {240, "1 cup", "Walmart - milk"}}, // Reduced fat milk $0.024 oz
        {493894, {240, "1 cup", "Walmart - milk"}}, // Low fat milk $0.022 oz
        {495973, {240, "1 cup", "Walmart - milk"}}, // Low fat chocolate milk $0.024 oz
        {496022, {240, "1 cup", "Walmart - milk"}}, // Almond milk $0.037 oz
        {709744, {240, "1 cup", "Walmart - milk"}}, // Lactose free reduced fat milk $3.38/52 oz

        {382182, {125, "1 serving", "Walmart - canned goods"}}, // Canned Green beans, salt $0.034 oz
        {523903, {125, "1 serving", "Walmart - canned goods"}}, // Canned Sweet peas $0.033 oz
        {605430, {125, "1 serving", "Walmart - canned goods"}}, // Canned Tomatoes $0.098 oz
        
        {449502, {113, "1 serving - 4 oz", "Walmart - meat & seafood"}}, // Steelhead Trout $10.68 lb
        {520725, {112, "1 serving - 4 o", "Walmart - meat & seafood"}}, // Smithfield Marinated Applewood Smoked Bacon Fresh Pork Loin Filet $3.48 lb
        {521946, {112, "1 serving - 4 oz", "Walmart - meat & seafood"}}, // Wild Caught Sea Scallops $14.98 lb
        {351440, {112, "1 serving - 4 oz", "Walmart - meat & seafood"}}, // Seafood Festival $8.33 lb
        {779628, {112, "1 serving - 4 oz", "Walmart - meat & seafood"}}, // Beef Choice Angus Chuck Roast $5.97 lb
        {174754, {112, "1 serving", "Walmart - meat & seafood"}}, // Ground beef  $6.78 lb
        {174754, {112, "1 serving", "Walmart - meat & seafood"}}, // Ground beef  $6.78 lb
    
        /*
         *  Popular recipes
         */
        
        {374386, {28, "2 tbsp", "Popular recipes"}}, // Guacamole
        {540630, {340, "1 serving", "Popular recipes"}}, // MACARONI & CHEESE WITH BROCCOLI
        {479091, {250, "1 serving", "Popular recipes"}}, // ROASTED GARLIC CHEESE RISOTTO WITH GRILLED CHICKEN
        {526689, {250, "1 serving", "Popular recipes"}}, // SWEET CHIPOTLE CHICKEN
        {479821, {112, "1 serving - 2 pancrepes", "Popular recipes"}}, // ALL NATURAL APPLE PANCREPES
        {704758, {28, "1 serving - 4 cookies", "Popular recipes"}}, // GINGER SNAPS COOKIES, GINGER SNAPS
        {785847, {250, "1 serving", "Popular recipes"}}, // Mexican casserole made with ground beef, beans, tomato sauce, cheese, taco seasonings, and corn chips
        {735254, {250, "1 serving", "Popular recipes"}}, // CHICKEN BREAST MEAT, MEXICAN STYLE RICE, FAJITA STYLE VEGETABLES, BLACK BEANS, CHEESE AND ROASTED CORN FAJITA BOWL, CHICKEN
        {587571, {250, "1 serving", "Popular recipes"}}, // GROUND BEEF CHILI FRESH SOUP
        {704230, {250, "1 serving", "Popular recipes"}}, // TENDER, FRESH HOMEMADE CHICKEN TINGA TACOS TO
        {454467, {60, "1 serving", "Popular recipes"}}, // FRESHLY BAKED CHOCOLATE CROISSANT
        {358266, {69, "1 bagel", "Popular recipes"}}, // ORIGINAL BAGELS WITH CREAM CHEESE
        {484125, {138, "1 bagel - 5 oz", "Popular recipes"}}, // OPEN FACED SALMON BAGEL
        {653798, {69, "1 bagel", "Popular recipes"}}, // FRENCH TOAST BAGELS, FRENCH TOAST
        {456295, {99, "4 pieces", "Popular recipes"}}, // PIZZA BAGELS
        {728442, {99, "3 pieces", "Popular recipes"}}, // CINNAMON BAGEL WITH PUMPKIN SPICE CREAM CHEESE
        {457432, {112, "1 piece", "Popular recipes"}}, // APPLE CRUMBLE PIE
        {574835, {250, "1 bowl", "Popular recipes"}}, // THAI COCONUT CURRY
        
        /*
         *  Popular processed meet
         */
        
        {539593, {65, "1 slice", "Popular processed meet"}}, // Thick cut bacon
        {539593, {25, "1 slice", "Popular processed meet"}}, // Normal cut bacon
        {614616, {27, "1 serving - 1 oz", "Popular processed meet"}}, // All Natural Pork Sausage Links, Lower Sodium, Skinless, CN .1oz., 1/10lb. , Fully Cooked , Certified Gluten- Free
        
        /*
         *  Popular raw meat & fish
         */
        
        {168046, {112, "1 serving", "Popular raw meet & fish"}}, // Fish, Salmon, Chum, raw (Alaska Native)
        {673856, {112, "1 serving", "Popular raw meet & fish"}}, // BONELESS SKINLESS WILD CAUGHT SALMON FILLETS
        {581671, {112, "1 serving", "Popular raw meet & fish"}}, // RIBEYE STEAK GRASSFED ORGANIC BEEF
        {577503, {112, "1 serving", "Popular raw meet & fish"}}, // CHICKEN BREAST TENDERLOINS
        {171509, {100, "1 serving", "Popular raw meet & fish"}}, // Chicken, broilers or fryers, breast, skinless, boneless, meat only, with added solution, raw
        
        /*
         *  Popular sweets and deserts
         */
        
        {167929, {71, "1 muffin", "Popular sweets"}}, // Muffin, blueberry, commercially prepared, low-fat
        {595535, {71, "1 muffin", "Popular sweets"}}, // CHOCOLATE CHUNK MUFFIN
        {807067, {33, "1 serving, 3 cupcakes", "Popular sweets"}}, // PUMPKIN CHEESECAKE MINI CUPCAKES
        {662818, {80, "1 cupcake", "Popular sweets"}}, // CHOCOLATE ICING CUPCAKES, CHOCOLATE
        {407297, {106, "1 serving", "Popular sweets"}}, // BAKERY FRESH GOODNESS, APPLE PIE
        {361028, {113, "1 pie", "Popular sweets"}}, // WAL-MART STORES, SUGAR FREE LEMON PIE
        {455826, {119, "1 serving, 1/9 pie", "Popular sweets"}}, // HOMESTYLE DUTCH APPLE PIE
        {407284, {133, "1 serving, 1/8 pie", "Popular sweets"}}, // BAKERY FRESH GOODNESS, PUMPKIN PIE
        {443792, {150, "1 serving", "Popular sweets"}}, // THE BAKERY, CARROT CAKE
        {484888, {63, "1 donut", "Popular sweets"}}, // FINE CHOCOLATE POWDERED DONUTS
        {517272, {55, "1 piece", "Popular sweets"}}, // APPLE DANISH COFFEE CAKE
        {438919, {55, "1 slice", "Popular sweets"}}, // CHEESE DANISH COFFEE CAKE
        {471978, {80, "1 cupcake", "Popular sweets"}}, // CUPCAKES WITH CREAM CHEESE FILLING
        {474245, {55, "5 pieces - 2 oz", "Popular sweets"}}, // BLUEBERRY CAKE DONUT HOLE
        {690188, {63, "1 donut", "Popular sweets"}}, // CHOCOLATE DONUT, CHOCOLATE
        {536459, {150, "1 serving", "Popular sweets"}}, // CARROT CAKE
        {446806, {119, "1 serving", "Popular sweets"}}, // PIE IN THE SKY BAKERY, PUMPKIN CHEESECAKE
        {489041, {112, "1 serving", "Popular sweets"}}, // QUARK PINEAPPLE
        {687538, {50, "3 pieces", "Popular sweets"}}, // PUMPKIN MAPLE PECAN BAKERY BITES, PUMPKIN MAPLE PECAN
        {493143, {35, "1 cookie", "Popular sweets"}}, // SUNBETT BAKERY, PROTEIN DELIGHTS, WHOLE GRAIN WAFER, PEANUT BUTTER CRISPS
        
        /*
         *  Popular gloceries
         */
        
        {558136, {120, "1 cup, 4 1/4 oz", "Popular gloceries"}}, // 100% ORGANIC WHITE WHOLE WHEAT FLOUR
        
        /*
         *  Popular kinds of bread
         */
        
        {445912, {250 ,"1 loaf, 8 3/4 oz", "Popular kinds of bread"}}, // Baguette
        {505637, {32, "1 slice", "Popular kinds of bread"}}, // WHOLE WHEAT BREAD
        {442903, {28, "1 slice, 1 oz", "Popular kinds of bread"}}, // 100% WHOLE WHEAT BREAD
        {784648, {24, "1 slice, 1 oz", "Popular kinds of bread"}}, // Bread, rye, reduced calorie and/or high fiber
        {554372, {27, "1 slice, 1 oz", "Popular kinds of bread"}}, // LIGHT SEEDLESS RYE BREAD

        /*
         *  Popular snacks
         */
        
        {352278, {28, "1 serving, 1 oz", "Popular snacks"}}, // Ruffles Original Potato Chips 9 Ounce Plastic Bag
        {618008, {28, "1 serving, 1 oz", "Popular snacks"}}, // SWEET POTATO VEGGIE CHIPS, SWEET POTATO
        {352437, {28, "1 bag, 1 oz", "Popular snacks"}}, // Cheetos Fantastix Chili Cheese Corn And Potato Snacks 1.0 Ounce Plastic Bag
        {174780, {55, "1 bar", "Popular snacks"}}, // Formulated bar, MARS SNACKFOOD US, SNICKERS MARATHON Chewy Chocolate Peanut Bar
        {353699, {28, "1 serving, 1 oz", "Popular snacks"}}, // Lays Simply Sea Salted Thick Cut Potato Chips 8.5 Ounce Plastic Bag
        {717232, {28, "1 serving, 1 oz", "Popular snacks"}}, // SWEET & SALTY M&M'S MILK CHOCOLATE AND CRISPY CANDIES, ALMONDS AND PRETZEL BALLS SNACKMIX, SWEET & SALTY
        {551419, {28, "1 serving, 1 oz", "Popular snacks"}}, // ROASTED SALTED PEANUTS THE AMERICAN TRAIL MIX, ROASTED
        {344650, {28, "1 serving, 1 oz", "Popular snacks"}}, // PRINGLES CRISPS FAT FREE SOUR CREAM & ONION
        {170564, {28, "1 serving, 1 oz", "Popular snacks"}}, // Seeds, sunflower seed kernels, oil roasted, without salt
        {170564, {15, "1 serving, 1/2 oz", "Popular snacks"}}, // ORGANIC CHESTNUT POLLEN
        
        /*
         *  Popular cheese
         */
        
        {747108, {19, "1 slice", "Popular cheese"}}, // American cheese
        {588017, {23, "1 slice", "Popular cheese"}}, // SWISS CHEESE SLICES
        
        /*
         *  Popular vegetables
         */
        
        {520405, {100, "1 serving", "Popular vegetables"}}, // SPINACH & SPRING MIX
        {170106, {45, "1 pepper", "Popular vegetables"}}, // Peppers, hot chili, red, raw
        {787814, {90, "1/2 cup", "Popular vegetables"}}, // Radish, raw
        {169228, {100, "1/2 cup", "Popular vegetables"}}, // Eggplant, raw
        {169383, {149, "1 serving", "Popular vegetables"}}, // Peppers, sweet, yellow, raw
        {168482, {112, "1 serving", "Popular vegetables"}}, // Sweet potato, raw, unprepared
        {448473, {67, "1 serving - 1 cup", "Popular vegetables"}}, // CHOPPED KALE
        {543669, {30, "1 serving - 1 cup", "Popular vegetables"}}, // SPINACH
        {371495, {128, "1 serving - 1/2 cup", "Popular vegetables"}}, // WYLWOOD, TURNIP GREENS
        {787220, {100, "1 serving - 1 cup", "Popular vegetables"}}, // Collards, raw
        
        /*
         *  Popular fruits
         */
        
        {600826, {123, "3/4 cup", "Popular fruits"}}, // Mango
        
        /*
         *  Popular pizza
         */
        
        {170701, {155, "1 slice 1/8 per pizza", "Pizza"}}, // School Lunch, pizza, BIG DADDY'S LS 16" 51% Whole Grain Rolled Edge Turkey Pepperoni Pizza, frozen
        
        /*
         *  Popular fast food
         */
        
        {787062, {84, "3 oz", "Popular fast food"}}, // Potato, french fries, fast food
        {639748, {115, "1 sandwich", "Popular fast food"}}, // SAUSAGE, EGG & CHEESE FULLY COOKED SAUSAGE, EGG AND PROCESS AMERICAN SHARP CHEESE ON A BUTTERMILK BISCUIT BREAKFAST SANDWICHES, SAUSAGE, EGG & CHEESE
        {783440, {316, "1 cheeseburger", "Popular fast food"}}, // Cheeseburger, 1 medium patty, with condiments, on bun, from fast food
        {414417, {115, "1 sandwich", "Popular fast food"}}, // ENGLISH MUFFIN SANDWICH
        {560137, {115, "1 sandwich", "Popular fast food"}}, // CANADIAN STYLE TURKEY BACON ENGLISH MUFFIN SANDWICH
        
        /*
         *  Populr sauces & dressing
         */
        
        {799547, {17, "1 tbsp", "Popular sauces"}}, // TOMATO KETCHUP
        {171009, {13, "1 tbsp", "Popular sauces"}}, // Salad dressing, mayonnaise, regular
        {499742, {27, "2 tbsp", "Popular sauces"}}, // SKINNYGIRL, HONEY DIJON SUGAR FREE DRESSING
        
        /*
         *  Popular dairy
         */
        
        {439075, {113, "1/2 cup", "Popular dairy"}}, // FAT FREE COTTAGE CHEESE
        {558839, {240, "1 cup", "Popular dairy"}}, // CULTURED LOWFAT KEFIR
        {168114, {180, "1 serving", "Popular dairy"}}, // Cheese, cottage, lowfat, 1% milkfat, no sodium added
        {168141, {28, "1 serving - 1 oz", "Popular dairy"}}, // Cheese, swiss, low fat
        {358226, {30, "1 serving - 2 tbsp", "Popular dairy"}}, // GREAT VALUE, SOUR CREAM
        {372259, {150, "1 serving - 5.3 oz", "Popular dairy"}}, // ORIGINAL GREEK PINEAPPLE NONFAT YOGURT
        {391805, {100, "1 serving", "Popular dairy"}}, // TRIPLE ZERO NONFAT GREEK YOGURT
        
        /*
         *  Popular canned food
         */
        
        {384432, {112, "4 oz", "Popular canned food"}}, // TUNA FILLETS IN SPRING WATER
        {507557, {112, "4 oz", "Popular canned food"}}, // ACKEES IN SALT WATER
        
        /*
         *  Popular toppings
         */
        
        {806099, {30, "2 tbsp, 1 oz", "Popular toppings"}}, // CARAMEL ZERO SUGAR PUREMADE SYRUP
        {460350, {30, "2 tbsp, 1 oz", "Popular toppings"}}, // PURE MAPLE SYRUP
        {170276, {30, "2 tbsp, 1 oz", "Popular toppings"}}, // Syrup, maple, Canadian
        {596317, {15, "1 tbsp, 1/2 oz", "Popular toppings"}}, // RICH CREAMY HEAVY WHIPPING CREAM
        {496766, {30, "1 tbsp, 1 oz", "Popular toppings"}}, // 100% PURE MAPLE SYRUP

        
        /*
         *  Popular spreads
         */
        
        {525901, {27, "2 tbsp, 1 oz", "Popular spreads"}}, // FAT FREE CREAM CHEESE
        {769648, {27, "2 tbsp, 1 oz", "Popular spreads"}}, // PHILADELPHIA CREAM CHEESE-SOFT PLAIN LIGHTEST 180.000 GR
        {395815, {27, "2 tbsp, 2 oz", "Popular spreads"}}, // CHOCOLATE PROTEIN PEANUT BUTTER
        {364666, {27, "2 tbsp, 2 oz", "Popular spreads"}}, // PEPPER JACK SPREADABLE CHEESE WITH ALMONDS
        {355076, {27, "2 tbsp, 1 oz", "Popular spreads"}}, // PHILADELPHIA CREAM CHEESE MEDITERRANEAN FULL FAT
        
        /*
         *  Popular beverages
         */
        
        {352181, {330, "12 oz", "Popular beverages"}}, // Starbucks Skinny Caramel Macchiato Chilled Expresso Beverage 48 Fluid Ounce Plastic Bottle
        {771706, {450, "16 oz", "Popular beverages"}}, // Honest Tea Green Dragon Glass Bottle, 16 fl oz
        {789437, {400, "14 oz", "Popular beverages"}}, // Coffee, Cappuccino, nonfat
        {771674, {330, "12 oz", "Popular beverages"}}, // Coca-Cola Life Can, 12 fl oz
        {553683, {280, "10 oz", "Popular beverages"}}, // MUSCLE MILK, NON DAIRY PROTEIN SHAKE
        {762418, {330, "12 oz", "Popular beverages"}}, // Mountain Dew Code Red Soda 12 Fluid Ounce
        {789335, {150, "1 cup", "Popular beverages"}}, // Coffee, NS as to type
        
        /*
         *  Popular frozen food
         */
        
        {398347, {270, "1 servnig, 10 oz", "Popular frozen food"}}, // KROGER, ENGLISH MUFFIN BREAKFAST SANDWICH, TURKEY SAUSAGE, EGG & CHEESE
        {405899, {61, "1 servnig", "Popular frozen food"}}, // GOLDEN, FILLED CREPES, BLUEBERRY BLINTZES
        {469487, {75, "1 servnig - 9 pieces", "Popular frozen food"}}, // BELGIAN BOYS, MINI PANCAKES
        {709666, {250, "1 serving", "Popular frozen food"}}, // SAUSAGE, EGG, PASTEURIZED PROCESS AMERICAN CHEESE MAPLE PANCAKE SANDWICHES, SAUSAGE, EGG, AMERICAN CHEESE
        {746542, {151, "1 patty", "Popular frozen food"}}, // BLACK ANGUS BACON & CHEDDAR CHEESE BEEF PATTIES, BLACK ANGUS BACON & CHEDDAR CHEESE
        {344865, {55, "1 waffle", "Popular frozen food"}}, // Kellogg's Eggo Waffles Belgian 5oz 24ct
        {602328, {198, "1 serving", "Popular frozen food"}}, // THE CURRY TIGER BURRITO, THE CURRY TIGER
        {672030, {198, "1 serving", "Popular frozen food"}}, // CHEESE PEPPERONI MINI BAGELS TOPPED WITH CHEESE, PEPPERONI MADE WITH PORK AND CHICKEN ADDED, AND TOMATO SAUCE PIZZA SNACKS, CHEESE
        {794953, {55, "1 cup", "Popular frozen food"}}, // ANTIOXIDANT FRUIT SLICED STRAWBERRIES, CULTIVATED BLUEBERRIES, DARK SWEET CHERRIES, MONTMORENCY SOUR CHERRIES, BLACKBERRIES BLEND
        
        /*
         *  Popular ice cream
         */
        
        {781552, {190, "1 serving, 1 cup", "Popular ice cream"}}, // Ice cream pie, with cookie crust, fudge topping, and whipped cream
        {781550, {175, "1 serving, 1 sundae", "Popular ice cream"}}, // Ice cream sundae, fudge topping, with cake, with whipped cream
        {781550, {103, "1 serving, 1 ice cream", "Popular ice cream"}}, // BLUE BUNNY, PREMIUM ICE CREAM, BANANA SPLIT
        {660314, {103, "1 serving, 1 ice cream", "Popular ice cream"}}, // DUTCH CHOCOLATE LOW FAT ICE CREAM, DUTCH CHOCOLATE
        {362700, {64, "1 serving, 1 ice cream", "Popular ice cream"}}, // FUDGE RICH CHOCOLATE LOW FAT ICE CREAM BAR
        {533680, {64, "1 serving, 2 1/2 oz", "Popular ice cream"}}, // BIRTHDAY CAKE LIGHT ICE CREAM
        {698216, {64, "1 serving, 2 1/2 oz", "Popular ice cream"}}, // COOKIES CREAM WHIPPED GELATO, COOKIES
        
        /*
         *  Popular breakfast food
         */
        
        {172988, {14, "1 serving - 2 tbsp", "Popular breakfast food"}}, // Cereals ready-to-eat, SUN COUNTRY, KRETSCHMER Wheat Germ, Regular
        {545791, {57, "1 serving - 1 bagel", "Popular breakfast food"}}, // ORGANIC BAGELS
        {679950, {57, "1 serving - 1 bagel", "Popular breakfast food"}}, // APPLE CINNAMON FLAVORED LARGER BAKERY STYLE BAGELS, APPLE CINNAMON
        {560304, {190, "1 serving - 7 oz", "Popular breakfast food"}}, // MINI BAGELS TOPPED WITH BACON, EGGS AND CHEESE
        {576661, {240, "1 serving - 9 oz", "Popular breakfast food"}}, // EGG BAGEL STUFFED WITH CHEDDAR CREAM CHEESE!
        
        /*
         *  Popular side dishes
         */
        
        {168880, {180, "1 cup", "Popular side dishes"}}, // Rice, white, medium-grain, enriched, cooked
        {519751, {180, "1 serving", "Popular side dishes"}}, // HOMESTYLE MASHED POTATOES
        {169376, {70, "1 cup", "Popular side dishes"}}, // Mushroom, white, exposed to ultraviolet light, raw
        {168995, {28, "1 cup - 1 oz", "Popular side dishes"}}, // Lambsquarters, raw (Northern Plains Indians)
        {170397, {112, "1 serving", "Popular side dishes"}}, // Cauliflower, cooked, boiled, drained, without salt
        {335290, {112, "1 serving", "Popular side dishes"}}, // Beans, Dry, Pink, 321 (0% moisture)
        
        /*
         *  Popular nutrition bars
         */
        
        {428190, {48, "1 bar", "Popular nutrition bars"}}, // WHOLE NUTRITION BAR FOR WOMEN
        {785145, {80, "1 bar", "Popular nutrition bars"}}, // Nutrition bar (Snickers Marathon Protein Bar)
        
        /*
         *  Popular nutrion powders
         */
        
        {396982, {55, "1 muffin", "Popular nutrition bars"}}, // MIGHTY MUFFIN
        
        /*
         *  Popular japanese food
         */
        
        {786291, {160, "6 rolls", "Popular japanese food"}}, // Sushi roll, salmon
        {386499, {180, "1 bowl", "Popular japanese food"}}, // JAPANESE-STYLE SOUP BOWL WITH COOKED UDON NOODLES
        {582535, {160, "6 rolls", "Popular japanese food"}}, // SUSHI CALIFORNIA ROLL
        {734346, {160, "6 rolls", "Popular japanese food"}}, // PHILADELPHIA ROLL WITH COLD SMOKED SOCKEYE SALMON
        {692300, {180, "1 bowl", "Popular japanese food"}}, // BEEF TERIYAKI JAPANESE STYLE NOODLES WITH SWEET SAUCE
        {745462, {119, "6 pieces", "Popular japanese food"}}, // SALMON SASHIMI
        {701924, {180, "1 bowl", "Popular japanese food"}}, // STIR FRY BLEND RICED CAULIFLOWER WITH STIR FRY VEGETABLES, STIR FRY BLEND
        {396360, {95, "3 pieces", "Popular japanese food"}}, // SHIRAKIKU, WHITE DAIFUKU RED BEAN CAKE
        {396340, {95, "1 cake", "Popular japanese food"}}, // HOKUSHIN, FUBUKI MANJU (WHEAT CAKE)

        /*
         *  Popular seasoning
         */
        
        {805183, {6, "1 tbsp", "Popular nutrition food"} }, // SEAWEED SPRINKLE
        
        /*
         *  Popular nutrition food
         */
        
        {580904, {5, "1 stick", "Popular nutrition food"}}, // LOW CALORIE VITAMIN MINERAL DRINK MIX
        {424579, {236, "1 serving - 8 oz", "Popular nutrition food"}}, // VITA-J, VITAMIN JUICE, NATURALLY SWEETENED NUTRITIONAL DIET JUICE BEVERAGE FROM CONCENTRATE, POMEGRANATE WITH BLUEBERRY
        {552939, {14, "1 serving - 1/2 oz", "Popular nutrition food"}}, // KOMBUCHA PROBIOTIC DRINK MIX
        {718164, {25, "1 serving - 2 scoops", "Popular nutrition food"}}, // CLEAN LEAN PROTEIN, SMOOTH VANILLA
        {348019, {280, "1 serving - 10 oz", "Popular nutrition food"}}, // Kellogg's Special K Protein Liquids Milk Chocolate 10fl oz
        {172251, {58, "1 serving - 2 oz", "Popular nutrition food"}}, // Babyfood, meat, turkey, junior
        {172874, {27, "1 serving - 1 oz", "Popular nutrition food"}}, // Turkey, skin, from retail parts, from dark meat, cooked, roasted
        {348019, {280, "1 serving - 10 oz", "Popular nutrition food"}}, // Kellogg's Special K Protein Liquids French Vanilla 10fl oz
        {473550, {32, "1 serving - 1 cake", "Popular nutrition food"}}, // MINI VITACAKES
        {173481, {58, "1 serving  - 2 oz", "Popular nutrition food"}}, // Babyfood, meat, lamb, strained
        {348300, {120, "1 serving", "Popular nutrition food"}}, // PEDIALYTE MIXED FRUIT 1 LITER BOTTLE 4 COUNT RETAIL
        {350849, {25, "1 serving", "Popular nutrition food"}}, // PROPEL POWDER CITRUS PUNCH   0.8Z
        {375097, {25, "1 serving", "Popular nutrition food"}}, // SODIUM-FREE SALT
        {447383, {3, "1 serving", "Popular nutrition food"}}, // ROUNDY'S, BAY LEAVES
        {492975, {3, "1 cup - 3G", "Popular nutrition food"}}, // AMAZONAS RAINFOREST PRODUCT, 100% NATURAL EMOLIENTE MIXED HERB TEA
        {501088, {500, "1 serving - 16.9 oz", "Popular nutrition food"}}, // WAT-AAH! POWER, VAPOR DISTILLED WATER   MAGNESIUM
        {530121, {480, "1 serving - 16 oz", "Popular nutrition food"}}, // SUAVE BLEND MILD YERBA MATE TEA, HERBAL COFFEE ALTERNATIVE serving
        {578584, {240, "1 serving - 8 oz", "Popular nutrition food"}}, // ANTIOXIDANT WATER
        {593876, {335, "1 serving - 12 oz", "Popular nutrition food"}}, // SPARKLING PASSION-RASPBERRY HAPPY FACE INFUSED TEA
        {355778, {7, "1 serving - 1/2 cup", "Popular nutrition food"}}, // PLUM PUFFS BABY FOOD BLUEBERRY PURP CARROT SWT POT
        {517399, {237, "1 serving - 9 oz", "Popular nutrition food"}}, // SPARKLING NATURAL MINERAL WATER
        {690236, {52, "1 serving - 1.96 oz", "Popular nutrition food"}}, // STRAWBERRY WATERMELON DRINK MIX, STRAWBERRY WATERMELON
        {748278, {14, "1 serving - 1 tbsp", "Popular nutrition food"}}, // Oil, canola
        {764800, {300, "1 serving - 12 oz", "Popular nutrition food"}}, // Tropicana Trop50 Pomegranate Blueberry 12 Fluid Ounce Plastic Bottle Single
        {777178, {112, "1 serving - 4 oz", "Popular nutrition food"}}, // ORGANIC KOMBU FLAKES
    }),

    food_catalogue_ ({
        
        
    {"Beef / Stock","beef stock"}, //  Говяжий холодец
    {"Beef / Broth","beef broth"},   //  Говяжье заливное
    {"Beef / Smoked jerky","beef smoked jerky", "stock broth"},
    {"Beef / Smoked jerky teriyaki","beef smoked jerky teriyaki", "stock broth"},
    {"Beef / Just smoked","beef smoked", "stock broth jerky teriyaki sausage frank brats sticks sauce", {},{{1005,10}}}, // Just smoked beef
    {"Beef / Smoked with seasoning or sauce","beef smoked",
        "stock broth jerky teriyaki sausage frank brats sticks sauce", {{1005, 9}}, {}}, // Smoked beef with seasoning/sauce
    {"Beef / Smoked sausage","beef smoked sausage"},    // Smoked beef sausage
    {"Beef / Smoked franks","beef smoked frank"},      // Smoked beef franks
    {"Beef / Smoked brats","beef smoked brats"},       // Smoked beef brats
    {"Beef / Smoked sticks","beef smoked sticks"},      // Smoked beef sticks
    {"Beef / Smoked with sauce ","beef smoked sauce"},       // Smoked beef with sauce
    {"Beef / Cumbles","beef crumbles", "smoked stock broth jerky"},  // Beef crumbles
    {"Beef / Gravy - only beef","beef gravy","smoked stock broth jerky crumbles mix", {}, {{1005,7}}}, // Beef gravy (no or little carb additions)
    {"Beef / Gravy - with sides","beef gravy","smoked stock broth jerky crumbles mix cooked",{{1005,6}}, {}}, // Beef gravy with vegetables on the side
    {"Beef / Gravy - powerd mixes","beef gravy mix","smoked stock broth jerky crumbles"}, // Powder for the beef gravy
    {"Beef / Fully cooked gravy ","beef gravy cooked","smoked stock broth jerky crumbles"}, // Fully cooked beef grazy (a plate)
    {"Beef / Raw pieces","beef raw animal", "smoked stock broth jerky crumbles gravy"}, // Raw beef - different kinds of
    {"Beef / Patties without additions","beef patties", "", {}, {{1093,100}}}, // Beef patties - like cutlets, raw, no salt or additions
    {"Beef / Patties and salt or bacon","beef patties", "", {{1093,99}}, {}}, // Beef patties - like cutlets, raw, with salt or additions (like bacon)
    {"Beef / Raw burgers","beef burger", "", {}, {{1005,5}}}, // Burgers, raw, no buns
    {"Beef / Raw burgers with buns","beef burger","helper dinner meal", {{1005,4}}, {}}, // Burgers with buns
    {"Beef / Burger helpers","beef burger helper","dinner meal", {{1005,4}}, {}}, // Helpers for burgers
    {"Beef / Full dinner mix with burgers","beef burger dinner","", {{1005,4}}, {}}, // Dinner mix with burgers
    {"Beef / Dinner mix with beef","beef burger meal","", {{1005, 4}}, {}}, // Dinner mix with parts of burgers
    {"Beef / Round parts of raw beef","beef round",
        "burger patties smoked stock broth jerky crumbles gravy animal ground", {}, {{1005,5}}}, // Different kinds of raw beef - the round part
    {"Beef / Roundy's","beef round",
        "burger patties smoked stock broth jerky crumbles gravy animal ground", {{1005,4}}, {}}, // Pavioli/macaroni/skillets from Roundy
    {"Beef / Ground raw beef","beef ground",
        "burger patties smoked stock broth jerky crumbles gravy animal",{}, {{1005,5}}}, // Ground beef, raw, no additions
    {"Beef / Ground raw beef with additions or vegan ground beef","beef ground",
        "burger patties smoked stock broth jerky crumbles gravy animal", {{1005,4}}, {}}, // Ground beef, raw, with additions or from vegan sources
    {"Beef / Raw beef","beef",
        "burger patties smoked stock broth jerky crumbles gravy animal round ground", {}, {{1005,2},{1093,120}}}, // Different kinds of raw beef
    {"Beef / Beef products with sides","beef",
        "burger patties smoked stock broth jerky crumbles gravy animal round ground", {{1005,1}}, {}}, // Different products from beef with carb additions
    {"Beef / Beef sausage and the like","beef",
        "burger patties smoked stock broth jerky crumbles gravy animal round ground", {{1093, 120}}, {{1005, 2}} }, // Different products from beef with salt but without carb additions
        
    {"Vegetables",
            "",
            "Pokeberry Melons Lemon Carambola Dewberries youngberries Boysenberries Blackberries Nectarines Rose-apples Grapefruit Raspberr Lime Longans Oranges",
            {}, {{2000,6},{1005,22},{1004,2},{1003,4},{1008,90},{1093,150}},
            {", raw"},
            {"Berries","Strawberries","Cranberries","Gooseberries","Pummelo","Kiwifruit","Salmonberries","Prickly%20pears","Acerola","Grapes,%20muscadine","Jujube","Oheloberries","Java-plum","Pollock","Haddock","Cantaloupe","Pears,%20raw","Peaches","Elderberries","Huckleberries","Mammy-apple","Bananas","Cranberry","Crabapples","strawberry","Sapodilla"},
            {169230, 776792, 571705, 580251, 484123, 566781, 688642, 492392, 508048, 448194, 473059, 706290, 449607, 487080, 386721, 487474, 578595, 507940, 735154, 466304, 578596, 474184, 468699, 505953, 596822, 587298, 736392, 487480, 528882, 168482}
    }, // Vegetables
    })

/*
 
 vegetables:
 
 http://localhost:9999/food_by_rank?inc_exact=%22,%20raw%22&nutrient_limits=%222000%22,%22%3C%22,%226%22,%221005%22,%22%3C%22,%2222%22,%221004%22,%22%3C%22,%222%22,%221003%22,%22%3C%22,%224%22,%221008%22,%22%3C%22,%2290%22,%221093%22,%22%3C%22,%22150%22&exc=Pokeberry%20Melons%20Lemon%20Carambola%20Dewberries%20youngberries%20Boysenberries%20Blackberries%20Nectarines%20Rose-apples%20Grapefruit%20Raspberr%20Lime%20Longans%20Oranges&exc_exact=%22Berries%22,%22Strawberries%22,%22Cranberries%22,%22Gooseberries%22,%22Pummelo%22,%22Kiwifruit%22,%22Salmonberries%22,%22Prickly%20pears%22,%22Acerola%22,%22Grapes,%20muscadine%22,%22Jujube%22,%22Oheloberries%22,%22Java-plum%22,%22Pollock%22,%22Haddock%22,%22Cantaloupe%22,%22Pears,%20raw%22,%22Peaches%22,%22Elderberries%22,%22Huckleberries%22,%22Mammy-apple%22,%22Bananas%22,%22Cranberry%22,%22Crabapples%22,%22strawberry%22,%22Sapodilla%22
 
 vegetables, alternative version:
 
 http://localhost:9999/food_by_rank?nutrient_limits=%222000%22,%22%3C%22,%225%22,%221005%22,%22%3C%22,%2210%22,%221004%22,%22%3C%22,%222%22,%221003%22,%22%3C%22,%223%22,%221008%22,%22%3C%22,%2250%22,%221093%22,%22%3C%22,%22150%22&inc=raw&exc=STRAWBERR%20drink%20COOKED%20BOILED%20SMOOTHIE


+ garlic 169230
776792, BABY DUTCH YELLOW POTATOES
571705, YELLOW POTATOES
white onion ?
580251, 100% NATURAL BROCCOLI FLORETS
baby spinach ?
484123, CAESAR SALAD KIT
566781, BABY ROMAINE SALAD GREENS
688642, ROMAINE SALAD
492392, SUGAR SNAP PEAS
508048, ORGANIC SPRING MIX SALAD
roma tomatoes ?
448194, CORN ON THE COB
397580, SWEET PETITE CARROTS
473059, BROCCOLI CUTS
706290, CELERY HEARTS
cabbage head ?
449607, MINI SWEET PEPPERS
487080, ROMAINE HEARTS LETTUCE
386721, ANGEL HAIR COLESLAW
487474, TRI-COLOR COLESLAW
578595, BABY RED POTATOES
507940, ENGLISH CUCUMBER CHIPS
735154, MATCHSTICK CARROTS
466304, CELERY STICKS
578596, BABY GOLD POTATOES
474184, BABY BELLA MUSHROOMS
468699, CHOPPED SPINACH
505953, ORANGE PEPPER
596822, KALE CRANBERRY SALAD
587298, KALE CRANBERRY PECAN CHOPPED COMPLETE SALAD KIT
736392, BUTTER LETTUCEInositolti
487480, SHREDDED ICEBERG LETTUCE
528882, CHICKEN CAESAR, SALAD BOWL*/

{
    std::cerr << "foods::foods" << std::endl;
}

} // namespace balanced_diet


/*
 
 Anxiety
 
 Chromium       1096    35mg
 Folate         1177    400
 B8 (Inositol)  1181    2000mg
 Choline        1180
 Serine         1227    8g
 Copper         1098
 Magnesium      1090
 Selenium       1103
 Zinc           1095
 Vitamin B-6    1175
 Vitamin B-3 (Niacin)   1167
 Vitamin D      1110    4000
 Vitamin E      1124    22.5
 Carnitine
 
 Depression
 
 Chromium       1096    35mg
 Folate         1177    400
 Vitamin B-12
 Vitamin B-6    1175
 Vitamin B-2
 Vitamin D      1110
 Carnitine
 Inositol
 Biotin
 Antioxidants
 Serine
 Zinc           1095
 Magnesium      1090
 Selenium       1103
 
 Insomnia
 
 Vitamin B-3    1167
 Folate         1177
 Vitamin B-6    1175
 Vitamin B-12
 Magnesium      1090
 Zinc           1095
 Copper         1098
 Oleic Acid
 Vitamin A      1104
 Vitamin B-1
 
 Asthma
 
 Folate
 Choline
 Magnesium
 Selenium
 Zinc
 Vitamin A
 Vitamin B-6
 Vitamin C
 Vitamin D
 Vitamin E
 Coenzyme Q10
 Carnitine
 
 Autium
 
 добавить
 Glutamine (protein)    15g     Beef
 Cysteine (protein)     2.2g    Egg white
 Glutathione (antioxidant)  250mg   Red pepper or asparagus
 
 Diabetes
 
 добавить
 Glutamine
 Cysteine
 Glutathione
 Lipoic Acid (antioxidant)    1800mg  organ meats
 
 Fatigue
 
 добавить
 Asparagine                         Asparagus or eggs or beef
 Glutamine
 CoQ10
 Antioxidants (Cysteine, a-Lipoic acid, Glutathione)
 
 Headaches
 
 добавить
 Calcium                1000mg      4oz hard cheese or mozarella or 1.5 kg of cottage cheese or 700g of greek yogurt
 Glutathione
 CoQ10
 Lipoic Acid (antioxidant)
 
 Hypertension
 
 добавить
 CoQ10
 Glutathione
 Lipoic Acid (antioxidant)
 
 Inflammation
 
 добавить
 Managese               2.3mg       Mussels or oysters 6oz or clams 7oz or spinach 3/2 cup (45g) or black tea 5 cup
 CoQ10
 Glutamine
 Lipoic Acid (antioxidant)
 
 Testosterone
 
 добавить
 Vitamin K              100mcg      Kale or mustard greens or swiss chard or collard greens or spinach or broccoli or brussels sprouts or beef liver or pork 200g or chicken 200g or hard cheese 120g or soft cheese 200g
 
 Weight management
 
 добавить
 Asparagine
 Calcium
 Vitamin B-5            5mg         Beef liver or chicken breast 400g or tuna 400g
 Glutamine
 Lipoic Acid (antioxidant)
 
 Anxiety + Depression + Insomnia + Asthma

 Chromium       1096    35mg        Витамины
 + Folate         1177    400         Витамины or Beef liver (6oz) or 1/5 cup spinach
 Choline        1180    425mg       Витамины or Beef (4.5 oz) liver or 4 large eggs
 + Serine         1227    8g          (protein) two egg whites
 Copper         1098                Витамины or Beef liver (5g) ot oysters
 + Magnesium      1090                Витамины
 + Selenium       1103    55mg        Витамины or 1 Brazil nut or tuna fish or 150g shrimp or 200g beef or 4 large eggs or 600g cottage cheese or 9 egg whites (270g)
 + Zinc           1095                Витамины or oysters or 200g beef
 Vitamin A      1104                Витамины or Beef liver (12g) or 3/4 cup of spinach
 + Vitamin B-1 (Thiamine)    1165     Витамины or 240g pork or 240g salmon
 + Vitamin B-2 (Riboflavin)   1166    Витамины or Beef liver (42g) or 2 cup yogurt or 250g beef or 250g clams
 + Vitamin B-3 (Niacin)   1167        Витамины or Beef liver (3.5 oz) or chicken breast 5oz or turkey breast 5oz or sockeye salmon 6oz or tuna 6oz or pork 7oz
 + Vitamin B-6    1175                Витамины or tuna yellowfish 6oz or salmon sockeye 9oz
 + (15%) Vitamin B-7 (Biotin)         1176  Витамины or Beef liver (3oz) or three eggs
 Vitamin B-8 (Inositol) 1181    2000mg  Витамины or soy beans 100g
 + Vitamin B-12   1178                Витамины or Beef liver (2.7g) or clams or trout, rainbow, wild or salmon, sockeye or tuna light or haddock or nutritional yeasts or beef 6oz
 + Vitamin D      1110    4000        Витамины
 + Vitamin E      1124    22.5        Витамины or wheat germ oil 1 tbspn
 + Vitamin C      1162                Витамины or 1/2 cup red pepper or guava 1 cup
 Carnitine              500-2000mg  Витамины
 Antioxidants
 Oleic Acid (one of Omega-9)        2 tblspn olive oil or macadamia oil or canola oilor avodaco oil or almond oil or sunflower oil
 Coenzyme Q10           600mg       Pork heart or beef heart or liver products
 
 Pain
 
 Choline
 + Magnesium
 Copper
 Selenium
 + Zinc
 + Vitamin B-1
 + Vitamin B-2
 + Vitamin B-6
 Vitamin B-8 (Inositol)
 + Vitamin B-12
 + Vitamin D
 + Vitamin E
 + Vitamin C
 Carnitine
 Lipoic Acid (bound to lysine)  2-3g    beef or chicken breast or pork or tuna or crab or tofu or low-fat ricotta
 Cysteine (protein)                     pork or beef or chicken breast or tuna or egg or low-fat yogurt
 Oleic Acid
 
 http://balanced-foods.com/balance?foods=%22170305%22,%22124%22&nutrients=%221227%22,%22%3E%22,%225%22,%221181%22,%22%3E%22,%222000%22,%221090%22,%22%3E%22,%22410%22,%221095%22,%22%3E%22,%2211%22,%221096%22,%22%3E%22,%2235%22,%221098%22,%22%3E%22,%220.9%22,%221103%22,%22%3E%22,%2255%22,%221110%22,%22%3E%22,%22600%22,%221124%22,%22%3E%22,%2222.5%22,%221167%22,%22%3E%22,%2216%22,%221175%22,%22%3E%22,%221.3%22,%221177%22,%22%3E%22,%22400%22,%221180%22,%22%3E%22,%22550%22,%221008%22,%22%3C%22,%222000%22,%221093%22,%22%3C%22,%222300%22,%221253%22,%22%3C%22,%22300%22,%221257%22,%22%3C%22,%222%22&groups=%22Breakfast%22,%22170305%22&mr=%221%22,%2265%22,%22170%22,%2241%22,%221%22,%222%22&target_weight=0&target_weight_deadline=0&recipes=
 
 
 http://balanced-foods.com/balance?foods=%22170305%22,%22124%22,%22172876%22,%22125%22,%22168308%22,%22210%22,%22603128%22,%22350%22,%22169080%22,%22120%22,%22169251%22,%22200%22,%22175138%22,%2230%22,%22320483%22,%22200%22,%22169276%22,%22150%22,%22787791%22,%22300%22,%22168429%22,%2275%22,%22169387%22,%2250%22,%22602465%22,%2210%22,%22169284%22,%2210%22,%22173111%22,%22360%22,%22173417%22,%22350%22,%22170562%22,%2210%22,%22169414%22,%225%22&nutrients=%221227%22,%22%3E%22,%225%22,%221181%22,%22%3E%22,%222000%22,%221090%22,%22%3E%22,%22410%22,%221095%22,%22%3E%22,%2211%22,%221096%22,%22%3E%22,%2235%22,%221098%22,%22%3E%22,%220.9%22,%221103%22,%22%3E%22,%2255%22,%221110%22,%22%3E%22,%22600%22,%221124%22,%22%3E%22,%2222.5%22,%221167%22,%22%3E%22,%2216%22,%221175%22,%22%3E%22,%221.3%22,%221177%22,%22%3E%22,%22400%22,%221180%22,%22%3E%22,%22550%22,%221008%22,%22%3C%22,%221500%22,%221093%22,%22%3C%22,%222300%22,%221253%22,%22%3C%22,%22300%22,%221257%22,%22%3C%22,%222%22&groups=%22Breakfast%22,%22170305%22,%22172876%22,%22168308%22,%22603128%22,%22169080%22,%22169251%22,%22175138%22,%22320483%22,%22169276%22,%22787791%22,%22168429%22,%22169387%22,%22602465%22,%22169284%22,%22173111%22,%22173417%22,%22170562%22,%22169414%22&mr=%221%22,%2265%22,%22170%22,%2241%22,%221%22,%222%22&target_weight=0&target_weight_deadline=0&recipes=
 
 
 
 */
