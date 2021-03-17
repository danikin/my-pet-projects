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

export function getGoodRotation(prevPos, curPos)
{
  var hypothenus = getDistancePointToPoint(prevPos, curPos);
  var cathetus = getDistancePointToPoint(prevPos,
      {latitude: prevPos.latitude, longitude: curPos.longitude});

  var good_rotation;
  
  // This is floating point, so cathetus could be a bit biger than hypothenus
  if (cathetus >= hypothenus)
    good_rotation = 0;
  else
    good_rotation = (Math.acos(cathetus/hypothenus) * 180.0) / Math.PI;

 // console.log(`getGoodRotation(${JSON.stringify(prevPos)}, ${JSON.stringify(curPos)})): cathetus=${cathetus}, hypothenus=${hypothenus}, acos=${good_rotation}`)

  if (curPos.longitude < prevPos.longitude)
    good_rotation = 180 - good_rotation;

  //console.log(`getGoodRotation(${JSON.stringify(prevPos)} AFTER 180, ${JSON.stringify(curPos)})): cathetus=${cathetus}, hypothenus=${hypothenus}, good_rotation=${good_rotation}`)

  if (curPos.latitude > prevPos.latitude)
    good_rotation = -good_rotation;
  
  //console.log(`getGoodRotation(${JSON.stringify(prevPos)} AFTER MINUS, ${JSON.stringify(curPos)})): cathetus=${cathetus}, hypothenus=${hypothenus}, good_rotation=${good_rotation}`)

  return good_rotation;
}

// Below is the bad functions because it does not consider different metrics for X and Y
// in terms of latitude and longitude
/*
export function getRotation(prevPos, curPos) {
    if (!prevPos) {
      return 0;
    }

  //console.log(`getRotation(${prevPos}, ${curPos})`);

    const xDiff = curPos.latitude - prevPos.latitude;
    const yDiff = curPos.longitude - prevPos.longitude;

    var bad_rotation = (Math.atan2(yDiff, xDiff) * 180.0) / Math.PI;

    var hypothenus = getDistancePointToPoint(prevPos, curPos);
    var cathetus = getDistancePointToPoint(prevPos,
      {latitude: prevPos.latitude, longitude: curPos.longitude});
    var good_rotation = (Math.acos(cathetus/hypothenus) * 180.0) / Math.PI;

    //console.log(`bad_rotation=${bad_rotation}, good_rotation=${good_rotation}`);

    return bad_rotation;
}*/

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
export function getDistancePointToPoint(from, to, accuracy: number = 1)
{

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
}

// Returns the bird flight distance from a point to a section
// Note: it works as if it iterates through all the points of the edge and
//  finds the nearest to "point"
function getDistancePointToSection(point, section_point1, section_point2)
{
  /*

      Algorithm:

      1.  Check if point inside the box formed by section vertex
      2.  If it's inside then the nearest point is the projection of "point" on
            the line "section_point1" -> "section_point2"
      3.  Otherwise the nearest point is the nearest by distance either of section verticies

  */

    if ((point.latitude < section_point1.latitude && point.latitude < section_point2.latitude ||
        point.latitude > section_point1.latitude && point.latitude > section_point2.latitude)
        
        &&
        
        (point.longitude < section_point1.longitude && point.longitude < section_point2.longitude ||
        point.longitude > section_point1.longitude && point.longitude > section_point2.longitude)
    )
    {
      // "point" is beyond the box - look at the corners
      return Math.min(getDistancePointToPoint(point, section_point1),
            getDistancePointToPoint(point, section_point2));
    }
    else
    {
      // "point" is inside the box - make a projection
      // It's: x = sqrt(a*a - 0.25*sqr(c*c + a*a - b*b)/(c*c))
      // where x - projection distance
      // a - distance between point and sections_point1
      // b - distance between point and sections_point2
      // c - distance between sections_point1 and sections_point2
      // Note: a formula through distance is better than the one through coordinate because
      //  x and y axis have different metrics
      var a = getDistancePointToPoint(point, section_point1);
      var b = getDistancePointToPoint(point, section_point2);
      var c = getDistancePointToPoint(section_point1, section_point2);

      var projection = Math.sqrt(a*a - 0.25 * (c*c + a*a - b*b) * (c*c + a*a - b*b) / (c*c));

      //console.log(`getDistancePointToSection: a=${a}, b=${b}, c=${c}, projection=${projection}`)

      return projection;
    }
}

// Returns the bird flight distance from a point to a section + a point of projection
//  {distance: 123, projection_point: {latitude: 11.11, longitude: 22.22} }
// Note: it works as if it iterates through all the points of the edge and
//  finds the nearest to "point"
// Note: the projection point is either a real projection or one of the corners - what's closer
//  to the point
function getDistancePointToSectionAndProjection(point, section_point1, section_point2)
{
  /*

      Algorithm:

      1.  Check if point inside the box formed by section vertex
      2.  If it's inside then the nearest point is the projection of "point" on
            the line "section_point1" -> "section_point2"
      3.  Otherwise the nearest point is the nearest by distance either of section verticies

  */

    if ((point.latitude < section_point1.latitude && point.latitude < section_point2.latitude ||
        point.latitude > section_point1.latitude && point.latitude > section_point2.latitude)
        
        &&
        
        (point.longitude < section_point1.longitude && point.longitude < section_point2.longitude ||
        point.longitude > section_point1.longitude && point.longitude > section_point2.longitude)
    )
    {
      // "point" is beyond the box - look at the corners
      var distance1 = getDistancePointToPoint(point, section_point1);
      var distance2 = getDistancePointToPoint(point, section_point2);
      return {
            distance: Math.min(distance1, distance2),
            projection: (distance1 < distance2) ? section_point1 : section_point2
      }
    }
    else
    {
      // "point" is inside the box - make a projection
      // It's: x = sqrt(a*a - 0.25*sqr(c*c + a*a - b*b)/(c*c))
      // where x - projection distance
      // a - distance between point and sections_point1
      // b - distance between point and sections_point2
      // c - distance between sections_point1 and sections_point2
      // Note: a formula through distance is better than the one through coordinate because
      //  x and y axis have different metrics
      var a = getDistancePointToPoint(point, section_point1);
      var b = getDistancePointToPoint(point, section_point2);
      var c = getDistancePointToPoint(section_point1, section_point2);

      var projection = Math.sqrt(a*a - 0.25 * (c*c + a*a - b*b) * (c*c + a*a - b*b) / (c*c));

      //console.log(`getDistancePointToSection: a=${a}, b=${b}, c=${c}, projection=${projection}`)

      var z = Math.sqrt(b*b - projection*projection);

      var projection_lat = section_point2.latitude +
        z*(section_point1.latitude - section_point2.latitude)/c;
      var projection_lon = section_point2.longitude +
        z*(section_point1.longitude - section_point2.longitude)/c;

      return {
        distance: projection,
        projection_point: {latitude: projection_lat, longitude: projection_lon}
      }
    }
}



// Returns the coordinate of the point that is "distance" meters
//  from "from" towards "to"
export function getPointTowardsTo(from, to, distance)
{
  var full_distance = getDistancePointToPoint(from, to);
  if (full_distance < 1)
    return from;
  var ratio = distance / full_distance;
  return {
    latitude: from.latitude + ratio * (to.latitude - from.latitude),
    longitude: from.longitude + ratio * (to.longitude - from.longitude)
  }
}

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
            Note: nearest not only to the points but to the section as a projection on the section
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
var two_point_cache = new Map();

// Single-point cache
var single_point_cache = new Map();

// It's with the edge around 10 meters
const RESOLUTION = 12;

// Returns a route from "from" to "to" along with distance and duration
// The resul will look like the call to on_route_ready:
//  on_route_ready({route: [points], distance: 123.45, duration: 255.11})
export function get_route(from, to, on_route_ready)
{
  // Sometimes a caller acts like this and this will ddos the routing server
  // So we need to workaround like this :-)
  if (from === undefined || from === {} || from.latitude === undefined || from.longitude === undefined ||
      to === undefined || to === {} || to.latitude === undefined || to.longitude === undefined)
  {
    return;
  }

  //console.log(`h3index2=${JSON.stringify(h3core.geoToH3(37, -122, 11))}`);
  
  var h3_from = h3core.geoToH3(from.latitude, from.longitude, RESOLUTION);
  var h3_to = h3core.geoToH3(to.latitude, to.longitude, RESOLUTION);

//  console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): h3_from=${JSON.stringify(h3_from)}, h3_to=${JSON.stringify(h3_to)}`);

  // It works with a string as a kye but does not work with an object as a key
  var route_first_hand = two_point_cache.get("" + h3_from + h3_to);

  // Found the route in a two-point cache
  if (route_first_hand !== undefined)
  {
    ++cache_hits1;

 // console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): first hand cache hit, cache_hits1=${cache_hits1}, two_point_cache size is ${two_point_cache.size}`);

//    console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): first hand cache hit, cache_hits1=${cache_hits1}, route_first_hand=${JSON.stringify(route_first_hand)}`);

    on_route_ready(route_first_hand);
    return;
  }

  var route_second_hand = single_point_cache.get(h3_to);

  // For debug
  var cache_miss_gone_off_route = false;

  // Found the route in a single-point-cache
  // That means that there is ROUTE that ends up with "to"
  // Now we need to determine if we can use that route as a template to
  // get the route from "from" to "to"
  // If we use it as a template then it will be one point less without rerouting (driver
  // goes along the route anyway and cuts )
  if (route_second_hand !== undefined)
  {
    // Note: it's still not a fact that this is a cache hit because the distance 
    //  from "from" to the route can be too long
//    console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): probably a second hand cache hit, cache_hits2=${cache_hits2}, route_second_hand=${JSON.stringify(route_second_hand)}, single_point_cache size is ${single_point_cache.size}`);

    // Search for a nearest point along the route
    // The meaning is to determine if we have the actual route in the cache that's inside
    // the existing route (which is a normal thing during car motion along its route)

    // route_second_hand is {route, distamce, duration}

    var route = route_second_hand.route;

    // Iterate all the route from the start and find the nearest section to "from" point
    var min_distance = 1000000000;
    var nearest_point_n = 0;
    var nearest_projection_point;
    var n = 0;
    for (let point of route)
    {
      if (n != 0)
      {
        //var distance = getDistancePointToSection(from, point, route[n-1]);
        
        // Get distance from "from" to the current section + the projection point
        //  of "from" on the section
        // Note: if a real projection is beyoind the section then return the nearest
        //  point of the section
        var d_and_p = getDistancePointToSectionAndProjection(from, point, route[n-1]);
        var distance = d_and_p.distance;
        var projection_point = d_and_p.projection_point;

        if (distance < min_distance)
        {
          nearest_projection_point = projection_point;
          min_distance = distance;
          nearest_point_n = n;
        }
      }
      ++n;
    } // for (let point of route)

    // Note: nearest_point_n == 0 means that the route comprises only one point - that route will
    // be rerouted

    /*if (min_distance != 0 || nearest_point_n == 0)
      console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): min_distance=${min_distance}, nearest_point_n=${nearest_point_n}, route.length=${route.length}, ${cache_hits1}(${two_point_cache.size}), ${cache_hits2}(${single_point_cache.size}), ${cache_misses}`);*/

    // If a driver is too far from route points then don't rebuild the route from the server
    // But! If the route was cut down to a single point (nearest_point_n == 0) then reroute anyway
    if (nearest_point_n != 0 && min_distance < 150 /* meters */)
    {
      var total_distance = 0;

      // Now build a new route without everything BEFORE the nearest point INCLUDING and
      // ADD "from" instead of the nearest one
      // Note: if for some reason "from" == rote[0] then the route remain unchanged. But then
      //  the big question is - how did this route miss the two_point_cache?
      // ^^^ WRONG!!! Read the note below
      var new_route = [];
      n = 0;
      for (let point of route)
      {
        // Note: we never spoil orignial route! We only cut it. This way we will reroute it at some
        // point. Otherwise it can have long bird flights near the offrouting driver
        // Note: we leave the nearest section, but cut everything all the sections before
        // Why? Otherwise we will always be rerouting
        if (n == nearest_point_n - 1)
          new_route.push(point);
        else
        // Here is >= istead of > because of the Note above
        if (n >= nearest_point_n)
        {
          total_distance += getDistancePointToPoint(route[n-1], point);
          new_route.push(point);
        }
        ++n;
      } // for (let point of route)

      // Only now is a second cache hit
      ++cache_hits2;

//      console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): new_route=${JSON.stringify(new_route)}`);

      // Now as we got the new route - save it in both caches
      h3_from = h3core.geoToH3(new_route[0].latitude, new_route[0].longitude, RESOLUTION);
      h3_to = h3core.geoToH3(new_route[new_route.length-1].latitude, new_route[new_route.length-1].longitude, RESOLUTION);

      // Approximate the duration of the new cut route - it's a cut distance divided by
      //  an original distance multiplied by original duration
      var total_duration = total_distance / route_second_hand.distance * route_second_hand.duration;

      var new_route_object = {route: new_route, distance: total_distance, duration: total_duration};

//      console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): new_route_object=${JSON.stringify(new_route_object)}`);

      // Save is in the two-point cache (it should be a new entry, otherwise we'd have found it
      // in the first hand :-) )
      two_point_cache.set("" + h3_from + h3_to, new_route_object);

      // Save it in the single-point cache (it should replace the older uncut version)
      single_point_cache.set(h3_to, new_route_object);

      // Now new_route_object has starts from the nearest section to "from" and all earlier
      // sections are cut. And this object has been stored in the cache. Remember - we never
      // spoil the route from the server - we only cut it. Otherwise the will be NOT REAL
      //
      // Now the object to return is DIFFERENT
      // Two scenarious:
      //
      // 1. "from" is close to the route. In this case we SUBSTITUTE the first section
      //  with ONE section: from -> route[1]
      //         | <- route[1]
      //         |
      //         |
      // (from) *| <- this point and down is cut
      //         | 
      //         |
      //         | <- route[0]
      //      
      // 2. "from" is far from the route. In this case we SUBSTITUTE the first section
      //  with TWO sections: from -> projection_point and projection_point -> route[1]
      //             | <- route[1]
      //             |
      //             |
      // (from) *****|
      //             | <- this point and down is cut
      //             |
      //             | <- route[0]
      //
      // In both scenatious we update the distance by dubstraction route[1] <-> route[0]
      //  from it and by adding either from <-> route[1] (1st scenario) or
      //  from <-> projection_point + projection_point <-> route[1] (2nd scenario)
      //  and update duration as distance change (positive or negative) multiplied by
      //  an average speed on the route
      var decorated_new_route_object;
      if (min_distance < 15)
      {
          // First scenario

        let distance_change = getDistancePointToPoint(from, new_route_object.route[1]) -             
            getDistancePointToPoint(new_route_object.route[0], new_route_object.route[1]);
        let duration_change = (new_route_object.distance != 0) ? (distance_change *
          new_route_object.duration / new_route_object.distance) : 0;

        decorated_new_route_object = {
            route: new_route_object.route,
            distance: new_route_object.distance + distance_change,
            duration: new_route_object.duration + duration_change
          }
          decorated_new_route_object.route[0] = from;

        console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}), FIRST SCENARIO OF CUT: min_distance = ${min_distance}, distance_change=${distance_change}, duration_change=${duration_change}, decorated_new_route_object.route=${JSON.stringify(decorated_new_route_object.route)}`)
      }
      else
      {
          // Second scenario

        let distance_change = min_distance +
            getDistancePointToPoint(nearest_projection_point, new_route_object.route[1]) -             
            getDistancePointToPoint(new_route_object.route[0], new_route_object.route[1]);
        let duration_change = (new_route_object.distance != 0) ? (distance_change *
          new_route_object.duration / new_route_object.distance) : 0;

        decorated_new_route_object = {
            route: [from, ... new_route_object.route],
            distance: new_route_object.distance + distance_change,
            duration: new_route_object.duration + duration_change
          }
          decorated_new_route_object.route[1] = nearest_projection_point;

        console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}), SECOND SCENARIO OF CUT: min_distance = ${min_distance}, distance_change=${distance_change}, duration_change=${duration_change}, decorated_new_route_object.route=${JSON.stringify(decorated_new_route_object.route)}`)
      }

      on_route_ready(decorated_new_route_object);

      /*

      // Return it
      // Hack: before returning we decorate the route with changing its first section
      //  with a point benining from "from" and with amended distance and duration
      //  accordingly
      // Why?
      // 1. To remove unpretty tail in the route
      // 2. To have distance and duration real time
      // Hack: if it's more than 30 meters form "from" to FIRST section
      //  then don't do it otherwise a route goes offroad and looks ugly
      var distance_to_first_section = getDistancePointToSection(from, 
        new_route_object.route[0],
        new_route_object.route[1]);
      if (distance_to_first_section < 15)
      {
        // Different in distances because of substitution the first point with "from"
        let distance_change = getDistancePointToPoint(from, new_route_object.route[1]) -             
            getDistancePointToPoint(new_route_object.route[0], new_route_object.route[1]);

        let decorated_new_route_object = new_route_object;

        let duration_change = (decorated_new_route_object.distance) ? (distance_change *
          decorated_new_route_object.duration / decorated_new_route_object.distance) : 0;

      //console.log(`duration_change=${duration_change}, distance_change=${distance_change}`)

        decorated_new_route_object.route[0] = from;
      
        decorated_new_route_object.distance += distance_change;
        decorated_new_route_object.duration += duration_change;

      //console.log(`duration_change=${duration_change}, distance_change=${distance_change}, ecorated_new_route_object.distance=${decorated_new_route_object.distance}, decorated_new_route_object.duration=${decorated_new_route_object.duration}`)

        on_route_ready(decorated_new_route_object);
      }
      else
      {

        // To void an ugly straight line to the SECOND point just add "from" before
        // the first point
        // Note: this still can result in an ugly tail but the one is better than an ugly
        // straight line above houses
        // Note and TODO! This can result in a bigger section backward on the route because
        // "from" is more than 15 meters to the side - so the code below just add this from
        // to the begining of the route and the next point is back
        // Probabl wes should consider here not just putting "from" in front but to
        // add a separate section that is projection of "from" on the nearest route section
        // and then use "from" as the first point, that projection as the second point
        // and the next point of that section as the third point. TODO!!!

        let distance_change = getDistancePointToPoint(from, new_route_object.route[0]);
        let duration_change = (new_route_object.distance) ? (distance_change *
          new_route_object.duration / new_route_object.distance) : 0;

        // Add "from" before the FIRST point and this goes ONLY to result NOT TO THE CACHE
        let decorated_new_route_object = {
          route: [from, ...new_route_object.route],
          distance: new_route_object.distance + distance_change,
          duration: new_route_object.duration + duration_change
        };

        console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}),
          distance_to_first_section=${distance_to_first_section},
          the new first and second section2 is 
          ${JSON.stringify(decorated_new_route_object.route[0])},
          ${JSON.stringify(decorated_new_route_object.route[1])},
          ${JSON.stringify(decorated_new_route_object.route[2])}`);

        on_route_ready(decorated_new_route_object);
      }*/
  
      return;
    }
    else
      // For debug reasons
      cache_miss_gone_off_route = true;
  } // if (route_second_hand !== undefined)

  // Now we haven't found the route locally - request a server

  ++cache_misses;


  var url = `http://router.project-osrm.org/trip/v1/driving/${from.longitude},${from.latitude};${to.longitude},${to.latitude}?source=first&destination=last&roundtrip=false`;

  console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): cache miss(${cache_misses}), min_distance=${min_distance}, nearest_point_n=${nearest_point_n} cache_miss_gone_off_route=${cache_miss_gone_off_route}, url=${url}`);
    
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

            // Now it's very important - add "from" and "to" as the first and last points of the route!!!
            // Why? Because thos points could be unroutable and the route was built based on
            // nearest routebale points. If we don't add them then we will be always rerouting
            // because a driver will be always to far from the first point of the route

            // No! It's bad practice because this way the route will be always in the single_point cache
            //  and will be never rerouted which will result in bird flight routes
            // A better solution for that is not to reroute until a driver reaches exactly the
            // first point. And vice versa - not to reroute when a driver gets past the last point
            // But how to do that - it's an open ended question. TODO!!!

            //coords.unshift(from);
            //coords.push(to);

            var route_object = {route: coords, distance: respJson.trips[0].distance, duration: respJson.trips[0].duration};

            two_point_cache.set("" + h3_from + h3_to, route_object);
            single_point_cache.set(h3_to, route_object);

            console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): route_object=${JSON.stringify(route_object)}, h3_from=${h3_from}, h3_to=${h3_to}, ${cache_hits1}(${two_point_cache.size}), ${cache_hits2}(${single_point_cache.size}), ${cache_misses}`)
            
            // console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): route_object=${JSON.stringify(route_object)}, h3_from=${h3_from}, h3_to=${h3_to} - saved in caches: ${JSON.stringify(two_point_cache.get("" + h3_from + h3_to))}, ${JSON.stringify(single_point_cache.get(h3_to))}`)
            
            // Call the call back with the result
            on_route_ready(route_object)
          })
        .catch( err => {
          console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}):, ERROR CALLING THE SERVER BY URL: ${url}`)
        })
} // function get_route


