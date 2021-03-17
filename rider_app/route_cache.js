/*
      Local cache for routes

      (c) Dennis Anikin 2020
*/

import * as h3core from './uber-h3-js/lib/h3core.js';

/*
    Some helper functions
*/

// Decodes a polyline to a route (array of points)
function polyline_decode(str, precision) {
    var index = 0,
        lat = 0,
        lng = 0,
        coordinates = [],
        shift = 0,
        result = 0,
        byte = null,
        latitude_change,
        longitude_change,
        factor = Math.pow(10, Number.isInteger(precision) ? precision : 5);

    // Coordinates have variable length when encoded, so just keep
    // track of whether we've hit the end of the string. In each
    // loop iteration, a single coordinate is decoded.
    while (index < str.length) {

        // Reset shift, result, and byte
        byte = null;
        shift = 0;
        result = 0;

        do {
            byte = str.charCodeAt(index++) - 63;
            result |= (byte & 0x1f) << shift;
            shift += 5;
        } while (byte >= 0x20);

        latitude_change = ((result & 1) ? ~(result >> 1) : (result >> 1));

        shift = result = 0;

        do {
            byte = str.charCodeAt(index++) - 63;
            result |= (byte & 0x1f) << shift;
            shift += 5;
        } while (byte >= 0x20);

        longitude_change = ((result & 1) ? ~(result >> 1) : (result >> 1));

        lat += latitude_change;
        lng += longitude_change;

        coordinates.push([lat / factor, lng / factor]);
    }

    return coordinates;
}

const toRad = (value: number) => (value * Math.PI) / 180;
const earthRadius = 6378137;
const robustAcos = (value: number): number => {
    if (value > 1) {
        return 1;
    }
    if (value < -1) {
        return -1;
    }

    return value;
};

// Returns the bird flight distance between two points
const getDistance = (
    from, to,
    accuracy: number = 1
) => {

  //console.log(`getDistance: ${JSON.stringify(from)}, ${JSON.stringify(to)}`)

    accuracy =
        typeof accuracy !== 'undefined' && !isNaN(accuracy) ? accuracy : 1;

    const fromLat = from.latitude;
    const fromLon = from.longitude;
    const toLat = to.latitude;
    const toLon = to.longitude;

    const distance =
        Math.acos(
            robustAcos(
                Math.sin(toRad(toLat)) * Math.sin(toRad(fromLat)) +
                    Math.cos(toRad(toLat)) *
                        Math.cos(toRad(fromLat)) *
                        Math.cos(toRad(fromLon) - toRad(toLon))
            )
        ) * earthRadius;

    return Math.round(distance / accuracy) * accuracy;
};


/*
      Stores locally (in the device RAM) cache of routes. A route is points + distance + duration

      A route is stored in a hash table witj two keys:
      a) Pair of FROM and TO in H3 index with resolution 12 (approx 10 meters)
      b) TO in H3 (points to FROM, TO pair for the sake of RAM saving)

      Note: if there are several routes with same H3 indexes for FROM and TO then consider
        them the same routes because it's come on only 10 miters miss :-)
      Note: if there are several routes with the same TO but with different FROM then
        it's very likely that we need only the recent one because it's all about either
        the navigation or a rider moving pin of A to a corrent place. So we store only
        the recent one with the same B (the H3 index of B to be more exact but c'mon 10 meters :-) )

      Algorithm works as the following:

      1.  Converts TO and FROM to H3 and finds a route
      2.  If the route is there then returns it (first hand cache hit)
      3.  If it's not then converts TO to H3 and finds a route
      4.  If the route is there then goes along the route and finds a nearest position
            for FROM
            Note: nearest not only to the points but to the edges as a projection on the edge
      5.  If it's less than 10 meters to any point of the route then
            cut the route before this point, add the point to the route and recalc
            distance and duration. Distance is done just by going through the route and
            connecting dots. Duration is Distance / (Average Speed Taken From the Route)
      6.  If the "if" in 5 works then return this cut route (second hand cache hit)
      7.  Otherwise request the server, index the route and return it (cache miss)

*/

// Statistics
var cache_hits1 = 0, cache_hits2 = 0, cache_misses = 0;

// Two-point cache
var two_point_cache = {};

// Single-point cache
var single_point_cache = {};

const RESOLUTION = 12;

// Returns a route from "from" to "to" along with distance and duration
// The resul will look like the call to on_route_ready:
//  on_route_ready({route: [points], distance: 123.45, duration: 255.11})
export function get_route(from, to, on_route_ready)
{
  //console.log(`h3index2=${JSON.stringify(h3core.geoToH3(37, -122, 11))}`);
  
  var h3_from = h3core.geoToH3(from.latitude, from.longitude, RESOLUTION);
  var h3_to = h3core.geoToH3(to.latitude, to.longitude, RESOLUTION);

  console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): h3_from=${JSON.stringify(h3_from)}, h3_to=${JSON.stringify(h3_to)}`);

  var route_first_hand = two_point_cache[{h3_from, h3_to}];

  // Found the route in a two-point cache
  if (route_first_hand !== undefined)
  {
    ++cache_hits1;

    console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): first hand cache hit, cache_hits1=${cache_hits1}, route_first_hand=${JSON.stringify(route_first_hand)}`);

    on_route_ready(route_first_hand);
    return;
  }

  var route_second_hand = single_point_cache[h3_to];

  // For debug
  var cache_miss_gone_off_route = false;

  // Found the route in a single-point-cache
  // That means that there is A ROUTE that ends up with "to"
  // Now we need to determine if we can use that route as a template to
  // get the route from "from" to "to"
  if (route_second_hand !== undefined)
  {
    ++cache_hits2;

    console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): second hand cache hit, cache_hits2=${cache_hits2}, route_second_hand=${JSON.stringify(route_second_hand)}`);

    // Search for a nearest point along the route
    // The meaning is to determine if we have the actual route in the cache that's inside
    // the existing route (which is a normal thing during car motion along its route)

    // route_second_hand is {route, distamce, duration}

    var route = route_second_hand.route;

    // Iterate all the route from the start and find the nearest to "from" point
    var min_distance = 1000000000;
    var nearest_point_n = 0;
    var n = 0;
    for (let point of route)
    {
      var distance = getDistance(point, from);
      if (distance < min_distance)
      {
        min_distance = distance;
        nearest_point_n = n;
      }
      ++n;
    }

    console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): min_distance=${min_distance}`);

    // If a driver is too far from route points then don't rebuild the route from the server
    // TODO: consider not only distances to points but to edges as well (projection)
    if (min_distance < 50 /* meters */)
    {
      var total_distance = 0;

      // Now build a new route without everything BEFORE the nearest point INCLUDING and
      // ADD the current position instead of the nearest one
      var new_route = [];
      n = 0;
      for (let point of route)
      {
        if (n == nearest_point_n)
          new_route.push(from);
        else
        if (n > nearest_point_n)
        {
          total_distance += getDistance(route[n-1], point);
          new_route.push(point);
        }
        ++n;
      }

      console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): new_route=${JSON.stringify(new_route)}`);

      // Now as we got the new route - save it in both caches
      h3_from = h3core.geoToH3(new_route[0].latitude, new_route[0].longitude, RESOLUTION);
      h3_to = h3core.geoToH3(new_route[new_route.length-1].latitude, new_route[new_route.length-1].longitude, RESOLUTION);

      // Approximate the duration of the new cut route
      var total_duration = total_distance * route_second_hand.distance / route_second_hand.duration;

      var new_route_object = {route: new_route, distance: total_distance, duration: total_duration};

      console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): new_route_object=${JSON.stringify(new_route_object)}`);

      // Save is in the two-point cache (it should be a new entry, otherwise we'd have found it
      // in the first hand :-) )
      two_point_cache[{h3_from, h3_to}] = new_route_object;

      // Save it in the single-point cache (it should replace the older uncut version)
      single_point_cache[h3_to] = new_route_object;

      // Return it
      on_route_ready(new_route_object);
      return;
    }
    else
      // For debug reasons
      cache_miss_gone_off_route = true;
  } // if (route_second_hand !== undefined)

  // Now we haven't found the route locally - request a server

  ++cache_misses;

  console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): cache miss, cache_misses=${cache_misses}, cache_miss_gone_off_route=${cache_miss_gone_off_route}`);

  var url = `http://router.project-osrm.org/trip/v1/driving/${from.longitude},${from.latitude};${to.longitude},${to.latitude}?source=first&destination=last&roundtrip=false`;

  console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): url=${url}`)
    
  // Call the server and return - the result along with cach fill will be gotten behind the scene
  fetch(url)
          .then(response => response.json())
          .then(responseJson => {
            let respJson = responseJson;

            console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): ENCODED: ${respJson.trips[0].geometry}`);

            let points = polyline_decode(respJson.trips[0].geometry, 5);
            let coords = points.map((point, index) => {
              return  {
                latitude : point[0],
                longitude : point[1]
              }
            })

            // We got the route!
            var route_object = {route: coords, distance: respJson.trips[0].distance, duration: respJson.trips[0].duration};

            // Save it in caches
            h3_from = h3core.geoToH3(from.latitude, from.longitude, RESOLUTION);
            h3_to = h3core.geoToH3(to.latitude, to.longitude, RESOLUTION);
            two_point_cache[{h3_from, h3_to}] = route_object;
            single_point_cache[h3_to] = route_object;

            console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): route_object=${JSON.stringify(route_object)}, h3_from=${h3_from}, h3_to=${h3_to} - saved in caches`)
            
            // Call the call back with the result
            on_route_ready(route_object)
          })
        .catch( err => {
          console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}):, ERROR CALLING THE SERVER BY URL: ${url}`)
        })
} // function get_route


