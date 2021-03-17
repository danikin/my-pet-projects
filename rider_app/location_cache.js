/*
      Local cache for locations

      (c) Dennis Anikin 2020
*/

import * as h3core from './uber-h3-js/lib/h3core.js';

// Stores a cache of addresses with a 12th-resolution H3 index as a key
var coordinate_to_address_cache_ = new Map();

// It's with the edge around 10 meters
const RESOLUTION = 12;

// Resolves coordinate to address and calls on_address_ready when address
// is ready
export function resolve_coordinate_to_address(coordinate, on_address_ready)
{
  //console.log(`resolve_coordinate_to_address2(${JSON.stringify(coordinate)})`);

  if (coordinate.latitude === undefined || coordinate.longitude === undefined)
  {
    console.log(`resolve_coordinate_to_address: bad coordinate: ${JSON.stringify(coordinate)}`);
    return;
  }

  var h3 = h3core.geoToH3(coordinate.latitude, coordinate.longitude, RESOLUTION);

  var address = coordinate_to_address_cache_.get(h3);
  if (address === undefined)
  {
    // Cache miss - request the server
    var url = `http://185.241.194.113:1234/geo/get_houses_by_location?lat=${coordinate.latitude}&lon=${coordinate.longitude}&resolution=10&ring_size=3`;

    //console.log(`resolve_coordinate_to_address(${JSON.stringify(coordinate)}): cache miss, url=${url}, h3=${h3}`);

    fetch(url)
          .then(response => response.json())
          .then(responseJson => {

              // If no address is found with this ccordinate then
              // save it as an empty string for not to cache miss it again and again
              if (responseJson.data.nearest_house === undefined)
                responseJson.data.nearest_house = "";

              // Save the address in the cache
              coordinate_to_address_cache_.set(h3, responseJson.data.nearest_house);

              //console.log(`resolve_coordinate_to_address(${JSON.stringify(coordinate)}): save in the cache: h3=${h3}, responseJson.data.nearest_house=${responseJson.data.nearest_house}, coordinate_to_address_cache_.size=${coordinate_to_address_cache_.size}`);

              // Notify a caller
              on_address_ready(responseJson.data.nearest_house);
          });
  }
  else
  {
    // Take the address from the cache
    on_address_ready(address);
  }
} // export function resolve_coordinate_to_address(coordinate, on_address_ready)

/*
*   Caches of addresses
*/

// First cache: beginning letters of a street -> {full_name: street name, coordinate: undefined}
var letters_to_street_name_ = new Map();

// Second cache: beginning letters of POI -> {full_name: POI_name, coordinate: coordinate)
var letters_to_poi_name_ = new Map();

// Third cache: exact street name + more letters -> {full_name: house_full_address, coordinate: coordinate}
var exact_street_name_and_more_to_house_full_address_ = new Map();

// Forth cache: exact house full address -> {full_name: house_full_address, coordinate: coordinate}
var exact_house_full_address_to_location_ = new Map();

// Fifth cache: exact poi name -> {full_name: POI_name, coordinate: coordinate}
var exact_poi_name_to_location_ = new Map();

// Autocompletes letters to address and geo-location
// When it's ready it calls on_ready(full_name, coordinate)
// Note: coordinate can be undefined
// Note: current_location serves helps to find something in close proximity
export function auto_complete(letters, current_location, on_ready)
{
  // Test on exact POI name
  // Note: the result is 100% the best because this is the EXACT name
  let result = exact_poi_name_to_location_.get(letters);
  if (result !== undefined)
  {
    on_ready(result.full_name, result.coordinate);
    return;
  }

  // Test on exact full house address
  // Note: the result is 100% the best because this is the EXACT house addres
  result = exact_house_full_address_to_location_.get(letters);
  if (result !== undefined)
  {
    on_ready(result.full_name, result.coordinate);
    return;
  }

  // Test on exact name of a street plus more
  // Note: the result is not 100% the best - it's just a suggestion of
  //  a full address given the EXACT street name + some more letters
  result = exact_street_name_and_more_to_house_full_address_.get(letters);
  if (result !== undefined)
  {
    on_ready(result.full_name, result.coordinate);
    return;
  }

  // Test on a POI beginning
  // Note: the result is not 100% the best - it's just a suggestion of
  //  a full POI name given it's beginning
  // Note: a beginning can be in different case, in a different language, with typos
  //  it's up to the server. When the response from the server s received then it's cached here
  result = letters_to_poi_name_.get(letters);
  if (result !== undefined)
  {
    on_ready(result.full_name, result.coordinate);
    return;
  }
  
  // Test on a street beginning
  // Note: the result is not 100% the best - it's just a suggestion of a street name
  // Note: a beginning can be in different case, in a different language, with typos
  //  it's up to the server. When the response from the server s received then it's cached here
  result = letters_to_street_name_.get(letters);
  if (result !== undefined)
  {
    on_ready(result.full_name, undefined);

    // Now as a caller got EXACT street name they can later try to autocomplete
    //  this EXACT street name + something more. Which will result in a cache miss
    //  and will go to Test on a full house address
    
    return;
  }

  // Ok it's a cache miss now :-)

  // Test on something that matches the search request and in a close proximity to
  // current position
  let url=`http://185.241.194.113:1234/geo/best_street_match?s=${letters}&lat=${current_location.latitude}&lon=${current_location.longitude}`;

  // Test on a beginning of a street. Why is it before the test on a full house address?
  // Because otherwise on each beginning of a street the caller will received not the
  // exact street name but some random house which is defenitely not the best suggestion :-)
  // Why is it before the test on beginning of POI? For the very same reason - a caller
  // will get some random POI
  //let url = `http://185.241.194.113:1234/geo/find_street?s=${letters}`;

  console.log(`auto_complete(${letters}): cache miss: url=${url}`);

  //console.log(`auto_complete(${letters}): test street beginning: ${url}`);
  fetch (url).then(response => response.json()).then(responseJson => {
    if (responseJson.data !== undefined && !(responseJson.data.length === 0))
    {
      // If we found a street in close proximity
      if (responseJson.data.street.distane < responseJson.data.poi.distance)
      {
        let street_name = responseJson.data.street.street_name;

        console.log(`got street_name=${street_name}`);

        on_ready(street_name, undefined);
        let to_cache = {full_name: street_name, coordinate: undefined};

      //console.log(`auto_complete(${letters}): test street beginning: ${url} - got  street name: ${street_name}`);

        // Save the EXACT street name in the cache as a beginning of a street
        letters_to_street_name_.set(street_name, to_cache);

        letters_to_street_name_.set(letters, to_cache);
      }
      // If we found POI in close proxmity
      else
      {
        let poi_full_name = responseJson.data.poi.poi_name;
        if (!responseJson.data.poi.address.startsWith("###POI_WITHOUT_STREET###"))
          poi_full_name += " " + responseJson.data.poi.address;
        let coordinate = {latitude: responseJson.data.poi.house_latitude,
                        longitude: responseJson.data.poi.house_longitude};
        on_ready(poi_full_name, coordinate);

        let to_cache = {full_name: poi_full_name, coordinate: coordinate};

          //console.log(`auto_complete(${letters}): test on POI: ${url}, got POI: ${JSON.stringify(to_cache)}`);

        // Save exact POI name in the cache as the exact POI name
        exact_poi_name_to_location_.set(poi_full_name, to_cache);
        letters_to_poi_name_.set(letters, to_cache);
      
      } // if found close POI
    
    } // the if response is not empty

    // Haven't found a street name - test on POI
    else
    {
      // Test on POI
      url = `http://185.241.194.113:1234/geo/find_house?poi=1&s=${letters}`;
      //console.log(`auto_complete(${letters}): test on POI: ${url}`);
      fetch(url).then(response => response.json()).then(responseJson => {
        if (responseJson.data !== undefined && !(responseJson.data.length === 0))
        {
          let poi_full_name = responseJson.data[0].poi_name + " " + responseJson.data[0].full_address;
          let coordinate = {latitude: responseJson.data[0].house_latitude,
                        longitude: responseJson.data[0].house_longitude};
          on_ready(poi_full_name, coordinate);

          let to_cache = {full_name: poi_full_name, coordinate: coordinate};

          //console.log(`auto_complete(${letters}): test on POI: ${url}, got POI: ${JSON.stringify(to_cache)}`);

          // Save exact POI name in the cache as the exact POI name
          exact_poi_name_to_location_.set(poi_full_name, to_cache);

          // Save it in the cache as begining of letter of POI as they were types
          // Note: those letters could be in different case, in different language or with typos
          //  it's all up to the server how to handle it in the request above
          // Note: we save event n empty string in the cache - this will be useful to put the
          //  name of the POI in the suggestion without even typing a letter
          /*let beginning = "";
          for (let i = 0; i < letters.length; i++) {
            letters_to_poi_name_.set(beginning, to_cache);
            beginning += "" + letters.charAt(i);
          }*/
          letters_to_poi_name_.set(letters, to_cache);
        }
        // No POI - test on a full house address
        else
        {
          // Test on a full house address
          url = `http://185.241.194.113:1234/geo/find_house?s=${letters}`;
          //console.log(`auto_complete(${letters}): test on a full house address: ${url}`);
          fetch(url).then(response => response.json()).then(responseJson => {
            if (responseJson.data !== undefined && !(responseJson.data.length === 0))
            {
              let house_full_name = responseJson.data[0].full_address;
              let coordinate = {latitude: responseJson.data[0].house_latitude,
                        longitude: responseJson.data[0].house_longitude};
              on_ready(house_full_name, coordinate);

              var to_cache = {full_name: house_full_name, coordinate: coordinate};

             // console.log(`auto_complete(${letters}): test on a full house address: ${url}, got a full address: ${JSON.stringify(to_cache)}`);

              // Save the full house address in the cache as the exact house address
              exact_house_full_address_to_location_.set(house_full_name, to_cache);

              // Save the beginning in the cache as EXACT street name and more
              exact_street_name_and_more_to_house_full_address_.set(letters, to_cache);
            }
            else
            {
              //console.log(`EMPTY RESPONSE ON URL ${url}: responseJson=${JSON.stringify(responseJson)}`)
            }

          }).catch( err => {
          console.log(`ERROR CALLING THE SERVER BY URL: ${url}, err=${err}`)
        });

        } // else - full house
      
      }).catch( err => {
          console.log(`ERROR CALLING THE SERVER BY URL: ${url}, err=${err}`)
        });

    } // else - POI

  }).catch( err => {
          console.log(`ERROR CALLING THE SERVER BY URL: ${url}, err=${err}`)
        });

  
} // export function auto_complete(letters, on_address_ready, on_localtion_ready)

