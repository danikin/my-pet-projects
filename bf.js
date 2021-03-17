    var arr = [];
    var currentFocus;

    var nutrient_names = [];
    var nutrient_ids = [];
    var nutrient_limits = new Map();

    var food_standard_weights = new Map();
    var food_standard_units = new Map();
    var food_objects = new Map();

    var catalogue_item_names = [];
    var catalogue_item_ids = [];
    var catalogue_example_food_ids = new Map();
    var current_catalogue_item_id = -1;

/*
 
 Sweets under 200 cal
 
/food_by_rank?limitsort=1000&nutrient_limits=%221008%22,%22%3C%22,%22200%22,%222000%22,%22%3E%22,%2210%22&inc=&exc=yogurt%20protein%20bars%20CHICKEN%20scallop%20MUSHROOM%20beef%20TERIYAKI%20beans%20SQUID%20capers%20SAUCE%20HORSERADISH%20VEGETABLE%20BALSAMIC%20SPICY%20GARLIC%20SALSA%20PORK
*/

var show_distance_to = -1;

    function closeAllLists(inp, elmnt) {
      /*close all autocomplete lists in the document,
      except the one passed as an argument:*/
      var x = document.getElementsByClassName("autocomplete-items");
      for (var i = 0; i < x.length; i++) {
        if (elmnt != x[i] && elmnt != inp) {
            x[i].parentNode.removeChild(x[i]);
        }
      }
    }
          
    function processResult(old_food_id, url_params, inp, result)
    {
        //console.log(result);
        
        let arr = result.data.map(x => x.food);
        let arr_ids = result.data.map(x => x.food_id);
        
        food_standard_weights = new Map();
        for (let x of result.data)
            food_standard_weights.set(x.food_id, Number(x.weight));
        food_standard_units = new Map();
        food_objects = new Map();
        for (let x of result.data)
        {
            food_standard_units.set(x.food_id, x.standard_unit);
            food_objects.set(x.food_id, x);
        }

        //console.log(arr_ids);
        
        autocomplete2(old_food_id, url_params, inp, arr, arr_ids);
    }

// Fills nutrient info in local arrays and the map
function fill_local_nutrient_info(result)
{
    if (nutrient_names.length === 0)
        nutrient_names = result.data.map(x => x.nutrient_name);
    if (nutrient_ids.length === 0)
        nutrient_ids = result.data.map(x => x.nutrient_id);
    if (nutrient_limits.size === 0)
    {
        for (let x of result.data)
        {
            nutrient_limits.set(x.nutrient_id, Number(x.nutrient_lower_limit));
        }
    }
}
    function processResultNutrients(old_food_id, url_params, inp, result)
    {
        //console.log(processResultNutrients);
        
        // Receive nutrients from the server only once
        fill_local_nutrient_info(result);
        
        let arr = nutrient_names;
        let arr_ids = nutrient_ids;
        autocomplete2(old_food_id, url_params, inp, arr, arr_ids);
    }

    function processResultCatalogue(old_food_id, url_params, inp, result)
    {
    //console.log(processResultNutrients);
    
    // Receive nutrients from the server only once
        if (catalogue_item_names.length === 0)
            catalogue_item_names = result.data.map(x => x.catalogue_item_name);
        if (catalogue_item_ids.length === 0)
            catalogue_item_ids = result.data.map(x => x.catalogue_item_id);
        if (catalogue_example_food_ids.size === 0)
        {
            for (let x of result.data)
                catalogue_example_food_ids.set(x.catalogue_item_id, Number(x.example_food_id));
        }
    
        let arr = catalogue_item_names;
        let arr_ids = catalogue_item_ids;
        autocomplete2(old_food_id, url_params, inp, arr, arr_ids);
    }

function bigImg(x) {
  x.style.height = "128px";
  x.style.width = "128px";
}

function normalImg(x) {
  x.style.height = "24px";
  x.style.width = "24px";
}


function show_food_comparison_chart(arr_ids)
{
    let chart_data = [['Distance, %', 'Protein', 'Fat', 'Carbs', 'Fiber', 'Cal', 'Sod']];
    
    for (let food_id of arr_ids)
    {
        let fo = food_objects.get(food_id);
        let ntr = fo.food.split('$$$')[1].split(' ');
        chart_data.push([Number((1-fo.distance)*100),
                         Number(ntr[1]),
                         Number(ntr[3]),
                         Number(ntr[5]),
                         Number(ntr[7]),
                         Number(ntr[9]/10),
                         Number(ntr[11]/10)]);
    }
    document.getElementById('curve_chart_floating').style.visibility = 'visible';
    
    new google.visualization.LineChart(document.getElementById('curve_chart_floating')).draw(google.visualization.arrayToDataTable(chart_data), {
title: 'Nutrient distribution',
curveType: 'function',
legend: { position: 'bottom' }
});
}

    function autocomplete2(old_food_id, url_params, inp, arr, arr_ids) {
        
        //console.log(arr_ids);
        
        // If this is a substitition search then show the comparison chart
        // Note: don't show this chart because it only confues a user
        //if (show_distance_to !== -1)
        //    show_food_comparison_chart(arr_ids);
                
        let a, b, i, val = inp/*this*/.value;
        
        //console.log('val=', val);
        
        let words = val.split(' ');
        
        /*close any already open lists of autocompleted values*/
        closeAllLists(inp);

        // In the show_distance_to mode don't skip on empty vals
        if (show_distance_to == -1)
        {
            if (!val)
            {
                inp.placeholder = "Enter search words";
                
                // Don't skip on empty vals here either because it might be interesting for a user to
                // see variants without entering a word
                // return false;
            }
        }
        else
        {
            //console.log(`val=${val}, placeholder=${inp.placeholder}`);
            if (!val)
                inp.placeholder = "Enter search words";
        }
        
        currentFocus = -1;
        /*create a DIV element that will contain the items (values):*/
        a = document.createElement("DIV");
        a.setAttribute("id", inp/*this*/.id + "autocomplete-list");
        a.setAttribute("class", "autocomplete-items");
        /*append the DIV element as a child of the autocomplete container:*/
        inp/*this*/.parentNode.appendChild(a);
        /*for each item in the array...*/
        for (i = 0; i < arr.length; i++) {
            
            /*// Search for every word
            var inds = [];
            var ind;
            for (let word of words)
            {
                ind = arr[i].toUpperCase().indexOf(word.toUpperCase());
                if (ind == -1)
                    break;
                inds.push(ind);
            }
            if (ind == -1)
                continue;
                
            // Sort all indexes
            inds.sort();
            
            //console.log(words);
            //console.log(inds);*/
            
            // Strong every word
            let txt = arr[i];
            let is_match = false;
            for (let word of words)
            {
                if (word.length === 0)
                    continue;
                // (?<!<[^>]*) - this is to prevent from replacing parts of "strong" - it will not
                //  search strings inside tags
                const regex = new RegExp(`((?<!<[^>]*)${word})`, "gi");
                let txt2 = txt.replace(regex, '<strong>$1</strong>');
                if (txt2 !== txt)
                    is_match = true;
                txt = txt2;
            }
            
            // If this item does not match at least one word then skip it
            // Note: NEVER skip the first item for show_distance_to search - because
            //  it's the item which we try to find a substitution for
            // Note: we show everything without words filter by default for the show_distance_to search
            if (show_distance_to != -1)
            {
                if (!is_match && val != '' && i)
                    continue
            }
            else
            if (!is_match)
            {
                //console.log(`no match: for arr[${i}]='${arr[i]}'`);
                
                // Don't skip if we mismatch on empty search string - a user might want to see it all
                if (val != '')
                    continue;
            }
            //console.log(txt);
            
            if (food_standard_units.get(Number(arr_ids[i])) !== undefined)
                txt = txt.replace("$$$", `${food_standard_units.get(Number(arr_ids[i]))} <span style="font-size: 11px"> `) + "</span>";
            else
                txt = txt.replace("$$$", `<span style="font-size: 11px"> `) + "</span>";
            
            // For the catalogue
            if (old_food_id === 111111111)
            {
                txt += `<img src="/food_pics?file=${catalogue_example_food_ids.get(Number(arr_ids[i]))}.jpeg" width="24" height="24" onmouseover="bigImg(this)" onmouseout="normalImg(this)" onerror="this.style.display='none'"/>`;
            }
            else
            {
                txt += `<img src="/food_pics?file=${arr_ids[i]}.jpeg" width="24" height="24" onmouseover="bigImg(this)" onmouseout="normalImg(this)" onerror="this.style.display='none'"/>`;
                
                // For the food - add rank
                if (old_food_id === 999999999)
                {
                    if (show_distance_to !== -1 && i === 0)
                        txt = "Substitute: <span style=\"color: red\">" + txt + "</span>";
                    
                    let fo = food_objects.get(Number(arr_ids[i]));
                    
                    // Rank
                    if (fo !== undefined && fo.food_rank !== undefined)
                    {
                        //txt += ` <a href="/nutrition?food=${arr_ids[i]}" target="blank">${fo.food_rank.toFixed(2)}</a>`;
                        
                        if (fo.food_rank < 1.0)
                            txt += ` <img src="/food_pics?file=green_leaf.jpeg" width="24" height="24"/>`;
                    }
                    
                    // Distance and common nutrients
                    if (fo !== undefined && fo.distance !== undefined)
                    {
                        txt += ` ${(100-fo.distance*100).toFixed()} %`;
                    }
                    if (fo !== undefined && fo.common_nutrients !== undefined)
                    {
                        txt += ` ${fo.common_nutrients}`;
                    }
                    
                    // Satiety - if it's bigger than 5 (whte bread) then green otherwise - red
                    if (fo !== undefined && fo.satiety !== undefined)
                    {
                        if (fo.satiety >= 5)
                            txt = `<span style="color: green">${txt}</span>`;
                        else
                            txt = `<span style="color: red">${txt}</span>`;
                    }
                }
            }

            
          /*check if the item starts with the same letters as the text field value:*/
          //if (arr[i].substr(0, val.length).toUpperCase() == val.toUpperCase()) {
          //var ind = arr[i].toUpperCase().indexOf(val.toUpperCase());
          //if (ind != -1) {
          
          // Form a string that strongs first occurencences of each word
          /*var txt = "";
          var cur_ind = 0;
          var w = 0;
          for (ind of inds) {
              txt += arr[i].substr(cur_ind, ind) + "<strong>" + arr[i].substr(ind, words[w].length) + "</strong>";
              cur_ind = ind + words[w].length;
              ++w;
          }
          txt += arr[i].substr(cur_ind);*/
          
         // if (true) {
            /*create a DIV element for each matching element:*/
            b = document.createElement("DIV");
            /*make the matching letters bold:*/
            // b.innerHTML = "<strong>" + arr[i].substr(0, val.length) + "</strong>";
            b.innerHTML = txt;//arr[i].substr(0, ind) + "<strong>" + arr[i].substr(ind, val.length) + "</strong>";
            //b.innerHTML += arr[i].substr(ind + val.length);
            /*insert a input field that will hold the current array item's value:*/
            b.innerHTML += "<input type='hidden' value='" + arr[i] + "'>";
            b.innerHTML += "<input type='hidden' value='" + arr_ids[i] + "'>";
            /*execute a function when someone clicks on the item value (DIV element):*/
                b.addEventListener("click", function(e) {
                    
                // Remove the floating chart from the screen
                document.getElementById('curve_chart_floating').innerHTML = '';
                document.getElementById('curve_chart_floating').style.visibility = 'hidden';
                    
                /*insert the value for the autocomplete text field:*/
                //console.log(inp.getElementsByTagName("input"));

                let food_or_nutrient_id = this.getElementsByTagName("input")[1].value;
                    
                // If a user picks a food with id show_distance_to then this means that
                //  she/he don't substitute it for now - so reset show_distance_to and do nothing
                if (Number(food_or_nutrient_id) === Number(show_distance_to))
                {
                    document.getElementById("radio_food_" + food_or_nutrient_id).checked = false;
                    show_distance_to = -1;
                    inp.value = "";
                    return;
                }
                
                // For the catalogue remember selected value in the input
                if (old_food_id === 111111111)
                    inp.value = this.getElementsByTagName("input")[0].value
                else
                    inp.value = '';
                /*close the list of autocompleted values,
                (or any other open lists of autocompleted values:*/
                closeAllLists(inp);
                    
                // If this is the catalogue
                if (old_food_id === 111111111)
                {
                    current_catalogue_item_id = Number(food_or_nutrient_id);
                }
                else
                {
                    
                    
                // Replace the food id in params
                //console.log(old_food_id);
                //console.log(food_id);
                const regex = new RegExp(old_food_id, "gi");
                let new_params = url_params.replace(regex, food_or_nutrient_id);
                    
                // If there is $$$$ in the URL then this is a placeholder for the
                // nutrient limit
                    //console.log(food_or_nutrient_id);
                new_params = new_params.replace("$$$$", nutrient_limits.get(Number(food_or_nutrient_id)));
                

                //new_params = new_params.replace("^^^", food_standard_weights.get(Number(food_or_nutrient_id)));
                
                // New food
                if (new_params.indexOf("^^^") != -1)
                {
                    //console.log(food_or_nutrient_id, show_distance_to);

                    let new_amount = food_current_values.get(Number(food_or_nutrient_id));
                    let added_amount = Number(food_standard_weights.get(Number(food_or_nutrient_id)));
                    if (new_amount === undefined)
                        new_amount = added_amount;
                    else
                        new_amount += added_amount;
                    
                    if (new_amount !== undefined)
                    {
                        // If there was a substitution then delete the substituted food
                        if (show_distance_to != -1)
                        {
                            
                            // Remove the food from the map and from ALL meal groups
                            console.log(`remove food ${show_distance_to}`);
                            food_current_values.delete(show_distance_to);
                            for (let [meal_group_name, meal_group] of meal_groups)
                            {
                                let i = 0;
                                for (let x of meal_group)
                                {
                                    if (x == show_distance_to)
                                    {
                                        meal_group.splice(i, 1);
                                        break;
                                    }
                                    ++i;
                                }
                            }
                        } // if (show_distance_to != -1)
                        
                        
                        let meal_group_name = document.getElementById("myInputMealGroup").value;
                        
                        // Add the food to the recipe
                        if (meal_group_name === 'to_the_recipe')
                        {
                            add_food_to_recipe(food_or_nutrient_id, new_amount);
                        }
                        // Add the food to the meal group and reload the page
                        else
                        {
                            console.log(`search ${food_or_nutrient_id} in meal groups`);
                            // Don't add new food to the meal group if its already in one of the meal groups
                            let found = false;
                            for (let [unused, foods] of meal_groups)
                            {
                                for (let food_id of foods)
                                    if (food_id == Number(food_or_nutrient_id))
                                    {
                                        found = true;
                                        break;
                                    }
                            }
                            if (!found)
                            {
                                console.log(`not found ${food_or_nutrient_id} in meal groups`);
                                let mg = meal_groups.get(meal_group_name);
                                if (mg === undefined)
                                    mg = meal_groups.set(meal_group_name, []).get(meal_group_name);
                                mg.push(Number(food_or_nutrient_id));
                        
                                console.log(mg);
                                console.log(meal_groups);
                            }
                            
                            // Apply changes and reload the page
                            apply_changes_from_sliders(Number(food_or_nutrient_id), new_amount);
                        }
                    }
                }
                // New nutrient
                else
                {
                    // Change nutrient reference amount
                    if (inp.id === 'addGoodNutrient')
                        nutrient_reference_values.set(Number(food_or_nutrient_id), nutrient_limits.get(Number(food_or_nutrient_id)));
                    else
                        nutrient_reference_values.set(Number(food_or_nutrient_id), -nutrient_limits.get(Number(food_or_nutrient_id)));
                    
                    // Apply changes and reload the page
                    apply_changes_from_sliders();
                    
                    //window.location.href = new_params;
                }
                    
                } // else: not catalogue
                    
                });
            a.appendChild(b);
         // }
        }
        
    }

// Combines the URL for /food_by_rank for "inp" autocomplete field
function combine_food_by_rank_url(inp, suggest_url)
{
    // Don't use suggest_url. Instead of that we combine this url from the most wanted good nutrients at the moment
    //  all bad nurients. The reason is that we want more goods but don't increase bads
    let first = true;
    //let url = `/food_by_rank?limitsort=50&inc=${inp.value}&group=any&nutrients=`;
    let url = `/food_by_rank?sort_by_satiety_per_cal=1&limitsort=50&inc=${inp.value}&nutrients=`;
    let any_good = false;
    for (let [nutrient_id, nutrient_value] of nutrient_current_values)
    {
        let reference_value = nutrient_reference_values.get(nutrient_id);
        if (reference_value < 0)
        {
            if (!first) url += ",";first = false;
            url += `"${nutrient_id}","<","${-reference_value}"`;
        }
        else
        if (nutrient_value < reference_value * 0.1)
        {
            if (!first) url += ",";first = false;
            url += `"${nutrient_id}",">","${reference_value}"`;
            any_good = true;
        }
    }
    
    // If no good were added then they're all more than 10% - so add them all that less than 100%
    if (!any_good)
    {
        for (let [nutrient_id, nutrient_value] of nutrient_current_values)
        {
            let reference_value = nutrient_reference_values.get(nutrient_id);
            if (reference_value > 0 && nutrient_value < reference_value)
            {
                if (!first) url += ",";first = false;
                url += `"${nutrient_id}",">","${reference_value}"`;
            }
        }
    }
    //let url = `${suggest_url}&inc=${inp.value}&group=any`;

    // Add a catalog filter
    if (current_catalogue_item_id !== -1)
        url += `&catalogue_item_id=${current_catalogue_item_id}`;

    // Add the substitution info
    /*if (show_distance_to != -1)
    {
        url += `&show_distance_to=-${show_distance_to}&distance_limit=0.3`;
        /*let fo = food_objects.get(show_distance_to);
         if (fo !== undefined)
         url += `&maxrank=${fo.food_rank}`;
    }*/
    
    // Add the substitution info
    // Note: substitutions work as a food that is close in satiety rather than in a micro nutrient profile
    if (show_distance_to != -1)
    {
        url += `&min_satiety_food=${show_distance_to}&satiety_delta=0.1`;
    }
    
    // Add filters from checkboxes
    url += "&nutrient_limits=";
    let is_first = true;
    if (document.getElementById("filter_high_fat").checked)
    {url += `${(is_first?"":",")}"1004",">","7"`;is_first=false;}
    if (document.getElementById("filter_high_carbs").checked)
    {url += `${(is_first?"":",")}"1005",">","15"`;is_first=false;}
    if (document.getElementById("filter_high_sugar").checked)
    {url += `${(is_first?"":",")}"2000",">","10"`;is_first=false;}
    if (document.getElementById("filter_high_sodium").checked)
    {url += `${(is_first?"":",")}"1093",">","150"`;is_first=false;}

    // Add filters for exact amount of nutrients and calories
    let protein = Number(document.getElementById("filter_protein").value);
    let fat = Number(document.getElementById("filter_fat").value);
    let carbs = Number(document.getElementById("filter_carbs").value);
    let calories = Number(document.getElementById("filter_calories").value);
    let sugar = Number(document.getElementById("filter_sugar").value);
    //let inulin = Number(document.getElementById("filter_inulin").value);
    let variance = Number(document.getElementById("filter_variance").value);
    if (!variance) variance = 1;
    
    if (protein)
    {url += `${(is_first?"":",")}"1003",">","${protein-variance}","1003","<","${protein+variance}"`;is_first=false;}
    if (fat)
    {url += `${(is_first?"":",")}"1004",">","${fat-variance}","1004","<","${fat+variance}"`;is_first=false;}
    if (carbs)
    {url += `${(is_first?"":",")}"1005",">","${carbs-variance}","1005","<","${carbs+variance}"`;is_first=false;}
    if (sugar)
    {url += `${(is_first?"":",")}"2000",">","${sugar-variance}","2000","<","${sugar+variance}"`;is_first=false;}
    //if (inulin)
    //{url += `${(is_first?"":",")}"1403",">","${inulin-variance}","1403","<","${inulin+variance}"`;is_first=false;}
    if (calories)
    {url += `${(is_first?"":",")}"1008",">","${calories-variance}","1008","<","${calories+variance}"`;is_first=false;}

    // Add filters from the prefiltered select box
    let pre_filters = document.getElementById("pre_filters").value;
    if (pre_filters !== "")
    {url += `${(is_first?"":",")+pre_filters}`;is_first=false;}
    if (document.getElementById("filter_high_rank").checked)
    {url += `&minrank=5`;is_first=false;}
    if (document.getElementById("filter_high_protein_or_fiber").checked)
    {url += "&min_pfind=1";is_first=false;}

    // Add the recipe info to the URL
    let all_recipes = JSON.parse(window.localStorage.getItem('BF_recipes'));
    if (all_recipes)
    {
        url += "&recipes=";
        is_first = true;
        for (let recipe of all_recipes)
        {
            url += `${(is_first?"":",")}`;is_first = false;
            url += `"${recipe[0]}","${recipe[1]}","${(recipe.length-2)/2}"`;
            for (let i = 2; i < recipe.length; ++i)
                url += `,"${recipe[i]}"`;
        }
    }
    
    console.log(`combine_food_by_rank_url: url=${url}`);
    
    return url;
}

    function autocomplete(suggest_url, old_food_id, url_params, inp, arr)
    {
        console.log(`autocomplete(1): suggest_url=${suggest_url}`);
        
        if (suggest_url === "nutrients")
        {
          /*execute a function when someone writes in the text field:*/
            inp.addEventListener("input", function(e) {
                let url = `/nutrients`;

                console.log(`autocomplete(4): suggest_url=${suggest_url}, url=${url}`);
                
                if (nutrient_names.length === 0 || nutrient_ids.length === 0)
                {
                    fetch(url)
                       .then(response => response.json())
                       .then(result => processResultNutrients(old_food_id, url_params, inp, result)
                       );
                }
                else
                    processResultNutrients(old_food_id, url_params, inp, "");
            });
        }
        else
            if (suggest_url === "/catalogue")
            {
              /*execute a function when someone writes in the text field:*/
                inp.addEventListener("input", function(e) {
                    let url = `/catalogue`;

                    console.log(`autocomplete(5): suggest_url=${suggest_url}, url=${url}`);
                    
                    if (catalogue_item_names.length === 0 || catalogue_item_ids.length === 0)
                    {
                        fetch(url)
                           .then(response => response.json())
                           .then(result => processResultCatalogue(old_food_id, url_params, inp, result)
                           );
                    }
                    else
                        processResultCatalogue(old_food_id, url_params, inp, "");
                });
            }
            else
        if (suggest_url.startsWith("/food_by_rank"))
        {
            /*execute a function when someone writes in the text field:*/
            inp.addEventListener("input", function(e) {
               
                // Combine the URL for the food_by_rank request
                let url = combine_food_by_rank_url(inp, suggest_url);
                
                console.log(`autocomplete(2): suggest_url=${suggest_url}, url=${url}`);
                fetch(url)
                 .then(response => response.json())
                 .then(result => processResult(old_food_id, url_params, inp, result)
                 );
            });

            inp.addEventListener("click", function(e) {
                
                    if (show_distance_to != -1)
                        inp.placeholder = "Searching for substitutes ...";
                    else
                        inp.placeholder = "Searching for the healthiest match ...";
                
                    // Combine the URL for the food_by_rank request
                    let url = combine_food_by_rank_url(inp, suggest_url)
                    
                    console.log(`autocomplete(3): suggest_url=${suggest_url}, url=${url}`);
                    fetch(url)
                    .then(response => response.json())
                    .then(result => processResult(old_food_id, url_params, inp, result)
                 );
                
            });
        }
        else
        {
            
            /*execute a function when someone writes in the text field:*/
            inp.addEventListener("input", function(e) {
                let url = `/ratio?json=1&inc=${inp.value}&${suggest_url}`;
                console.log(`autocomplete(6): suggest_url=${suggest_url}, url=${url}`);
                //console.log(url);
                fetch(url)
                 .then(response => response.json())
                 .then(result => processResult(old_food_id, url_params, inp, result)
                 );
            });
        }
      
      /*execute a function presses a key on the keyboard:*/
      inp.addEventListener("keydown", function(e) {
          let x = document.getElementById(/*this*/inp.id + "autocomplete-list");
          if (x) x = x.getElementsByTagName("div");
          if (e.keyCode == 40) {
            /*If the arrow DOWN key is pressed,
            increase the currentFocus variable:*/
            currentFocus++;
            /*and and make the current item more visible:*/
            addActive(x);
          } else if (e.keyCode == 38) { //up
            /*If the arrow UP key is pressed,
            decrease the currentFocus variable:*/
            currentFocus--;
            /*and and make the current item more visible:*/
            addActive(x);
          } else if (e.keyCode == 13) {
            /*If the ENTER key is pressed, prevent the form from being submitted,*/
            e.preventDefault();
            if (currentFocus > -1) {
              /*and simulate a click on the "active" item:*/
              if (x) x[currentFocus].click();
            }
          }
      });
      function addActive(x) {
        /*a function to classify an item as "active":*/
        if (!x) return false;
        /*start by removing the "active" class on all items:*/
        removeActive(x);
        if (currentFocus >= x.length) currentFocus = 0;
        if (currentFocus < 0) currentFocus = (x.length - 1);
        /*add class "autocomplete-active":*/
        x[currentFocus].classList.add("autocomplete-active");
      }
      function removeActive(x) {
        /*a function to remove the "active" class from all autocomplete items:*/
        for (let i = 0; i < x.length; i++) {
          x[i].classList.remove("autocomplete-active");
        }
      }

      /*execute a function when someone clicks in the document:*/
      document.addEventListener("click", function (e) {
          closeAllLists(inp, e.target);
      });
      
    } // function autocomplete(inp, arr)


	function save_recipe(name)
	{
    		fetch(window.location.href + "&recipe=" + name);
    		document.getElementById('new_recipe').value = "";
	}
 
    function update_sliders(event)
    {
    //<input type="range" min="1" max="100" value="50" id="myRange" onChange="alert(event.target.value)" onInput="alert(event.target.value)">
    }

function range_x_by_value(v, r, range_max)
{
    /*if (v <= r)
        return Number(range_max * ((v - r) / r + 1) / 2).toFixed();
    else
        return Number(range_max * v / (v + r)).toFixed();*/

    if (v <= r)
        return Number(range_max * ((v - r) / r + 1) / 2).toFixed();
    else
        return Number(range_max * ((v - r) / r + 1) / 2 - 4*(v-r)*range_max/(9*r)).toFixed();
}

function range_value_by_x(x, r, range_max)
{
    if (x <= range_max/2)
        return Number(2 * x * r / range_max).toFixed();
    else
        return Number(-(range_max - x)*r*8/range_max + 10 * x * r / range_max).toFixed();
                      // Number(range_max * r / (2*(range_max - x))).toFixed();
}

                      // Forces autocomplete of a new food to start search as if a user entered some text in the field
                      function force_autocomplete()
                      {
        var event = document.createEvent('Event');
        event.initEvent('input', true, true);
        document.getElementById("myInputNewFood").dispatchEvent(event);
    }
                      
function assemble_get_data_from_server_url(nutrient_id, nutrient_old_value, nutrient_new_value)
    {
        let url = `/balance_simple?foods=`;
        
        // Pass current values of food and nutrients to the server
        let first = 1;
        let first_checked = 1;
        let checked_foods = '';
        for (let [food_id, food_cur] of food_current_values)
        {
            if (!first) url += ","; first = 0;
            url += `"${food_id}","${food_cur}"`;
            
            // Add checked food to stick it from moving when group sliders move
            if (document.getElementById("checked_food_" + food_id).checked)
            {
                if (!first_checked) checked_foods += ","; first_checked = 0;
                checked_foods += `"${food_id}"`;
            }
        }
        url += "&checked_foods=" + checked_foods;
        
        // Note: for nutrients we send REFERENCE VALUES, not current, because
        //  we don't need them on the server side
        url += "&nutrients=";
        first = 1;
        for (let [nutrient_id, nutrient_reference] of nutrient_reference_values)
        {
            // Note: in nutrient_reference_values RECEIVED FROM SERVER the meaning of signs is this:
            //  + means that this is a good nutrient
            //  - means that this is a bad nutrient
            // Note: in "nutrients=" SENT TO SERVER the meaning of signs is this:
            //  +id means that this is a nutrient that's within limits - no matter good or bad
            //  -id means that this is a nutrient that's not within limits - no matter good or bad
            //  +id && +value means that this is a good nutrient within limits
            //  +id && -value means that this is a bad nutrient withint limits
            //  -id && +value means that this is a good nutrient not within limits
            //  -id && -value means that this is a bad nutrient not withint limits
            
            // Good nutrient - check on limits
            if (nutrient_reference > 0)
            {
                if (nutrient_current_values.get(nutrient_id) < nutrient_reference)
                {
                    // Good nutrient is not within limits
                    nutrient_id = -nutrient_id; // -
                    // nutrient_reference = nutrient_reference // +
                }
                // else - Good nutrient is within limits
                // +
                // +
            }
            else
            // Bad nutrient - check on limits
            if (nutrient_reference < 0)
            {
                if (nutrient_current_values.get(nutrient_id) > -nutrient_reference)
                {
                    // Bad nutrient is not within limits
                    nutrient_id = -nutrient_id; // -
                    // nutrient_reference = nutrient_reference; // -
                }
                else
                {
                    // Bad nutrient within limits
                    // nutrient_id // +
                    // nutrient_reference = nutrient_reference; // -
                }
            }
            
            if (!first) url += ","; first = 0;
            url += `"${nutrient_id}","${nutrient_reference}"`;
        }
        
        if (nutrient_id != -1)
            url += `&nutrient_id=${nutrient_id}&nutrient_old_value=${nutrient_old_value}&nutrient_new_value=${nutrient_new_value}`;
        
        // Add meal groups
        url += "&groups=";
        first = 1;
        for (let [meal_group_name, foods] of meal_groups)
        {
            if (!first) url += ","; first = 0;
            url += `"${meal_group_name}"`;
            for (let food_id of foods)
                url += `,"${food_id}"`;
        }
        
        // Add mr data
        url += "&mr=";
        first = 1;
        for (let i = 0; i < 6; ++i)
        {
            if (!first) url += ","; first = 0;
            url += `"${mr_data[i]===undefined?-1:mr_data[i]}"`;
        }
        
        // Add the target goal data
        if (target_weight)
            url += `&target_weight=${target_weight}`;
        if (target_weight_deadline)
            url += `&target_weight_deadline=${target_weight_deadline}`;

        // Add the recipe info to the URL
        let all_recipes = JSON.parse(window.localStorage.getItem('BF_recipes'));
        if (all_recipes)
        {
            url += "&recipes=";
            is_first = true;
            for (let recipe of all_recipes)
            {
                url += `${(is_first?"":",")}`;is_first = false;
                url += `"${recipe[0]}","${recipe[1]}","${(recipe.length-2)/2}"`;
                for (let i = 2; i < recipe.length; ++i)
                    url += `,"${recipe[i]}"`;
            }
        }
        
        console.log(`balance url: ${url}`);
        
        return url;
    } // function assemble_get_data_from_server_url
    
    // Requests data from the server and when it comes updates
    //  sliders, bubbles and local maps
    // Note: if nutrient_id is -1 then the server deducts the nutrient data from the food
    //  data that came from the client (from maps)
    //  if nutrient_id is not -1 then the server rebalances both food and nutrient data
    //  and responses with the total update of the food and nutrient data
    function get_data_from_server(nutrient_id, nutrient_old_value, nutrient_new_value)
    {
        // Enable the Save button
        // This element can be absent if there is no food
        if (document.getElementById("save_current_meal_plan_button"))
            document.getElementById("save_current_meal_plan_button").disabled = false;
        
        let url = assemble_get_data_from_server_url(nutrient_id, nutrient_old_value, nutrient_new_value);
        
        fetch(url)
        .then(response => response.json())
        .then(result => {
            
            //console.log(url);
            //console.log(result.data);
            
            // Adjust upper limit of calories due to the goal
            let calories_limit = nutrient_reference_values.get(1008);
            if (calories_limit && calories_limit < 0)
            {
                console.log(`target_weight=${target_weight}, target_weight_deadline=${target_weight_deadline}`);
                if (target_weight && target_weight_deadline && result.additional_weight_info.calorie_target_intake_1w !== undefined)
                {
                    let new_calorie_target;
                    switch (Number(target_weight_deadline))
                    {
                        case 7: new_calorie_target = result.additional_weight_info.calorie_target_intake_1w; break;
                        case 14: new_calorie_target = result.additional_weight_info.calorie_target_intake_2w; break;
                        case 30: new_calorie_target = result.additional_weight_info.calorie_target_intake_1m; break;
                        case 60: new_calorie_target = result.additional_weight_info.calorie_target_intake_2m; break;
                        case 90: new_calorie_target = result.additional_weight_info.calorie_target_intake_3m; break;
                    }
                    
                    if (new_calorie_target >= 0)
                    {
                        nutrient_reference_values.set(1008, -Math.round(new_calorie_target));
                        document.getElementById('nutrient1008').value = -Math.round(nutrient_reference_values.get(1008));
                    }
                    else
                    {
                        let target_weight_u = (is_imperial?`${Math.round(target_weight/0.454)} lbs`:`${Math.round(target_weight)} kg`);
                        // If the target is negative then this means that user's goal is unrealistic - then don't change
                        //  calories reference and warn the user
                        if (result.additional_weight_info.calorie_target_intake_2w >= 0)
                            alert(`Your goal weight ${target_weight_u} within ${target_weight_deadline} days is unrealistic. But it could be real in 2 weeks`);
                        else
                        if (result.additional_weight_info.calorie_target_intake_1m >= 0)
                            alert(`Your goal weight ${target_weight_u} within ${target_weight_deadline} days is unrealistic. But it could be real in 1 month`);
                        else
                        if (result.additional_weight_info.calorie_target_intake_2m >= 0)
                            alert(`Your goal weight ${target_weight_u} within ${target_weight_deadline} days is unrealistic. But it could be real in 2 months`);
                        else
                        if (result.additional_weight_info.calorie_target_intake_3m >= 0)
                            alert(`Your goal weight ${target_weight_u} within ${target_weight_deadline} days is unrealistic. But it could be real in 3 months`);
                        else
                            alert(`Your goal weight ${target_weight_u} within ${target_weight_deadline} days is unrealistic event within 3 months. Please try to slightly adjust your goal weight`);
                    }
                }
            }
            
            // Move sliders, change data on bubles, change data in current values and
            //  change data in the food/nutrient table
            for (let x of result.data)
            {
                if (x.food_id !== undefined)
                {
                    // Update the current value, the range and the bubble
                    food_current_values.set(Number(x.food_id), x.food_value);
                    let range = document.getElementById("myRange_food_" + x.food_id);
                    let range_x = range_x_by_value(x.food_value,
                                                   food_referenece_values.get(x.food_id), 10000);
                    range.stepUp(range_x - range.value);
                    let bubble = document.getElementById("bubble_food_" + x.food_id);
                    //console.log(x.food_id, food_current_values.get(x.food_id).toFixed());
                    bubble.innerHTML = display_weight_number(food_current_values.get(x.food_id), false, false);

                    let food_weight_input = document.getElementById("food" + x.food_id);
                    //console.log(x.food_id, food_current_values.get(x.food_id).toFixed());
                    food_weight_input.value = display_weight_number(food_current_values.get(x.food_id), true, false);
                    
                    // Update "td" with kg in 3 months
                    let td = document.getElementById("kg_3_month_" + x.food_id);
                    let food_resp_3_month = result.weight_data[31].total_fat_gain * x.food_cal_exc_protein / result.additional_weight_info.calorie_intake;
                    
                    //console.log(`KG in 3 MONTHS: ${x.food_id}, ${food_resp_3_month}`);
                    
                    /* - result.additional_weight_info.calorie_need*/
                    
                    if (food_resp_3_month >= 1000)
                        td.innerHTML = `${display_weight_number(food_resp_3_month, false, true)} of fat`;
                    else
                        td.innerHTML = "";
                    
                    // Update the best nutrient percent
                    //let best_n_percent = document.getElementById(`best_${x.food_id}`).innerHTML;
                    document.getElementById(`best_${x.food_id}`).innerHTML = x.food_best_nurient; //Math.round(x.food_value * best_n_percent / 100.0);
                                                                                        
                    // Update sCal
                    document.getElementById(`scal_${x.food_id}`).innerHTML = x.food_scal;
                }
                if (x.nutrient_id !== undefined)
                {
                    // Update the current value, the range and the bubble
                    nutrient_current_values.set(Number(x.nutrient_id), x.nutrient_value);
                    let range = document.getElementById("myRange_nutrient_" + x.nutrient_id);
                    let range_x = range_x_by_value(x.nutrient_value,
                                                   Math.abs(nutrient_reference_values.get(x.nutrient_id)), 10000);
                    range.stepUp(range_x - range.value);
                    let bubble = document.getElementById("bubble_nutrient_" + x.nutrient_id);
                    bubble.innerHTML =
                        (nutrient_current_values.get(x.nutrient_id) < 1.0 ?
                         nutrient_current_values.get(x.nutrient_id).toFixed(1) :
                         nutrient_current_values.get(x.nutrient_id).toFixed());
                    //console.log(`set data: x.nutrient_id=${x.nutrient_id}, x.nutrient_value=${x.nutrient_value}, range.value=${range.value}, range_x=${range_x}`);

                    // Change good/bad signs
                    let nutrient_details = document.getElementById("nutrient_details_" + x.nutrient_id);
                    if (nutrient_reference_values.get(x.nutrient_id) > 0)
                    {
                        let ratio = Math.round(100.0*(nutrient_reference_values.get(x.nutrient_id)-x.nutrient_value)/
                                     nutrient_reference_values.get(x.nutrient_id));
                        
                        if (ratio <= 0)
                        {
                            // Add special warning for protein
                            if (x.nutrient_id == 1003 && nutrient_reference_values.get(x.nutrient_id) > 70)
                                nutrient_details.innerHTML = "<span style=\"color: green;\"><b>GOOD</b></span>, make sure having enough physical activity for this high level of protein otherwise it goes to belly fat";
                            else
                                nutrient_details.innerHTML = "<span style=\"color: green;\"><b>GOOD</b></span>";
                        }
                        else
                        {
                            // Add special warning for protein and fiber
                            if (x.nutrient_id == 1003)
                                nutrient_details.innerHTML = `<span style="color: red;"><b>(${ratio} % less)</b></span> increase protein - increase satiety, increase metabolism, lose weight, look at Pro column above`;
                            else
                            if (x.nutrient_id == 1079)
                                nutrient_details.innerHTML = `<span style="color: red;"><b>(${ratio} % less)</b></span> increase fiber - increase satiety, reduce hunger, look at Fib column above`;
                            else
                            if (x.nutrient_id == 1162)
                                nutrient_details.innerHTML = `<span style="color: red;"><b>(${ratio} % less)</b></span> increase vitamin C - boost immunity, look at Vit column above`;
                            else
                            {
                                nutrient_details.innerHTML = `<span style="color: red;"><b>(${ratio} % less)</b><!--click <a href="JavaScript:force_autocomplete();">here</a> to find food rich in this nutrient--></span>`;
                            }
                        }
                    }
                    else
                    {
                        let ratio = (100.0*(nutrient_reference_values.get(x.nutrient_id)+x.nutrient_value)/
                                     -nutrient_reference_values.get(x.nutrient_id)).toFixed();

                        if (ratio <= 0)
                            nutrient_details.innerHTML = "<span style=\"color: green;\"><b>GOOD</b></span>";
                        else
                        {

                            
                            // Add special warning for sugars and carbs
                            if (x.nutrient_id == 2000)
                                nutrient_details.innerHTML = `<span style="color: red;"><b>(${ratio} % more)</b></span> cut sugars - reduce hunger (not just white powder, bakery, bread etc - look at Sug col above`;
                            else
                            if (x.nutrient_id == 1005)
                                nutrient_details.innerHTML = `<span style="color: red;"><b>(${ratio} % more)</b></span> cut carbs to reduce hunger - look at Car column above`;
                            else
                            if (x.nutrient_id == 1008)
                            {
                                let calorie_need2 = result.additional_weight_info.calorie_need2;
                                console.log(calorie_need2);
                                nutrient_details.innerHTML = `<span style="color: red;"><b>(${ratio} % more)</b></span>, click <a href="javascript:void(0)" onclick="get_data_from_server(1008, nutrient_current_values.get(1008), -nutrient_reference_values.get(1008));">here</a> to rebalance. Also for equilibrium your need is <a href="javascript:void(0)" onclick="get_data_from_server(1008, nutrient_current_values.get(1008), ${calorie_need2});">${Number(calorie_need2).toFixed()}</a> cal, see details below`;
                            }
                            else
                            {
                                nutrient_details.innerHTML = `<span style="color: red;"><b>(${ratio} % more)</b></span>`;
                            }
                        }
                    }
                }
                if (x.food_nutrient_id !== undefined)
                {
                    let td = document.getElementById("food_nutrient_" + x.food_nutrient_id).innerHTML =
                        x.value;
                }
                if (x.meal_group_name !== undefined)
                {
                    // Determine the reference value for this meal group
                    let ref_value = 0;
                    for (let food_id of meal_groups.get(x.meal_group_name))
                        ref_value += food_referenece_values.get(food_id);
                    // Update the range and the bubble for the meal group
                    let range = document.getElementById("myRange_meal_group_" + x.meal_group_name);
                    
                    // A meal group can be empty and then there is no range and buble elements
                    if (range !== undefined && range !== null)
                    {
                        let range_x = range_x_by_value(Number(x.meal_group_value),
                                                   ref_value, 10000);
                        //console.log(range_x, range.value, ref_value);
                    
                        range.stepUp(range_x - range.value);
                    
                        let bubble = document.getElementById("bubble_meal_group_name_" + x.meal_group_name);
                        //console.log(x.food_id, food_current_values.get(x.food_id).toFixed());
                        bubble.innerHTML = display_weight_number(x.meal_group_value, false, false);
                    }
                }
                if (x.meal_group_nutrient_id !== undefined)
                {
                    // Update an item in the meal group nutrient list
                    let td = document.getElementById("meal_group_nutrient_" + x.meal_group_nutrient_id);
                    
                    // A meal group can be empty and then there is no "td"s with nutrient values
                    if (td)
                        td.innerHTML = x.value;
                }
                if (x.meal_group_scal !== undefined)
                {
                    // Update an item in the meal group nutrient list
                    let td = document.getElementById("meal_group_scal_" + x.meal_group_scal);
                    
                    // A meal group can be empty and then there is no "td"s with nutrient values
                    if (td)
                        td.innerHTML = x.value;
                }
            } //for (let x of result.data)
            
            // Fill mr data
            if (result.metabolic_info !== undefined)
            {
                document.getElementById("in_mr_data_gender").value = mr_data[0] = result.metabolic_info.in_mr_data_gender;
                mr_data[1] = result.metabolic_info.in_mr_data_weight;
                
                document.getElementById("in_mr_data_weight").value = (is_imperial?(mr_data[1]/0.454).toFixed():mr_data[1]);
                
                mr_data[2] = result.metabolic_info.in_mr_data_height;
                if (is_imperial)
                {
                    document.getElementById("in_mr_data_height_feet").value = Math.trunc(mr_data[2] / 30.48);
                    document.getElementById("in_mr_data_height_inches").value = Math.round((mr_data[2] - Math.trunc(mr_data[2] / 30.48) * 30.48) / 2.54);
                }
                else
                    document.getElementById("in_mr_data_height").value = mr_data[2];
                document.getElementById("in_mr_data_age").value =
                    mr_data[3] = result.metabolic_info.in_mr_data_age;
                document.getElementById("in_mr_data_exercise").value = mr_data[4] = result.metabolic_info.in_mr_data_exercise;
                document.getElementById("in_mr_data_body_type").value = mr_data[5] = result.metabolic_info.in_mr_data_body_type;
                
                let food_thermic_effect = result.additional_weight_info.carbs_thermic_effect +
                    result.additional_weight_info.protein_thermic_effect +
                    result.additional_weight_info.alcohol_thermic_effect;
                
                document.getElementById("metabolic_rate").innerHTML =
                    `Basal metabolic rate ${result.metabolic_info.out_basal_metabolic_rate.toFixed()} cal<br>Metabolic rate ${result.metabolic_info.out_metabolic_rate.toFixed()} cal<br>Recommended protein intake ${result.metabolic_info.out_protein_max_intake.toFixed()} grams<br>Food thermic effect ${food_thermic_effect.toFixed()}<br>` +
                `Protein need above basal metabolism ${result.additional_weight_info.protein_need_above_bmr_cal.toFixed()} cal` +
                (result.additional_weight_info.protein_need_above_bmr_cal < 0 ? ", it's negative because you intake  less amount of protein than your body needs" :"") +
                `<br>Calorie need a day to reach equilibrium in weight ${result.additional_weight_info.calorie_need2.toFixed()} <span style="color: green">&lt;-- eat this much :-) </span><br><a target="_blank" href='${assemble_get_data_from_server_url(-1)}'>raw data</a>`;
            }
            
            // Fill weight data info
            // If the chart is collapsed then just ignore this data
            if (document.getElementById("chart_period"))
            if (result.weight_data !== undefined)
            {
                let week_result;
                let month_result;
                let month3_result;

                let chart_data1 = [['Day', 'Metabolic rate', 'Calorie surplus']];
                let chart_data2 = [['Day', (is_imperial?'Fat gain, oz':'Fat gain, g'), (is_imperial?'Muscle gain, oz':'Muscle gain, g'), (is_imperial?'Muscle loss due deficit, oz':'Muscle loss due deficit, g')]];
                let chart_data3 = [['Day', (is_imperial?'Total fat gain, lb':'Total fat gain, kg'), (is_imperial?'Total muscle gain, lb':'Total muscle gain, kg'), (is_imperial?'Total muscle loss due deficit, lb':'Total muscle loss due deficit, kg')]];
                let chart_data4 = [['Day', (is_imperial?'Weight, lb':'Weight, kg')]];

                for (let x of result.weight_data)
                {
                    if (x.day == 7)
                        week_result = x;
                    if (x.day == 30)
                        month_result = x;
                    if (x.day == 90)
                        month3_result = x;

                    // Fill chart data
                    if (x.day <= document.getElementById("chart_period").value)
                    {
                        chart_data1.push([x.day, x.metabolism, x.calorie_surplus]);
                        chart_data2.push([x.day,
                                          Math.round(is_imperial?x.fat_gain/28.3495:x.fat_gain),
                                        Math.round(is_imperial?x.muscle_gain/28.3495:x.muscle_gain),
                                        Math.round(is_imperial?x.muscle_loss_component_due_to_deficit/28.3495:x.muscle_loss_component_due_to_deficit)]);
                        let total_fat_gain = (is_imperial?x.total_fat_gain/454:x.total_fat_gain/1000);
                        let total_muscle_gain = (is_imperial?x.total_muscle_gain/454:x.total_muscle_gain/1000);
                        let total_muscle_loss_due_deficit = (is_imperial?x.total_muscle_loss_due_deficit/454:x.total_muscle_loss_due_deficit/1000);
                        chart_data3.push([x.day, Math.round(total_fat_gain), Math.round(total_muscle_gain), Math.round(total_muscle_loss_due_deficit)]);
                        chart_data4.push([x.day, Math.round((is_imperial?mr_data[1]/0.454:mr_data[1]) + total_fat_gain + total_muscle_gain)]);
                    }
                }

                                                   

                                                           new google.visualization.LineChart(document.getElementById('curve_chart1')).draw(google.visualization.arrayToDataTable(chart_data1), {
                        title: 'Metabolism',
                        curveType: 'function',
                        legend: { position: 'bottom' }
                      });
                                          
                                          new google.visualization.LineChart(document.getElementById('curve_chart2')).draw(google.visualization.arrayToDataTable(chart_data2), {
                        title: (is_imperial?'Fat/Muscle gain per day, oz':'Fat/Muscle gain per day, grams'),
                        curveType: 'function',
                        legend: { position: 'bottom' }
                      });
                                          
                                          new google.visualization.LineChart(document.getElementById('curve_chart3')).draw(google.visualization.arrayToDataTable(chart_data3), {
                        title: (is_imperial?'Fat/Muscle gain total, lb':'Fat/Muscle gain total, kg'),
                        curveType: 'function',
                        legend: { position: 'bottom' }
                      });
                                          
                                          new google.visualization.LineChart(document.getElementById('curve_chart4')).draw(google.visualization.arrayToDataTable(chart_data4), {
                        title: (is_imperial?'Weight, lb':'Weight, kg'),
                        curveType: 'function',
                        legend: { position: 'bottom' }
                      });
                            
                        
                                                             
                function fat_to_text(gain)
                {
                    if (gain/1000 < 0.1 && gain/1000 > -0.1)
                        gain = 0;
                    
                    if (gain > 0)
                        return `<span style="color: red;">gain ${display_weight_number(gain, false, true)}</span>`;
                    else
                        return `<span style="color: green;">burn ${display_weight_number(-gain, false, true)}</span>`;
                }

                function muscle_to_text(gain)
                {
                    if (gain/1000 < 0.1 && gain/1000 > -0.1)
                        gain = 0;
                    
                    if (gain >= 0)
                        return `<span style="color: green;">build ${display_weight_number(gain, false, true)}</span>`;
                    else
                        return `<span style="color: red;">lose ${display_weight_number(-gain, false, true)}</span>`;
                }
                
                document.getElementById("week_gain").innerHTML =
                    `In a week you'll ${fat_to_text(week_result.total_fat_gain)} of fat and ${muscle_to_text(week_result.total_muscle_gain)} of muscle`;
                                         
                // If we still burn muscle but the calorie intaken as needed - then recommend
                // increase exercise
                                          /*console.log(week_result.total_muscle_gain, result.additional_weight_info.calorie_intake, result.additional_weight_info.calorie_need2);*/
                if (week_result.total_muscle_gain <= -100 &&
                        Math.abs(result.additional_weight_info.calorie_intake - result.additional_weight_info.calorie_need2) < 5)
                        {
                            let txt;
                            if (mr_data[4] == 0)
                                txt = 'to 1-3 times/week';
                            else
                            if (mr_data[4] == 1)
                                txt = 'to 4-5 times/week';
                            else
                            if (mr_data[4] == 2)
                                txt = 'to daily or make it intense 3-4 times/week';
                            else
                            if (mr_data[4] == 3)
                                txt = 'to intense exercise 6-7 times/week';
                            else
                            if (mr_data[4] == 4)
                                txt = 'to very intense exercise daily';
                            else
                                txt = 'as much you can';
                            document.getElementById("week_gain").innerHTML +=
                            `, to stop burn muscle increase your exercise ${txt}`;
                        }
                                          
                document.getElementById("month_gain").innerHTML =
                    `In a month you'll ${fat_to_text(month_result.total_fat_gain)} of fat and ${muscle_to_text(month_result.total_muscle_gain)} of muscle`;
                document.getElementById("month3_gain").innerHTML =
                    `In 3 months you'll ${fat_to_text(month3_result.total_fat_gain)} of fat and ${muscle_to_text(month3_result.total_muscle_gain)} of muscle`;
                
                let sodium_g = result.additional_weight_info.water_retention_sodium;
                let txt = '';
                if (sodium_g > 100)
                    txt += `Cut sodium - lose <span style="color: red;">${display_weight_number(sodium_g, false, true)}</span> of excess water bound by sodium, look at Sod column above<br>`;
                let carbs_g = result.additional_weight_info.water_retention_carbs;
                if (carbs_g > 100)
                    txt += `Cut carbs - lose <span style="color: red;">${display_weight_number(carbs_g, false, true)}</span> of excess water bound by carbs, look at Car column above`;
                document.getElementById("carbs_n_sodium").innerHTML = txt;

                // Fill diet type info
                txt = `${(100*result.additional_weight_info.calorie_intake_protein_percent).toFixed()}% of calories from protein<br>`;
                                          txt += `${(100*result.additional_weight_info.calorie_intake_fat_percent).toFixed()}% of calories from fat<br>`;
                                          txt += `${(100*result.additional_weight_info.calorie_intake_carbs_percent).toFixed()}% of calories from carbs (out of which `;
                                          txt += `${(100*result.additional_weight_info.calorie_intake_sugar_percent).toFixed()}% of calories from sugar)<br>`;
                                          txt += `${(100*result.additional_weight_info.calorie_intake_alcohol_percent).toFixed()}% of calories from alcohol<br>`;
                                          
                if (result.additional_weight_info.calorie_intake_sugar_percent > 0.2)
                    txt += '<span style="color: red;">You consume too much sugar which makes you hungry and makes you eat more</span><br>';
                if (result.additional_weight_info.calorie_intake_protein_percent < 0.1)
                    txt += '<span style="color: red;">You consume too little protein which is bad for your hair, nails, muscles</span><br>';
                if (result.additional_weight_info.calorie_intake_fat_percent > 0.65 &&
                    result.additional_weight_info.calorie_intake_carbs_percent < 0.2)
                    txt += '<span style="color: green;">You are on a keto diet which is good! It reduces hunger</span><br>';
                if (result.additional_weight_info.calorie_intake_protein_percent > 0.35)
                    txt += '<span style="color: green;">You are on a very high protein diet! Which is good but beware you have enough exercise</span><br>';
                                          
                document.getElementById("type_of_diet").innerHTML = txt;
            } // if (result.weight_data !== undefined)
            
        });
    } // function get_data_from_server(nutrient_id)
                      
function mr_data_change(id)
{
    let data = Number(document.getElementById(id).value);
    console.log(data, id);
    if (id === 'in_mr_data_gender') {mr_data[0] = data; apply_changes_from_sliders();}
    if (id === 'in_mr_data_exercise') {mr_data[4] = data; apply_changes_from_sliders();}
    if (id === 'in_mr_data_body_type') {mr_data[5] = data; apply_changes_from_sliders();}
}
                                
// Change the current meal plan to the one selected from the meal plan select box
function select_meal_plan()
{
    console.log('select_meal_plan');
    
    let v = document.getElementById("meal_plans_input").value;
    
    // Create a new meal plan without foods and reload the page
    if (v === 'Create new meal plan')
    {
        food_current_values.clear();
        food_referenece_values.clear();
        meal_groups.clear();
        current_meal_plan_name = '';
        apply_changes_from_sliders();
    }
    
    let meal_plans = JSON.parse(window.localStorage.getItem('BF_mean_plans'));
    for (let x of meal_plans)
    {
        if (x[0] === v)
        {
            console.log(`plan selected: ${x[1]}`);
            
            // Add the meal plan name to the URL if it's not there
            if (x[1].indexOf('&meal_plan=') == -1)
                x[1] += `&meal_plan=${v}`;
            
            window.location.href = x[1];
        }
    }
}

// Saves all local changes on the page to the current meal plan
function save_current_meal_plan()
{
    //window.localStorage.removeItem("BF_mean_plans");
            
    // Get the meal plan name from the input
    let value = document.getElementById("meal_plans_input").value;
    document.getElementById("save_current_meal_plan_button").disabled = true;
    
    // Change the current meal plan name to tha one from the input
    current_meal_plan_name = value;
    
    // Assemble a new URL with that meal plan name
    let url = assemble_meal_plan_url();
    
    console.log('save_current_meal_plan: ', value, current_meal_plan_name);
            
    let meal_plans = window.localStorage.getItem("BF_mean_plans");
    if (meal_plans === undefined || meal_plans === null)
    {
        meal_plans = new Array;
        meal_plans.push([value, url]);
        window.localStorage.setItem("BF_mean_plans", JSON.stringify(meal_plans));
    }
    else
    {
        current_meal_plan_name = value;
                
        meal_plans = JSON.parse(meal_plans);
        // If there is a plan with the same name then just replace it
        let renamed = false;
        for (let x of meal_plans)
        if (x[0] == current_meal_plan_name)
        {
            x[1] = url;
            renamed = true;
        }
        if (!renamed) meal_plans.push([value, url]);
        window.localStorage.setItem("BF_mean_plans", JSON.stringify(meal_plans));
    }
            
    console.log('meal plan saved: ', meal_plans);

    show_meal_plan_select_box();
}
                                          
function remove_current_meal_plan()
{
    // Take meal plans from local storage, remove current one and put plans back
    let meal_plans = window.localStorage.getItem("BF_mean_plans");
    if (meal_plans === undefined || meal_plans === null)
        return;
    meal_plans = JSON.parse(meal_plans);
    if (!meal_plans.length)
        return;
    let new_meal_plans = [];
    for (let x of meal_plans)
        if (x[0] !== current_meal_plan_name)
            new_meal_plans.push(x);
    window.localStorage.setItem("BF_mean_plans", JSON.stringify(new_meal_plans));
    
    // Reset current meal plan name to prevent it from showing up again
    current_meal_plan_name = '';
    
    console.log('remove_current_meal_plan', new_meal_plans, current_meal_plan_name);
    
    // Reload the page without the meal plan in the URL
    apply_changes_from_sliders();
}
                                          
function show_meal_plan_select_box()
{
    // Take meal plans from local storage
    let meal_plans = window.localStorage.getItem("BF_mean_plans");
    if (meal_plans === undefined || meal_plans === null)
        meal_plans = new Array;
    else
        meal_plans = JSON.parse(meal_plans);
    
    console.log('show_meal_plan_select_box: ', meal_plans);
    
    // Find current meal plan in local storage
    let is_current_meal_plan_found = false;
    for (let x of meal_plans)
        if (x[0] == current_meal_plan_name)
        {
            is_current_meal_plan_found = true;
            break;
        }
    // Not found - add it with the current url page
    if (current_meal_plan_name !== '' && !is_current_meal_plan_found)
    {
        is_current_meal_plan_found = true;
        meal_plans.push([current_meal_plan_name, assemble_meal_plan_url()]);
        window.localStorage.setItem("BF_mean_plans", JSON.stringify(meal_plans));
    }
    
    let txt = `<input onchange="select_meal_plan()" list="meal_plans_datalist" id="meal_plans_input" value="${current_meal_plan_name}" placeholder="Enter meal plan name" style="width: 350px"><datalist id="meal_plans_datalist" style="background-color: #f1f1f1; font-size: 16px; height: 35px; width: 100%; border: 1px solid #d4d4d4; border-bottom: none; border-top: none; z-index: 99;">`;
    
    for (let x of meal_plans)
    {
        txt += `<option value="${x[0]}"`;
        // Mark the current plan selected;
        if (x[0] === current_meal_plan_name)
            txt +=` selected`;
        //console.log(x[1]);
        //console.log(url);
        txt += `>`;//${x[0]}</option>`;
    }
    if (!is_current_meal_plan_found)
    {
        // If the meal
        current_meal_plan_name = '';
        //txt += `<option value="" disabled selected hidden>Select meal plan>`;
    }
    txt += `<option value="Create new meal plan"></datalist>`;
    
    //console.log(`show_meal_plan_select_box: txt=${txt}`);
    
    // This element can be absent if there is no food
    if (document.getElementById("locally_saved_meal_plans"))
        document.getElementById("locally_saved_meal_plans").innerHTML = txt;
}

function mr_data_keyup(event)
{
                                              let data = Number(this.value);
                                              
                                              console.log(data, this.id);
                                              
                                            if (event.keyCode === 13)
                                            {
                                                mr_data[1] = document.getElementById("in_mr_data_weight").value;
                                                
                                                // Pounds to kilograms
                                                if (is_imperial)
                                                    mr_data[1] = Math.round(mr_data[1] * 0.454);
                                                
                                                // Feet and inches to cm
                                                if (is_imperial)
                                                {
                                                    let feet = document.getElementById("in_mr_data_height_feet").value;
                                                    let inches = document.getElementById("in_mr_data_height_inches").value;
                                                    mr_data[2] = Math.round(feet * 30.48 + inches * 2.54);
                                                }
                                                else
                                                    mr_data[2] = document.getElementById("in_mr_data_height").value;
                                                
                                                mr_data[3] = document.getElementById("in_mr_data_age").value;
                                                apply_changes_from_sliders();
                                            }

}
                                          
var is_imperial = window.location.href.indexOf("imperial=1") != -1;
                                          
function metric_imperial_change()
{
    is_imperial = document.getElementById("oz_grams").value == "oz";
    
    // For imperial we need feet and inches for the height
    if (is_imperial)
    {
        document.getElementById("in_mr_data_weight").placeholder = "Weight, lb";
        document.getElementById("td_in_mr_data_height").innerHTML=`<input style="width: 50px;" id="in_mr_data_height_feet" type="text" placeholder="Height, feet">&nbsp;<input style="width: 50px;" id="in_mr_data_height_inches" type="text" placeholder="Height, inches">`;
        document.getElementById("td_in_mr_data_weight_header").innerHTML='Weight, lb';
        document.getElementById("td_in_mr_data_height_header").innerHTML="Height, ft, &Prime;";

        document.getElementById("in_mr_data_height_feet").addEventListener("keyup", mr_data_keyup);
        document.getElementById("in_mr_data_height_inches").addEventListener("keyup", mr_data_keyup);
    }
    else
    {
        document.getElementById("in_mr_data_weight").placeholder = "Weight, kg";
        document.getElementById("td_in_mr_data_height").innerHTML=`<input id="in_mr_data_height" type="text" placeholder="Height">`;
        document.getElementById("td_in_mr_data_weight_header").innerHTML='Weight, kg';
        document.getElementById("td_in_mr_data_height_header").innerHTML="Height, cm";
                            
        document.getElementById("in_mr_data_height").addEventListener("keyup", mr_data_keyup);
    }
    
    // Update the recipes visual
    show_recipes();

    // Update everything from the server
    get_data_from_server(-1);
}
                                          
// Displays weight number in grams or oz - depending on settings
function display_weight_number(number, is_for_edit, is_big_number)
{
    // Grams
    if (!is_imperial)
    {
        if (is_big_number && !is_for_edit)
            return (number / 1000).toFixed(1) + " kg";
        else
            return number.toFixed();
    }
    // oz
    else
    {
        if (is_big_number && !is_for_edit)
            return (number / 453.592).toFixed(1) + " lbs";
                                      
        let oz = number / 28.3495;
        if (is_for_edit)
        {
            if (oz < 1.0)
                return oz;
            else
                return Math.round(oz);
        }
        let r = Math.abs(oz - Math.trunc(oz));
        if (r > 0.875)
            return Math.ceil(oz);
        else
        {
            let txt = (Math.trunc(oz) ? Math.trunc(oz): "");
            if (r < 0.375)
                txt += '&frac14;';
            else
            if (r < 0.625)
                txt += '&frac12;';
            else
            if (r < 0.875)
                txt += '&frac34;';
            return txt;
        }
    }
}
 
let current_recipe_id = -1;

// Adds the food to the recipe
function add_food_to_recipe(food_id, amount)
{
    if (current_recipe_id === -1)
        return;
    
    let all_recipes = JSON.parse(window.localStorage.getItem('BF_recipes'));
    // Search for the current recipe
    if (all_recipes)
        for (let recipe of all_recipes)
            if (recipe[0] === current_recipe_id)
            {
                // Search for the food in the recipe
                let food_found = false;
                for (let i = 2; i < recipe.length; i += 2)
                {
                    if (recipe[i] == food_id)
                    {
                        recipe[i+1] += amount;
                        food_found = true;
                        break;
                    }
                }
                // If food was not found then push it
                if (!food_found)
                {
                    recipe.push(food_id);
                    recipe.push(amount);
                }
            }
    console.log(`add_food_to_recipe after add: new_recipes=${JSON.stringify(all_recipes)}`);
    window.localStorage.setItem('BF_recipes', JSON.stringify(all_recipes));
    show_recipes();
}
                    
// Changes current recipe
function change_current_recipe()
{
    current_recipe_id = Number(document.getElementById("recipes_select_box").value);
    console.log(`change_current_recipe: current_recipe_id=${current_recipe_id}`);
    show_recipes();
}
                    
// Removes current recipe
function remove_recipe()
{
    console.log(`remove_recipe: current_recipe_id=${current_recipe_id}`);
    if (current_recipe_id !== -1)
    {
        let all_recipes = JSON.parse(window.localStorage.getItem('BF_recipes'));
        let new_recipes = [];
        if (all_recipes)
            for (let x of all_recipes)
                if (x[0] !== current_recipe_id)
                    new_recipes.push(x);
        console.log(`remove_recipe after remove: new_recipes=${JSON.stringify(new_recipes)}`);
        window.localStorage.setItem('BF_recipes', JSON.stringify(new_recipes));
        current_recipe_id = -1;
        show_recipes();
    }
}
                    
// Creates a new empty recipe
function create_new_recipe()
{
    let all_recipes = JSON.parse(window.localStorage.getItem('BF_recipes'));
    if (all_recipes === null || all_recipes === undefined)
        all_recipes = new Array();
    current_recipe_id = 5000000 + Math.trunc(Math.random() * 1000000);
    all_recipes.push([current_recipe_id, document.getElementById("new_recipe_name").value]);
    window.localStorage.setItem('BF_recipes', JSON.stringify(all_recipes));
    
    console.log(`create_new_recipe: current_recipe_id=${current_recipe_id}, all_recipes=${JSON.stringify(all_recipes)}`);
    
    show_recipes();
}
                    
// Changes amount of food per recipe
function change_recipe_food_amount(food_id)
{
    let amount = Number(document.getElementById(`cur_recipe_food_${food_id}`).value);
    if (is_imperial) amount *= 28.3495;
    
    //console.log(amount);

    let all_recipes = JSON.parse(window.localStorage.getItem('BF_recipes'));
    if (all_recipes)
    {
        // Search for a food in the current recipe and update it
        for (let recipe of all_recipes)
            if (recipe[0] === current_recipe_id)
                for (let i = 2; i < recipe.length; i += 2)
                    if (recipe[i] == food_id)
                    {
                        // If the amount is zero then remove the food from the recipe
                        if (amount == 0)
                            recipe.splice(i,2);
                        else
                            recipe[i+1] = amount;
                        break;
                    }
        window.localStorage.setItem('BF_recipes', JSON.stringify(all_recipes));
    }
    
    show_recipes();
    
    // Get data from the server to update current meal if it leverages the changed recipe
    get_data_from_server(-1);
}
                    
// Recipes are stored in local storage
function show_recipes()
{
    let all_recipes = JSON.parse(window.localStorage.getItem('BF_recipes'));
    
    console.log(`show_recipes: all_recipes=${JSON.stringify(all_recipes)}`);
    
    let txt = `<tr><td colspan="20" style="border-bottom: 1px solid #ddd; height: 15px;"></td></tr>\
    <tr><td><b>Your recipes</b></td>\
        <td colspan="2"><select onchange="change_current_recipe()" style="background-color: #f1f1f1; font-size: 16px; height: 40px; border: 1px solid #d4d4d4; border-bottom: none; border-top: none; z-index: 99;" id="recipes_select_box" autocomplete="off">`;
    
    // Recipes are stored as an array of arrays:
    //  [[food_id_for_recipe, recipe_name, food1_id, food1_amount, ... , foodN_id, foodN_amount]]
    let current_recipe;
    if (all_recipes)
    {
        for (let x of all_recipes)
        {
            txt += `<option value="${x[0]}" ${x[0] === current_recipe_id ? (current_recipe = x,"selected") : ""}>${x[1]}</option>`;
        }
    }

    txt += `\
    <option disabled ${current_recipe !== undefined?"":"selected"} hidden value="-1">Choose a recipe</option></select>\
    <input onclick="remove_recipe()" type="submit" value="Remove recipe"></td>\
    <td><input id="new_recipe_name" value="" placeholder="Enter new recipe name" style="width: 350px"><datalist id="meal_plans_datalist" style="background-color: #f1f1f1; font-size: 16px; height: 35px; width: 100%; border: 1px solid #d4d4d4; border-bottom: none; border-top: none; z-index: 99;">\
    </td><td colspan="3"><input onclick="create_new_recipe()" type="submit" value="Create new recipe">\
    </td></tr>`;
    
    txt += `<tr><td colspan="15" style="height: 15px;"></td></tr><tr>`;
    
    // If there is current recipe then output all its foods
    if (current_recipe !== undefined)
    {
        txt += `<td>${is_imperial?"oz":"grams"}</td><td colspan="3"></td>`;
        // Output headers for nutrients
        for (let nutrient_header_name of nutrient_header_names)
        {
            txt += `<td>${nutrient_header_name}</td>`;
        }
        txt += `<td></td><td>USDA info</td></tr>`;
        
        // Get per 100g for nutritions of all food that are included in the recipe
        let url = '/food_details?foods=';
        let is_first = true;
        for (let i = 2; i < current_recipe.length; i += 2)
        {
            if (!is_first)url+=",";is_first=false;
            url+=`"${current_recipe[i]}"`;
        }
        url+='&nutrients=';
        is_first = true;
        for (let [nutrient_id] of nutrient_reference_values)
        {
            if (!is_first)url+=",";is_first=false;
            url+=`"${nutrient_id}"`;
        }
        
        console.log(`show_recipes: url=${url}`);
        fetch(url)
       .then(response => response.json())
       .then(result => {
           let i = 3;
           let total_nutrient_amounts = [];
           let total_recipe_weight = 0;
           let j = 0;
           for (let x of result.data)
           {
               let food_id = x.food_id;
               let food_name = x.food_name;
               let food_amount = current_recipe[i];
               total_recipe_weight += food_amount;
               
               txt += `<tr>\
            <td><input onkeyup="if (event.keyCode === 13) change_recipe_food_amount(${food_id})" id="cur_recipe_food_${food_id}" style="width: 75px; height: 20px;" type="text" value="${display_weight_number(food_amount, true, false)}"></td>\
            <td><img src="/food_pics?file=${food_id}.jpeg" width="24" height="24" onmouseover="bigImg(this)" onmouseout="normalImg(this)"\
            onerror="this.style.display='none'"/></td>\
            <td><a href="https://www.google.com/search?q=${food_name}" target="blank">${food_name.substring(0,45)}</a></td><td></td>`;
               j = 0;
               for (let y of x.nutrients)
               {
                   let nutrient_id = y.nutrient_id;
                   let nutrient_amount_per_100g = y.amount;
                   let this_nutrient_amount = nutrient_amount_per_100g*food_amount/100;
                   //console.log(nutrient_amount_per_100g, food_amount, nutrient_amount_per_100g*food_amount/100);
                   if (total_nutrient_amounts[j] === undefined)
                       total_nutrient_amounts[j] = 0;
                   total_nutrient_amounts[j++] += this_nutrient_amount;
                   txt += `<td>${Math.round(nutrient_amount_per_100g)}/${Math.round(this_nutrient_amount)}</td>`;
               }
               i += 2;
               txt += `<td></td><td><a href="/nutrition?food=${food_id}" target="blank">${food_id}</a></td></tr>`;
           } // for (let x of result.data)
           
           // Total nutrients per recipe
           txt += `<tr><td colspan="10" style="height: 15px;"></td></tr><tr><td colspan="4">Total per recipe</td>`;
           for (let k = 0; k < j; ++k)
               txt += `<td>${Math.round(total_nutrient_amounts[k]*100/total_recipe_weight)}/${Math.round(total_nutrient_amounts[k])}</td>`;
           txt += `</tr><tr><td colspan="15" style="height: 15px;"></td></tr>`;
    
        document.getElementById("recipe_table").innerHTML = txt;
           
       });
    }
    else
        document.getElementById("recipe_table").innerHTML = txt;
}

function on_load()
{
    const allRanges = document.querySelectorAll(".range-wrap");
    allRanges.forEach(wrap => {
      const range = wrap.querySelector(".range");
      const bubble = wrap.querySelector(".bubble");

      // Every time a user move a slider even a little bit we
      //    update everything through the server
      range.addEventListener("change"/*"input"*/, () => {
        update_food_or_nutrient_map(range);
      });
    });
    
    // If there is food then show nutrient table by default, otherwise - don't
    if (food_current_values.size > 0)
        document.getElementById('nutrient_table').style.display = 'block';
    else
        document.getElementById('nutrient_table').style.display = 'none';
    
    //document.getElementById("in_mr_data_gender").addEventListener("onchange", mr_data_change);
    document.getElementById("in_mr_data_weight").addEventListener("keyup", mr_data_keyup);
    
    if (is_imperial)
    {
        document.getElementById("in_mr_data_weight").placeholder = "Weight, lb";
        document.getElementById("td_in_mr_data_height").innerHTML=`<input style="width: 50px;" id="in_mr_data_height_feet" type="text" placeholder="Height, feet">&nbsp;<input style="width: 50px;" id="in_mr_data_height_inches" type="text" placeholder="Height, inches">`;
        document.getElementById("td_in_mr_data_weight_header").innerHTML='Weight, lb';
        document.getElementById("td_in_mr_data_height_header").innerHTML="Height, ft, &Prime;";

        document.getElementById("in_mr_data_height_feet").addEventListener("keyup", mr_data_keyup);
        document.getElementById("in_mr_data_height_inches").addEventListener("keyup", mr_data_keyup);
        
        // This element can be absent if there is no food
        if (document.getElementById("oz_grams"))
            document.getElementById("oz_grams").value ="oz";
    }
    else
        document.getElementById("in_mr_data_height").addEventListener("keyup", mr_data_keyup);
    document.getElementById("in_mr_data_age").addEventListener("keyup", mr_data_keyup);
    //document.getElementById("in_mr_data_exercise").addEventListener("onchange", mr_data_change);
    //document.getElementById("in_mr_data_body_type").addEventListener("onchange", mr_data_change);
    
    // Show recipes
    show_recipes();

    // Show the meal plans select box with currently saved meal plans
    show_meal_plan_select_box();

    // Load initial data from the server
    get_data_from_server(-1);

    // Updates local maps based on what the user changed on sliders and then
    //  requests the server to rebalance, receive the response, update maps and bubles
    function update_food_or_nutrient_map(range)
    {
        //console.log(range.id, range.name, range.value);
        
        let food_id = -1;
        let nutrient_id = -1;
        let meal_group_name = '';
        let ind = range.id.indexOf("myRange_food_");
        if (ind != -1)
            food_id = Number(range.id.substring(ind + 13));
        else
        if ((ind = range.id.indexOf("myRange_nutrient_")) != -1)
            nutrient_id = Number(range.id.substring(ind + 17));
        else
            meal_group_name = range.id.substring(range.id.indexOf("myRange_meal_group_") + 19);

        let nutrient_old_value = 0;
        let nutrient_new_value = 0;

        // If a nutrient value is changed then specify explicitly its the diff between its previous value
        //  because food amounts will be updated based on the diff
        if (nutrient_id != -1)
        {
            nutrient_old_value = nutrient_current_values.get(nutrient_id);
            nutrient_new_value = range_value_by_x(range.value,
                                                  Math.abs(nutrient_reference_values.get(nutrient_id)), 10000);
            
            // Update data in the local map based on what's on a slider
            // Note: the bubble will update ONLY upon receiving the info from the server - this
            //  is the best way to get the most accurate info
            //nutrient_current_values.set(nutrient_id, nutrient_new_value);
        }
        else
        if (food_id != -1)
        {
            // Update data in the local map based on what's on a slider
            // Note: the bubble will update ONLY upon receiving the info from the server - this
            //  is the best way to get the most accurate info
            food_current_values.set(food_id,
                                    range_value_by_x(range.value, food_referenece_values.get(food_id), 10000));
        }
        else
        if (meal_group_name !== '' && meal_group_name !== undefined)
        {
            // Determine the ratio of the current meal group value to the new one
            let meal_group_cur_value = 0;
            let meal_group_ref_value = 0;
            for (let food_id of meal_groups.get(meal_group_name))
            {
                meal_group_cur_value += food_current_values.get(food_id);
                meal_group_ref_value += food_referenece_values.get(food_id);
            }

            let meal_group_new_value = range_value_by_x(range.value, meal_group_ref_value, 10000);
            let ratio = meal_group_new_value / meal_group_cur_value;

            // Update data for food in local maps for food - multiply each food in the group by the ratio
            for (let food_id of meal_groups.get(meal_group_name))
                food_current_values.set(food_id, food_current_values.get(food_id) * ratio);
        }
        
        // Inside:
        // Call the server
        // Update maps based on what we got from the server
        get_data_from_server(nutrient_id, nutrient_old_value, nutrient_new_value);
     } // function update_food_or_nutrient_map(range)
} // function on_load()

var var_add_healthy_food = 0;
function add_healthy_food()
{
    var_add_healthy_food = 1;
    
    apply_changes_from_sliders();
}
                    
function apply_changes_from_sliders(new_food_id, new_food_amount)
{
    console.log(`apply_changes_from_sliders: ${new_food_id}, ${new_food_amount}`);

    let url = assemble_meal_plan_url(new_food_id, new_food_amount);
    
    window.location.href = url;
}
                                          
function assemble_meal_plan_url(new_food_id, new_food_amount)
{
    if (new_food_id !== undefined)
        new_food_id = Number(new_food_id);
    
    // Assebmle the new balance URL
    
    // Start with food
    let url = `/balance?foods=`;
    let first = 1;
    let new_food_added = false;
    for (let [food_id, food_cur] of food_current_values)
    {
        // If new_food_id is specified and aleready exists then add it
        //  to the food amount
        if (food_id === new_food_id)
        {
            food_cur = new_food_amount;
            new_food_added = true;
        }
    
        // If food has zero amount then just skip it
        if (food_cur !== 0)
        {
            if (!first) url += ","; first = 0;
            url += `"${food_id}","${food_cur}"`;
        }
    }
    
    console.log(`assemble_meal_plan_url: new_food_added=${new_food_added}, new_food_id=${new_food_id}`);
    
    // If food_id specified and has not been added then add it
    if (!new_food_added && new_food_id !== undefined)
    {
        if (!first) url += ",";
        url += `"${new_food_id}","${new_food_amount}"`;
    }
    
    // Add nutrients
    url += "&nutrients=";
    first = 1;
    for (let [nutrient_id, nutrient_cur] of nutrient_reference_values)
    {
        if (!first) url += ","; first = 0;
        if (nutrient_cur >= 0)
            url += `"${nutrient_id}",">","${nutrient_cur}"`;
        else
        if (nutrient_cur < 0)
            url += `"${nutrient_id}","<","${-nutrient_cur}"`;
    }
    
    // Add meal groups
    url += "&groups=";
    first = 1;
    for (let [meal_group_name, foods] of meal_groups)
    {
        if (!first) url += ","; first = 0;
        url += `"${meal_group_name}"`;
        for (let food_id of foods)
            url += `,"${food_id}"`;
    }
    
    // Add mr data
    url += "&mr=";
    first = 1;
    for (let i = 0; i < 6; ++i)
    {
        if (!first) url += ","; first = 0;
        url += `"${mr_data[i]===undefined?-1:mr_data[i]}"`;
    }
    
    // Add the target goal data
    if (target_weight)
        url += `&target_weight=${target_weight}`;
    if (target_weight_deadline)
        url += `&target_weight_deadline=${target_weight_deadline}`;
    
    // Add the name of the meal plan
    if (current_meal_plan_name !== '')
        url += `&meal_plan=${current_meal_plan_name}`;
    
    if (is_imperial)
        url += "&imperial=1";

    // Add the recipe info to the URL
    // Note: add only those recipes that take part in the meal
    let all_recipes = JSON.parse(window.localStorage.getItem('BF_recipes'));
    if (all_recipes)
    {
        url += "&recipes=";
        is_first = true;
        for (let recipe of all_recipes)
        {
            let food_id = recipe[0];
            if (food_id != new_food_id && food_current_values.get(food_id) === undefined)
                continue;
            url += `${(is_first?"":",")}`;is_first = false;
            url += `"${recipe[0]}","${recipe[1]}","${(recipe.length-2)/2}"`;
            for (let i = 2; i < recipe.length; ++i)
                url += `,"${recipe[i]}"`;
        }
    }
    if (var_add_healthy_food)
        url += "&add_healthy_food=1";
    
    console.log(`assemble_meal_plan_url: new_food_id=${new_food_id}, new_food_amount=${new_food_amount}, url=${url}`);
    
    return url;
}

function auto_rebalance()
{
    // Find the nutrient that will require the minimum relative change of food amount
    //  to rebalance
    let max_distance_diff = 0;
    let max_distance = 0;
    for (let [nutrient_id, nutrient_cur] of nutrient_current_values)
    {
        let reference_value = nutrient_reference_values.get(nutrient_id);
        
        // If this nutrient is present and is not within limits (no matter good or bad)
        if (nutrient_cur &&
            reference_value > 0 && nutrient_cur < reference_value ||
            reference_value < 0 && nutrient_cur > -reference_value)
        {
            let distance = Math.abs(reference_value) / nutrient_cur;
            let distance_diff = Math.abs(1.0 - distance);
            if (distance_diff > max_distance_diff)
            {
                max_distance_diff = distance_diff;
                max_distance = distance;
            }
        }
    }
    
    // It's already perfectly rebalanced :-)
    if (!max_distance)
        return;

    if (max_distance > 1.0)
        max_distance *= 1.01;
    else
        max_distance /= 1.01;
        
    // Now just multiply amounts of foods by best_distance
    for (let [food_id, food_cur] of food_current_values)
        food_current_values.set(food_id, food_cur * max_distance);
    
    // And make all the changes to the server
    get_data_from_server(-1);
}
