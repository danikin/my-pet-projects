/*
      Bulding routes

      (c) Dennis Anikin 2020
*/


/*
    Some helper functions
*/

export class LatLng
{
    latitude: number;
    longitude: number;

    constructor(latitude: number, longitude: number)
    {
        this.latitude = latitude;
        this.longitude = longitude;
    }
};

// Decodes a polyline to a route (array of points)
function polyline_decode(str : string, precision : number) : LatLng[] {
    var index = 0,
        lat = 0,
        lng = 0,
        coordinates : LatLng[] = [],
        shift = 0,
        result = 0,
        byte = null,
        latitude_change : number,
        longitude_change : number,
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

        coordinates.push(new LatLng(lat / factor, lng / factor));
    }

    return coordinates;
}

export class RouteObject
{
    route: LatLng[] = [];
    distance: number = 0;
    duration: number = 0;
};

// Returns a route from "from" to "to" along with distance and duration
// The resul will look like the call to on_route_ready:
//  on_route_ready({route: [points], distance: 123.45, duration: 255.11})
export function get_route(from :LatLng,
        to : LatLng,
        on_route_ready : (route_object : RouteObject) => void)
{
    return;
    
  var url = `http://router.project-osrm.org/trip/v1/driving/${from.longitude},${from.latitude};${to.longitude},${to.latitude}?source=first&destination=last&roundtrip=false`;

  console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}`);

  // Call the server and return - the result along with cach fill will be gotten behind the scene
  fetch(url)
          .then(response => response.json())
          .then(responseJson => {
            let respJson = responseJson;

            console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): ENCODED: ${respJson.trips[0].geometry}`);

            let coords = polyline_decode(respJson.trips[0].geometry, 5);

            // We got the route!

            var route_object : RouteObject = {
                route : coords,
                distance: respJson.trips[0].distance,
                duration: respJson.trips[0].duration};

            console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}): route_object=${JSON.stringify(route_object)}`);
            
            // Call the call back with the result
            on_route_ready(route_object)
          })
        .catch( err => {
          console.log(`get_route(${JSON.stringify(from)}, ${JSON.stringify(to)}):, ERROR CALLING THE SERVER BY URL: ${url}`)
        })
} // function get_route

