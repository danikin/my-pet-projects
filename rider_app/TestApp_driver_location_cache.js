/*
      Local cache for locations

      (c) Dennis Anikin 2020
*/

import * as h3core from './uber-h3-js/lib/h3core.js';

// Stores a cache of addresses with a 12th-resolution H3 index as a key
var coordinate_to_address_cache = new Map();

// It's with the edge around 10 meters
const RESOLUTION = 12;

// Resolves coordinate to address and calls on_address_ready when address
// is ready
export function resolve_coordinate_to_address(coordinate, on_address_ready)
{
  var h3 = h3core.geoToH3(coordinate.latitude, coordinate.longitude, RESOLUTION);

  var address = coordinate_to_address_cache.get(h3);
  if (address === undefined)
  {
    // Cache miss - request the server
    var url = `http://185.241.194.113:1234/geo/get_houses_by_location?lat=${coordinate.latitude}&lon=${coordinate.longitude}&resolution=11&ring_size=1`;

    console.log(`resolve_coordinate_to_address(${JSON.stringify(coordinate)}): cache miss, url=${url}`);

    fetch(url)
          .then(response => response.json())
          .then(responseJson => {

              // Save the address in the cache
              coordinate_to_address_cache.set(h3, responseJson.data.nearest_house);

              // Notify a caller
              on_address_ready(responseJson.data.nearest_house);
          });
  }
  else
  {
    // Take the address from the cache
    on_address_ready(address);
  }
}
