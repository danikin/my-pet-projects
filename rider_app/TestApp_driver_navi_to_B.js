import React from 'react';
import MapView, { PROVIDER_GOOGLE, PROVIDER_APPLE } from 'react-native-maps';
import Marker from 'react-native-maps';
import { StyleSheet, Dimensions, FlatList, Text, View, TextInput, Button, Image, ImageBackground, Icon } from 'react-native';

import { useState, useEffect } from 'react';
import {TouchableOpacity } from 'react-native';
import { Camera } from 'expo-camera';

import MapViewDirections from 'react-native-maps';

import { AntDesign } from '@expo/vector-icons'; 


/*
const { width: winWidth, height: winHeight } = Dimensions.get('window');

const preview_styles = StyleSheet.create({
    preview: {
        height: winHeight,
        width: winWidth,
        position: 'absolute',
        left: 0,
        top: 0,
        right: 0,
        bottom: 0,
    },
});*/

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


const registration_pic_names = ['Ваш протрет', 'Главная страница паспорта', 'Прописка в паспорте', 'Права', 'Обратная сторона прав', 'СТС', 'Обратная сторона СТС', 'Чистая машина с номером'];

export default class DriverApp extends React.Component {



  constructor(props) {
    super(props);
    this.state = {
        isLoading : false,
        page :  "best_order",

        camera_type: Camera.Constants.Type.back,
        driver_region : {latitude : 61.666594 + (Math.random()-.5)*0.05, longitude : 50.827201 + (Math.random()-.5)*0.05, latitudeDelta: 0.03, longitudeDelta: 0.03},


      coords1 : []/*[{"latitude":61.65058,"longitude":50.8583},{"latitude":61.65096,"longitude":50.8587},{"latitude":61.65075,"longitude":50.85932},{"latitude":61.64985,"longitude":50.85991},{"latitude":61.65303,"longitude":50.88211},{"latitude":61.65723,"longitude":50.87968},{"latitude":61.65822,"longitude":50.87587},{"latitude":61.66329,"longitude":50.86906},{"latitude":61.66579,"longitude":50.8581},{"latitude":61.66576,"longitude":50.85613},{"latitude":61.66614,"longitude":50.85199},{"latitude":61.66803,"longitude":50.84614},{"latitude":61.66386,"longitude":50.83855},{"latitude":61.66546,"longitude":50.8346},{"latitude":61.66768,"longitude":50.83207},{"latitude":61.66518,"longitude":50.82155},{"latitude":61.66686,"longitude":50.81979},{"latitude":61.66778,"longitude":50.82372},{"latitude":61.66682,"longitude":50.82479},{"latitude":61.66686,"longitude":50.825}]*/,


      coords2 : []/*[{"latitude":61.66686,"longitude":50.825},{"latitude":61.66778,"longitude":50.82372},{"latitude":61.66633,"longitude":50.81757},{"latitude":61.66465,"longitude":50.81933},{"latitude":61.66398,"longitude":50.81653},{"latitude":61.66417,"longitude":50.81567},{"latitude":61.66385,"longitude":50.81514},{"latitude":61.66352,"longitude":50.81618},{"latitude":61.66114,"longitude":50.81868},{"latitude":61.66018,"longitude":50.81825},{"latitude":61.65675,"longitude":50.8058},{"latitude":61.65523,"longitude":50.80813},{"latitude":61.64986,"longitude":50.81914},{"latitude":61.64161,"longitude":50.80904},{"latitude":61.64165,"longitude":50.80977}]*/,

      pic_status: ["plussquareo", "plussquareo","plussquareo","plussquareo","plussquareo","plussquareo","plussquareo","plussquareo"]



    }

this.directions_ts = Date.now() - 10000;

    this.ws = new WebSocket('ws://185.241.194.113:80');

    this.ws.onopen = () => {
        // If the driver is not created yet then create them
        if (this.API_driver_id === undefined)
          this.API_registerDriver();

    };

    this.ws.onmessage = (e) => {
      //console.log("Message received: " + e.data);
      this.API_on_message(JSON.parse(e.data));
    };

    this.ws.onclose = function(e) {
      console.log('Socket is closed. Reconnect will be attempted in 1 second.', e.reason);
      setTimeout(function() {
        this.ws.connect();
      }, 1000);
    }

    this.ws.onerror = (e) => {
      console.log('Socket encountered error: ', e.message, 'Closing socket');
      this.ws.close();
    };
    
    Camera.requestPermissionsAsync();

    this.cameraRef = React.createRef();

    //this.HandleRegionChangeInAcceptedOrder = this.HandleRegionChangeInAcceptedOrder.bind(this);
  }

  componentDidMount() {

  }

// Optimizes the route of the driver:
// 1. Removes the part of the track that was already finished
// 2. Determines remaining time and distance
// 3. Sets coordinate for the next joint on the route to hint the driver
// 4. Rebuilds the route if a driver is gone offroute (TODO!)
optimizeRoute(route)
{
  var current_point = {latitude: this.driver.lat, longitude: this.driver.lon};

  // Iterate all the route from the start and find the nearest point to the driver
  var min_distance = 1000000000;
  var total_distance = 0;
  var nearest_point_n = 0;
  var n = 0;
  for (let point of route)
  {
    var distance = getDistance(point, current_point);
    if (n)
      total_distance += getDistance(route[n-1], point);
    if (distance < min_distance)
    {
      min_distance = distance;
      nearest_point_n = n;
    }
    ++n;
  }

  //console.log(`total_distance=${total_distance}, projected distance=${this.state.distance1}`)

  // Update distance information (distance to A or to B)
  if (this.state.assigned_order_status == "ride_begins")
    this.setState({distance2: total_distance})
  else
    this.setState({distance1: total_distance})

  // Now build a new route without everything BEFORE the nearest point INCLUDING and
  // ADD the current position instead of the nearest one
  var new_route = [];
  n = 0;
  for (let point of route)
  {
    if (n == nearest_point_n)
      new_route.push(current_point);
    else
    if (n > nearest_point_n)
      new_route.push(point);
    ++n;
  }

  // Find the next turn in the new route + distance to the joint
  var distance_to_turn = 0;
  n = 0;
  var prev_r;
  for (let point of new_route)
  {
    if (n >= 1)
    {
      var cur_r = this.getRotation(new_route[n-1], point);

      if (n >= 2)
      {
        var r = Math.abs(prev_r - cur_r)      
        // If the angle is sharp enough then show the marker here
        if (r > 15 && r < 165)
        {
          console.log("R=" + (prev_r - cur_r));

          var next_turn_description;
          if (prev_r - cur_r < 0)
          {
            if (prev_r - cur_r > -25)
              next_turn_description = "возьмите правее"
            else
            if (prev_r - cur_r < -100)
              next_turn_description = "резкий поворот направо"
            else
              next_turn_description = "поверните направо"
          }
          else
          {
            if (prev_r - cur_r < 25)
              next_turn_description = "возьмите левее"
            else
            if (prev_r - cur_r > 100)
              next_turn_description = "резкий поворот налево"
            else
              next_turn_description = "поверните налево"
          }

          this.setState({
            next_turn_coordinate : route[n-1],
            next_turn_distance : distance_to_turn,
            next_turn_description: next_turn_description});
          break;
        }
      }

      distance_to_turn += getDistance(route[n-1], point);
      
      prev_r = cur_r;
    }
    ++n;
  }

  return new_route;

}

directionsQueryInProgressCounter = [0, 0];
directionsQueryInProgressTimers = [[], []];

counterOfGetDirectionCalls = [0, 0];

// Returns directions
// Note: it's a request to a third-party server!!! Can be slow or blocked. TODO!
getDirections(startLoc, destinationLoc, number)
{

  //console.log(`Inside getDirections: this.directionsQueryInProgressCounter[${number}]=${this.directionsQueryInProgressCounter[number]}, this.counterOfGetDirectionCalls[${number}]=${this.counterOfGetDirectionCalls[number]}, startLoc=${startLoc.latitude, startLoc.longitude}, destinationLoc=${destinationLoc.latitude, destinationLoc.longitude}`);

  // First: clear all timers accosiated with "number" type of girections
  // Why? They are obviously obsolete
  for (let t of this.directionsQueryInProgressTimers[number])
  {
    if (t !== undefined)
      clearTimeout(t);
  }
  this.directionsQueryInProgressTimers[number] = [];

  // Second: if something is in progress then schedule this call for 1 second
  // If there are no subsequent calls within that second then this one will succeed
  // Otherwise it will be canceled in the First stage above
  var timeout = 0;
  if (this.directionsQueryInProgressCounter[number] != 0)
    timeout = 1000;

  // Run fetch inside a timer
  var timer = setTimeout(function()
  {    
    ++this.directionsQueryInProgressCounter[number];

    ++this.counterOfGetDirectionCalls[number];
  

        fetch(`http://router.project-osrm.org/trip/v1/driving/${startLoc.longitude},${startLoc.latitude};${destinationLoc.longitude},${destinationLoc.latitude}?source=first&destination=last&roundtrip=false`)

          .then(response => response.json())
          .then(responseJson => {

              let respJson = responseJson;

       // console.log(`ENCODED: ${respJson.trips[0].geometry}`);

        let points = polyline_decode(respJson.trips[0].geometry, 5);
        let coords = points.map((point, index) => {
            return  {
                latitude : point[0],
                longitude : point[1]
            }
        })

        //console.log(`getDirections3: number=${number}, coords=${JSON.stringify(coords)}`)

        if (number === 0)
          this.setState({
            coords1: coords,
            distance1 : respJson.trips[0].distance,
            duration1 : respJson.trips[0].duration})
        else
        if (number === 1)
          this.setState({
            coords2: coords,
          distance2 : respJson.trips[0].distance,
          duration2 : respJson.trips[0].duration})

        //console.log(`getDirections3: number=${number}, coords1=${JSON.stringify(this.state.coords1)}, coords2=${JSON.stringify(this.state.coords2)}`)

            //  -1 in progress
            --this.directionsQueryInProgressCounter[number];
          })
          // Don't forget to catch! Otherwise this counter will never be zero
        .catch( err => { --this.directionsQueryInProgressCounter[number]; })
   /* } catch(error) {

      console.log(`getDirections, error=${error}`);
        return error
    }*/
  }.bind(this), timeout)  // Bind is very important!

  // Save the timer
  this.directionsQueryInProgressTimers[number].push(timer)
}

/* {{ flex: 1 }} */

/*             style={{
              flex: 0.1,
              alignSelf: 'flex-end',
              alignItems: 'center',
            }}*/


/*

 onPress={() => {
              this.setState({
                camera_type: this.state.camera_type === Camera.Constants.Type.back ?
                  Camera.Constants.Type.front : Camera.Constants.Type.back
            })
            }}

            */

/*



           <TouchableOpacity
            style={{
              flex: 0.1,
              alignSelf: 'flex-end',
              alignItems: 'center',
            }}
            >



   
  
          </TouchableOpacity>


        </View>

*/

render_camera()
{
  return (
    <View style={{ flex: 1 }}>
      <Camera style={{ flex: 1 }} type={this.state.camera_type} ref={this.cameraRef}>
        <View style={{ flex: 1,
            backgroundColor: 'transparent',
              alignItems: 'center',
              flexDirection: 'column'}}>
          <View style={{ flex: 0.1}}></View>
          <Text style={{ flex: 0.7, color: 'white', fontSize: '25px', fontStyle: 'bold' }}>
            {registration_pic_names[this.state.page_camera_n]}
          </Text>
          <TouchableOpacity style={{
              flex: 0.2
            }}
            onPress={() => {
              this.cameraRef.current.takePictureAsync()
                .then((data) => {
                  this.setState({
                  takeImageText: "PICTURE TAKEN",
                  photo: data.uri,
                  page: "show_pic"
                }, console.log(data.uri))
              })
              console.log('take picture')
            }}>
            <AntDesign style={{ flex:1 }} name="camera" size={96} color="white" />
          </TouchableOpacity>
        </View>
      </Camera>
    </View>
  );

}

/*

          <Text style={{ fontSize: 18, marginBottom: 10, textAlign: 'center', color: 'white' }}> Сделать фотку </Text>
*/

/*
    Abstract away all communication to a server. API is subject to change
*/

API_driver_id = undefined;

// Returns driver by id
_API_helper_get_driver_by_id(e, driver_id)
{
    // Note: it wil search for either assigned and unassigned drivers
    for(let driver of e.data.drivers)
    {
      if (driver.driver_id == driver_id)
        return driver;
    }
    return {};
}

_API_helper_get_unassined_order_by_id(e, rider_id)
{
    for (let rider of e.data.riders)
    {
      if (rider.rider_id == rider_id && rider.status == "order")
        return rider;
    }
    return {};
}

_API_helper_get_unassined_or_assigned_order_by_id(e, rider_id)
{
    for (let rider of e.data.riders)
    {
      if (rider.rider_id == rider_id && 
          (rider.status == "order" || rider.status == "assigned_order"))
        return rider;
    }
    return {};
}

buildDirections()
{
  this.getDirections({latitude: this.driver.lat, longitude: this.driver.lon}, {latitude: this.best_order.latA, longitude: this.best_order.lonA}, 0)
  this.getDirections({latitude: this.best_order.latA, longitude: this.best_order.lonA}, {latitude: this.best_order.latB, longitude: this.best_order.lonB}, 1)
}

 getRotation(prevPos, curPos) {
    if (!prevPos) {
      return 0;
    }

  //console.log(`getRotation(${prevPos}, ${curPos})`);

    const xDiff = curPos.latitude - prevPos.latitude;
    const yDiff = curPos.longitude - prevPos.longitude;
    return (Math.atan2(yDiff, xDiff) * 180.0) / Math.PI;
  }

API_on_message(e)
{  
  // If the driver is not created yet then create them
  if (this.API_driver_id === undefined)
        this.API_registerDriver();

  // Store the prev driver info - for her/his coordinate
  if (this.driver !== undefined)
  {
    this.prev_one_driver = this.driver;
    //console.log(`this.prev_driver_info.length=${this.prev_driver_info.length}`)
  }

  // Get current driver (it may not exist yet because the registraion above could be not finished yet)
  this.driver = this._API_helper_get_driver_by_id(e, this.API_driver_id);
  if (this.driver.driver_id != this.API_driver_id)
    return;

  // If this driver is assigned then save his/her assigned order id
  this.assigned_order_id = this.driver.assigned_order_id;


  //console.log(`API_on_message3: driver=${JSON.stringify(driver)}`)

  // Get the best order
  if (this.driver.best_rider_id !== undefined)
    this.best_order = this._API_helper_get_unassined_or_assigned_order_by_id(e, this.driver.best_rider_id);


  //console.log(`BEFORE check on assigned order: this.assigned_order_id=${this.assigned_order_id}, this.API_driver_id=${this.API_driver_id}`)

  // Check if the driver is assigned
  // This is hack: we first got the best order and then do all the driver-assigned logic
  // Because best order is used to build the route to the rider, but it's better not to
  // use best order for that and switch to assigned_order. TODO!
  if (this.assigned_order_id !== undefined &&
      this.assigned_order_id != 0 &&
      this.assigned_order_id != -1)
  {
    //console.log(`The driver is assiggned3: this.API_driver_id=${this.API_driver_id}, this.assigned_order_id=${this.assigned_order_id}`)

    var distance = getDistance(
          {latitude: this.prev_one_driver.lat, longitude: this.prev_one_driver.lon},
          {latitude: this.driver.lat,longitude: this.driver.lon}
        );

    // If the distance has not changed much (less than 1 meter) then do nothing
    if (distance < 1)
      return;

    console.log(`distance=${distance}`)
    var rotation = this.getRotation(
                {latitude: this.prev_one_driver.lat,
                longitude: this.prev_one_driver.lon},

                {latitude: this.driver.lat,
                longitude: this.driver.lon})

    if (rotation != 0)
      this.setState({rotation: rotation});

    this.setState({
      driver_region : {latitude: this.driver.lat, longitude: this.driver.lon, latitudeDelta :     this.state.driver_region.latitudeDelta, longitudeDelta: this.state.driver_region.longitudeDelta},

    })

    // Each time driver's position changes - optimize the route
    // Note: it could be a route to A or to B
    if (this.state.assigned_order_status == "ride_begins")
      this.setState({coords2 : this.optimizeRoute(this.state.coords2)})
    else
      this.setState({coords1 : this.optimizeRoute(this.state.coords1)})

    // Build all needed directions and not to often
    // Note: don't build directions for accepted orders because the driver is moving and this
    // is too frequent
   // this.buildDirections();

    if (this.map !== undefined)
    {
      // Calculate rotation as average for 5 previous points
      /*var rotation = 0;
      if (this.prev_driver_info !== undefined && this.prev_driver_info.length >= 5)
      {
          var i;
          for (i = 1; i < this.prev_driver_info.length; ++i)
          {
              rotation = rotation + this.getRotation(
                {latitude: this.prev_driver_info[i-1].lat, longitude: this.prev_driver_info[i-1].lon},
                {latitude: this.prev_driver_info[i].lat, longitude: this.prev_driver_info[i].lon});
          }
          rotation /= (i-1);
      }

      console.log(`rotation2=${rotation}, i=${i}`)*/


      //console.log(`rotation2=${rotation}, this.API_driver_id=${this.API_driver_id}`)

      /*if (rotation == 0)
      {
        console.log(`${this.prev_one_driver.lat},${this.prev_one_driver.lon},${this.driver.lat},${this.driver.lon}`)
      }*/

      

      if (rotation != 0)
      {
        // If the distance from the driver and the current map is too long then
        // don't animate - just set
        if (distance > 1000)
          this.map.setCamera({heading :
            rotation,
            center: {latitude: this.driver.lat,longitude: this.driver.lon},
            pitch: 45, altitude: 300, zoom: 17});
        else
          this.map.animateCamera({heading :
            rotation,
            center: {latitude: this.driver.lat,longitude: this.driver.lon},
            pitch: 45, altitude: 300, zoom: 17}, {duration: 1100})

      }
    }

    return
  }

  //console.log(`AFTER check on assigned order: this.assigned_order_id=${this.assigned_order_id}`)

  //console.log(`API_on_message3: best_order=${JSON.stringify(best_order)}`)

  //console.log(`API_on_message2: this.API_driver_id=${this.API_driver_id}, driver.best_rider_id=${driver.best_rider_id},best_order.latA=${best_order.latA},best_order.lonA=${best_order.lonA}`)

  // If best_order's lon/lon changed then resolve them to addresses
  /*if (this.state.best_order_A === undefined || this.state.best_order_A.latitude != best_order.latA ||
      this.state.best_order_A.longitude != best_order.lonA)
  {
    var url = "http://185.241.194.113:1234/geo/get_houses_by_location?lat=" +
best_order.latA + "&lon=" + best_order.lonA + "&resolution=11&ring_size=1";

    //console.log(url);

    fetch(url)
          .then(response => response.json())
          .then(responseJson => {
            this.setState({addr_A : responseJson.data.nearest_house})
          });
  }
  if (this.state.best_order_B === undefined || this.state.best_order_B.latitude != best_order.latB ||
      this.state.best_order_B.longitude != best_order.lonB)
  {
    fetch("http://185.241.194.113:1234/geo/get_houses_by_location?lat=" +
best_order.latB + "&lon=" + best_order.lonB + "&resolution=11&ring_size=1")
          .then(response => response.json())
          .then(responseJson => {
            this.setState({addr_B : responseJson.data.nearest_house})
          });
  }*/
  // Change the state from info from web socket
  // Note: a driver can be changed outside by the user of the test map and
  //  that change will be shown here
  this.setState({
        best_order_price: this.best_order.price,
        best_order_pickup_ETA: this.best_order.best_pickup_ETA,
        best_order_pickup_distance: this.best_order.best_pickup_distance,
        best_order_id: this.best_order.unassigned_order_id,
        best_order_A : {latitude: this.best_order.latA, longitude: this.best_order.lonA},
        best_order_B : {latitude: this.best_order.latB, longitude: this.best_order.lonB},

        driver_region : {latitude: this.driver.lat, longitude: this.driver.lon, latitudeDelta : this.state.driver_region.latitudeDelta, longitudeDelta: this.state.driver_region.longitudeDelta}
    })

  //console.log(`best_order_id = ${this.state.best_order_id}`)

  //console.log(`BEFORE this.buildDirections(): page=${this.state.page}, this.assigned_order_id=${this.assigned_order_id}`)

  // Build all needed directions and not to often
  this.buildDirections()

  // Don't call the address resolving to frequently
  var now = Date.now();
  if (!(this.directions_ts === undefined) && now - this.directions_ts < 1000)
  {
   // console.log("SKIP DIRECTIONS AND ADDR RESOLVING IN on_message")
    return
  }

  //console.log("now2=" + now + ", diff2=" + (now - this.directions_ts))

  this.directions_ts = now

  // Resolve best_order's lon/lon to addresses

    var url_A = "http://185.241.194.113:1234/geo/get_houses_by_location?lat=" +
this.best_order.latA + "&lon=" + this.best_order.lonA + "&resolution=11&ring_size=1";

    //console.log(`url_A=${url_A}`);

    fetch(url_A)
          .then(response => response.json())
          .then(responseJson => {
            this.setState({addr_A : responseJson.data.nearest_house})
          });

    var url_B = "http://185.241.194.113:1234/geo/get_houses_by_location?lat=" +
this.best_order.latB + "&lon=" + this.best_order.lonB + "&resolution=11&ring_size=1";

    //console.log(`url_B=${url_A}`);

    fetch(url_B)
          .then(response => response.json())
          .then(responseJson => {
            this.setState({addr_B : responseJson.data.nearest_house})
          });

          //console.log('BEFORE this.getDirections');

}

// Send a message from the driver that the ride has begun
API_ride_begins()
{
  this.ws.send(`{"event":"test_start_ride","data":{"driver_id":${this.API_driver_id}}}`);

  // Save ride begins timestamp as a starting time to go to B
  this.ride_begins_ts = Date.now();

  this.setState({assigned_order_status: "ride_begins"})
}

// Registers a new driver
API_registerDriver()
{
  if (this.API_driver_id === undefined)
  {
    // Create a driver at a random place with a random id :-)
    this.API_driver_id = Math.round(Math.random() * 1000000000);

    this.ws.send(`{"event":"test_newdriver", "data":{"id":${this.API_driver_id },"lat":${this.state.driver_region.latitude},"lon":${this.state.driver_region.longitude}}}`)
  }

}

// Accepts the best order by the driver
API_accept_best_order()
{
    console.log(`API_accept_best_order:  this.state.best_order_id=${this.state.best_order_id}, this.API_driver_id=${this.API_driver_id}`);

  if (this.API_driver_id !== undefined || this.state.best_order_id !== undefined)
  {
    this.ws.send(`{"event":"test_accept_order", "data":{"rider_id":${this.state.best_order_id},"driver_id":${this.API_driver_id}}}`)

    console.log(`order ${this.state.best_order_id} is accepted by driver ${this.API_driver_id}`);

    // Save order acceptance timestamp as a starting time to go to A
    this.order_acceptance_ts = Date.now();

    // Set this state here before the answer from the server to prepare the map in the meanwhile
    this.setState({page: "assigned_order"})
  }

}

/*HandleRegionChangeInAcceptedOrder(region)
{
  this.setState(
    {driver_marker_coordinate: {latitude: region.latitude, longitude: region.longitude}}
  )
}*/

render()
{


  /*if (this.state.isLoading) {
      return (
        <View style={{ flex: 1, padding: 20, separator: {
    marginVertical: 8,
    borderBottomColor: '#737373',
    borderBottomWidth: 1,
  } }}>
          <ActivityIndicator />
        </View>
      );
    }*/


    // Registration
    if (this.state.page == "registration")
    {
      var is_reg_done = true;
      for (let i of this.state.pic_status)
         {
           if (i != 'checksquare')
           {
             is_reg_done = false;
             break;
           }
         }

      var button_next;
      if (is_reg_done)
        button_next = (<Text 
        

        onPress={() => {this.setState({page: "best_order"})}}

        style={{flex: 0.8, borderColor: 'black', borderWidth: 1, backgroundColor: 'green', color: 'black', textShadowColor: 'rgba(0, 0, 0, 0.25)',textAlign: 'center', 
        verticalAlign: 'center',
        
textShadowOffset: {width: 0, height: 4},
textShadowRadius: 4,
fontFamily: 'Roboto',
fontStyle: 'bold',
fontWeight: 900,
alignItems: 'center',
letterSpacing: '0.04em',
textTransform: 'uppercase',
        fontSize: '30px'}}
        
        >Начать работать</Text>);
      else
        button_next = (<Text
        
        style={{flex: 0.8, borderColor: 'black', borderWidth: 1, backgroundColor: '#FFFFFF', color: 'black', textShadowColor: 'rgba(0, 0, 0, 0.25)',textAlign: 'center', 
        verticalAlign: 'center',
        
textShadowOffset: {width: 0, height: 4},
textShadowRadius: 4,
fontFamily: 'Roboto',
fontStyle: 'bold',
fontWeight: 900,
alignItems: 'center',
letterSpacing: '0.04em',
textTransform: 'uppercase',
        fontSize: '17px'}}
        
        >Ждем ваши фотки :-)</Text>);


        // Show a registration form
        return (
<View style={{flex: 1, justifyContent: 'space-around'}}>

    <View style={{flex: 0.8}}></View>

    <Text  style={{flex: 0.8, backgroundColor: 'white', color: 'black', textShadowColor: 'rgba(0, 0, 0, 0.25)',textAlign: 'center', 
        verticalAlign: 'center',
        
textShadowOffset: {width: 0, height: 4},
textShadowRadius: 4,
fontFamily: 'Roboto',
fontStyle: 'bold',
fontWeight: 900,
alignItems: 'center',
letterSpacing: '0.04em',
textTransform: 'uppercase',
        fontSize: '25px'}}>Регистрация</Text>



        <View style={{flex: 0.8, flexDirection: 'row', justifyContent: 'flex-end'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth:1,fontSize:'17px',fontWeight: 'bold', flex: 0.9, height: 45}}>Ваш портрет</Text>
          <AntDesign style={{flex:0.2}} name={this.state.pic_status[0]} size={48} color="black" onPress={() => {this.setState({page: "camera", page_camera_n: 0, camera_type: Camera.Constants.Type.front})}}/>
        </View>

        <View style={{flex: 0.8, flexDirection: 'row', justifyContent: 'center'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth:1,fontSize:'17px',fontWeight: 'bold', flex: 0.9, height: 45 }}>Главная страница паспорта</Text>
          <AntDesign style={{flex:0.2}} name={this.state.pic_status[1]} size={48} color="black" onPress={() => {this.setState({page: "camera", page_camera_n: 1, camera_type: Camera.Constants.Type.back})}}/>
        </View>

        <View style={{flex: 0.8, flexDirection: 'row', justifyContent: 'space-around'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth:1,fontSize:'17px',fontWeight: 'bold', flex: 0.9 , height: 45}}>Прописка в паспорте</Text>
          <AntDesign style={{flex:0.2}} name={this.state.pic_status[2]} size={48} color="black" onPress={() => {this.setState({page: "camera", page_camera_n: 2, camera_type: Camera.Constants.Type.back})}}/>
        </View>

        <View style={{flex: 0.8, flexDirection: 'row', justifyContent: 'space-between'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth:1,fontSize:'17px',fontWeight: 'bold', flex: 0.9,  height: 45}}>Права</Text>
          <AntDesign style={{flex:0.2}} name={this.state.pic_status[3]} size={48} color="black" onPress={() => {this.setState({page: "camera", page_camera_n: 3, camera_type: Camera.Constants.Type.back})}}/>
        </View>

        <View style={{flex: 0.8, flexDirection: 'row', justifyContent: 'spce-evenly'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth:1,fontSize:'17px',fontWeight: 'bold', flex: 0.9,  height: 45}}>Обратная сторона прав</Text>
          <AntDesign style={{flex:0.2}} name={this.state.pic_status[4]} size={48} color="black" onPress={() => {this.setState({page: "camera", page_camera_n: 4, camera_type: Camera.Constants.Type.back})}}/>
        </View>


        <View style={{flex: 0.8, flexDirection: 'row'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth:1,fontSize:'17px',fontWeight: 'bold', flex: 0.9,  height: 45 }}>СТС</Text>
          <AntDesign style={{flex:0.2}} name={this.state.pic_status[5]} size={48} color="black" onPress={() => {this.setState({page: "camera", page_camera_n: 5, camera_type: Camera.Constants.Type.back})}}/>
        </View>

        <View style={{flex: 0.8, flexDirection: 'row'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth:1,fontSize:'17px',fontWeight: 'bold', flex: 0.9,  height: 45 }}>Обратная сторона СТС</Text>
          <AntDesign style={{flex:0.2}} name={this.state.pic_status[6]} size={48} color="black" onPress={() => {this.setState({page: "camera", page_camera_n: 6, camera_type: Camera.Constants.Type.back})}}/>
        </View>
        <View style={{flex: 0.8, flexDirection: 'row'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth:1,fontSize:'17px',fontWeight: 'bold', flex: 0.9,  height: 45 }}>Чистая машина с номером</Text>
          <AntDesign style={{flex:0.2}} name={this.state.pic_status[7]}size={48} color="black" onPress={() => {this.setState({page: "camera", page_camera_n: 7, camera_type: Camera.Constants.Type.back})}}/>
        </View>

        {button_next}




      <View style={{flex: 0.2, flexDirection: 'row'}}>

      </View>

</View>
        )
    }
    else
   // Camera page
    if (this.state.page == "camera")
    {
      return this.render_camera();
    }
    else
    // Showing pic after it's made
    if (this.state.page == "show_pic")
    {
      return (
        <View style={{flex:1}}>
         
            <ImageBackground style={{flex:1, alignItems: 'center',
              flexDirection: 'column'}} source={[   {uri: this.state.photo} ]}>

              <View style={{flex:0.7}}>
                <View style={{flex:0.1}}/>
                <Text style={{ flex: 0.9, color: 'white', fontSize: '25px', fontStyle: 'bold' }}>
            {registration_pic_names[this.state.page_camera_n]}
                </Text>
              </View>

              <View style={{flex:0.3, flexDirection: 'row', alignItems: 'center', justifyContent: 'space_between'}}>
                
                <View style={{flex:0.5, flexDirection: 'column'}}>
                  <TouchableOpacity style={{flex:0.6}} onPress={() => {
                    this.state.pic_status[this.state.page_camera_n] = 'checksquare';
                    this.setState({page: "registration"
                    })}}>
                    <AntDesign name="checksquare" size={96} color="white"/>
                  </TouchableOpacity>
                  <Text style={{flex:0.4, fontSize: '25px', color: 'white'}}>Сохранить</Text>
                </View>

                <View style={{flex:0.5, flexDirection: 'column'}}>
                  <TouchableOpacity style={{flex:0.6}} onPress={() => {
                    this.setState({page: "camera"
                    })}}>
                    <AntDesign name="camera" size={96} color="white"/>
                  </TouchableOpacity>
                  <Text style={{flex:0.4, fontSize: '25px', color: 'white'}}>Переснять</Text>
                </View>

              </View>

            </ImageBackground>
      </View>
      )
    }
    else
    if (this.state.page == "best_order")
    {

/*
     this.map.animateCamera({center: {latitude: 61.666594,longitude: 50.827201},pitch: 2, heading: 20,altitude: 200, zoom: 40, tilt: 30}, 50000)

     region={this.state.driver_region}
*/

      // If there is no best order then this means that there are no orders at all
      if (this.state.best_order_id === undefined ||
          this.state.best_order_id == 0 ||
          this.state.best_order_id == -1)
          return (<View style={styles.container}>
              <MapView style={styles.mapStyle} provider={PROVIDER_APPLE} initialRegion={{
                latitude: 61.666594,
                longitude: 50.827201,
                latitudeDelta: 0.006,
                longitudeDelta: 0.006
              }} 
              showsBuildings
              ref={ref => { this.map = ref }}
              onLayout={() => {
             //   this.map.animateToBearing(125);
                //this.map.animateToViewingAngle(85, 3000);
           
                this.map.animateCamera({center: {latitude: 61.666594,longitude: 50.827201}, pitch: 45})
              }}
              
              
              >
                
              </MapView>

              <TouchableOpacity activeOpacity={1}>
                <Text
       style={{fontFamily: 'Roboto', fontSize: '16px', fontWeight: 'bold', lineHeight: 23,
display: 'flex',
alignItems: 'center',
letterSpacing: '0.04em',
textTransform: 'uppercase',
color: '#000000',
textShadowColor: 'rgba(0, 0, 0, 0.25)',
textShadowOffset: {width: 0, height: 4}, textShadowRadius: 4
       }}
      >Ищем лучший заказ для вас ...</Text>
              </TouchableOpacity>
            </View>
          );
      // If there is a best order then show it the driver
      else
        return (
      <View style={styles.container}>
        <MapView provider={PROVIDER_GOOGLE} style={styles.mapStyle}
          initialRegion={{
            latitude: 61.666594,
            longitude: 50.827201,
            latitudeDelta: 0.03,
            longitudeDelta: 0.03
          }}
          region={this.state.driver_region}>

<MapView.Marker coordinate={this.state.driver_region}/>
<MapView.Marker pinColor='green' coordinate={this.state.best_order_A}/>
<MapView.Marker pinColor='black' coordinate={this.state.best_order_B}/>

<MapView.Polyline lineDashPattern={[25,25]}
coordinates={this.state.coords1}
	strokeColor='green'
	strokeWidth={3}/>

  <MapView.Polyline lineDashPattern={[25,25]}
coordinates={this.state.coords2}
	strokeColor='black'
	strokeWidth={3}/>

        </MapView>


      <Text
       style={{

position: 'absolute',
width: Dimensions.get('window').width,
height: 23,
left: 0,
top: 61,

fontFamily: 'Roboto',
fontWeight: 'bold',
fontSize: '20px',
lineHeight: 23,
display: 'flex',
textAlign: 'center',
alignItems: 'center',
letterSpacing: '0.04em',
textTransform: 'uppercase',

color: '#000000',

textShadowColor: 'rgba(0, 0, 0, 0.25)',
textShadowOffset: {width: 0, height: 4},
textShadowRadius: 4

       }}
      >Самый выгодный заказ</Text>

        <Text style={{ position: 'absolute', left : 10, right : 10, top: 100, height: 40, borderColor: 'gray', backgroundColor: 'white',
        borderWidth: 3, fontSize: '19px', fontWeight: 'bold'}}>{this.state.addr_A}</Text>

<View

style={{
position: 'absolute', left : 150, top: 145,
width: 0,
    height: 0,
    backgroundColor: 'transparent',
    borderStyle: 'solid',
    borderLeftWidth: 10,
    borderRightWidth: 10,
    borderTopWidth: 15,
    borderLeftColor: 'transparent',
    borderRightColor: 'transparent'

}}

/>

        <Text style={{ position: 'absolute', left : 10, right : 10, top: 165, height: 40, borderColor: 'gray', backgroundColor: 'white',
        borderWidth: 3, fontSize: '19px', fontWeight: 'bold'}}>{this.state.addr_B}</Text>


<Text style={{
position: 'absolute',
width: 400,
height: 146,
left: 0,
top: 200,

fontFamily: 'Roboto',
fontStyle: 'normal',
fontWeight: 900,
fontSize: '30px',
lineHeight: 59,
display: 'flex',
alignItems: 'center',
textAlign: 'center',
letterSpacing: '0.04em',
textTransform: 'uppercase',

color: '#000000',

textShadowOffset: {width: 0, height: 4},
textShadowRadius: 4,
textShadowColor: 'rgba(0, 0, 0, 0.25)'

}}>{Math.round(this.state.best_order_price)} <Text style={{fontSize: '19px'}}>руб</Text></Text>

<Text style={{ position: 'absolute', left : 10, right : 10, top: 250, height: 40,
        fontSize: '19px', fontWeight: 'bold'}}>Подача {(this.state.duration1/60.0).toFixed(1)} минут, {(this.state.distance1/1000.0).toFixed(1)}км</Text>
<Text style={{ position: 'absolute', left : 10, right : 10, top: 270, height: 40,
        fontSize: '19px', fontWeight: 'bold'}}>Поездка {(this.state.duration2/60.0).toFixed(1)} минут, {(this.state.distance2/1000.0).toFixed(1)}км</Text>


        <TouchableOpacity style={{position: 'absolute', left : 10, right : 10, bottom: 80, height: 50,  alignItems: "center", backgroundColor: "#AAFFAA", padding: 10, borderColor: 'black', borderWidth: 1}}
          onPress={() => this.API_accept_best_order()}>
          <Text style={{ fontSize: '25px', textTransform: 'uppercase', fontFamily: 'Roboto', fontWeight: 'bold'}}>Принять заказ</Text>
        </TouchableOpacity>

      </View>
      );

// style={{ position: 'absolute', left : 10, right : 10, bottom: 80, height: 40}}

    }
    else
    // The driver has an assigned order
    if (this.state.page == "assigned_order")
    {
      // Get miutes and seconds to go
      var minutes;
      var seconds;
      
      if (this.state.assigned_order_status == "ride_begins")
        seconds = Math.trunc((this.ride_begins_ts - Date.now()) / 1000 + this.state.duration2)
      else
        seconds = Math.trunc((this.order_acceptance_ts - Date.now()) / 1000 + this.state.duration1)
      if (seconds <= 0)
        seconds = 0;
      else
      {
        // Get round number of minutes
        minutes = Math.trunc(seconds / 60)
  
        // Get number of seconds within a minute
        seconds = seconds % 60;
      }

      // Get distance to go
      var distance_to_go = (((this.state.assigned_order_status == "ride_begins") ?
        this.state.distance2 : this.state.distance1)/1000).toFixed(1);


      var render_duration_and_distance;

      // If we have not got assigned order id yet then don't show anythins on the
      // assigned_order page because otherwise it would be like we got A which is not true
      if (this.assigned_order_id === undefined || this.assigned_order_id == -1)
        render_duration_and_distance = (<View></View>)
      else
      if (!minutes && !seconds || !distance_to_go)
        render_duration_and_distance = (
          <View style={{flex: 1, position: 'absolute', bottom: 10, height: 40, flexDirection: 'row'}}>
            <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: '15'}}>приехали</Text>
          </View>
        )
      else
      if (!minutes)
        render_duration_and_distance = (
          <View style={{flex: 1, position: 'absolute', bottom: 10, height: 40, flexDirection: 'row'}}>
              <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: '15'}}>{seconds} сек, </Text>
              <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: '15'}}>{distance_to_go} км</Text>
          </View>)
      else
        render_duration_and_distance = (
          <View style={{flex: 1, position: 'absolute', bottom: 10, height: 40, flexDirection: 'row'}}>
              <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: '15'}}>{minutes} мин {seconds} сек, </Text>
              <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: '15'}}>{distance_to_go} км</Text>
          </View>)


      var render_assigned_order_info = (<View></View>)

      // Render various assigned order related buttons depending on its status
      if (this.state.assigned_order_status == "ride_begins")
      {
        render_assigned_order_info = (<View></View>)
      }
      else
      if (this.state.assigned_order_status == "at_A")
      {
        render_assigned_order_info = (<TouchableOpacity style={{flex: 1, position: 'absolute', left : 10, right : 10, alignItems: "center", backgroundColor: "rgba(100,255,100,100)", padding: 10, borderColor: 'black', borderWidth: 1}} onPress={() => {
              // Begin the ride
              this.API_ride_begins();
              this.setState({assigned_order_status: "ride_begins"})
            }}>
            <Text style={{fontFamaly: 'roboto', textTransform: 'uppercase', fontStyle: 'bold', fontSize: '25'}}> Начинаю поездку </Text>
          </TouchableOpacity>)
      }
      else
      // On the way to A
      {
          // If the driver is close to A then show her/him the in-place button
          if (this.state.distance1 < 200)
            render_assigned_order_info = (<TouchableOpacity style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 30, alignItems: "center", backgroundColor: "rgba(100,255,100,100)", padding: 10, borderColor: 'black', borderWidth: 1}} onPress={() => { return this.setState({assigned_order_status: "at_A"})}}>
            <Text style={{fontFamaly: 'roboto', textTransform: 'uppercase', fontStyle: 'bold', fontSize: '25'}}> На месте </Text>
            </TouchableOpacity>)
          else
            render_assigned_order_info = (<View></View>)
      }
/*

            <MapView.Marker coordinate={this.state.driver_region}
            ref={ref => { this.marker = ref}}
            
            >
              <View></View>
              <MapView.Callout><Text>Turn left</Text></MapView.Callout>


              onRegionChangeComplete={() => this.marker.showCallout()}
*/
      // Here should be a navigtor to the to pickup point
      return (
          <View style={styles.container}>
            <MapView provider={PROVIDER_GOOGLE} style={styles.mapStyle}

              initialRegion={{
                latitude: 61.666594,
                longitude: 50.827201,
                latitudeDelta: 0.1,
                longitudeDelta: 0.1
              }}
             // region={this.state.driver_region}
             // initialRegion={this.state.driver_region}


             //onRegionChangeComplete={(region) => {this.HandleRegionChangeInAcceptedOrder(region)}}

              //region={this.state.driver_region}

                showsBuildings
                ref={ref => { this.map = ref }}

             // onLayout={() => {this.map.setCamera({center: {latitude: 61.666594,longitude: 50.827201}})

             //   this.map.animateToBearing(125);
                //this.map.animateToViewingAngle(85, 3000);
           
                //this.map.animateCamera({center: {latitude: 61.666594,longitude: 50.827201}, pitch: 45})
              // rotation={this.state.rotation}
             
             // }}

             // onMapReady={() => {
               // this.map.setCamera({center: {latitude: 61.666594,longitude: 50.827201}})}}
               onRegionChangeComplete={() => this.marker.showCallout()}
          >

         

            <MapView.Marker coordinate={this.state.driver_region}/*coordinate={this.state.driver_marker_coordinate}*//>
            <MapView.Marker pinColor='green' coordinate={this.state.best_order_A}/>
            <MapView.Marker pinColor='black' coordinate={this.state.best_order_B}/>

            <MapView.Marker coordinate={this.state.next_turn_coordinate} ref={ref => { this.marker = ref}}>
              <View>
              <MapView.Callout style={{backgroundColor: '#000077'}}><Text style={{color: 'black', fontFamily: 'roboto', fontSize: '15'}}>Через {Math.round(this.state.next_turn_distance)} метров{"\n"}{this.state.next_turn_description}</Text></MapView.Callout>
              </View>
            </MapView.Marker>

            <MapView.Polyline
              coordinates={this.state.coords1}
              lineDashPattern={[25,1]}
	            strokeColor='rgba(0, 255, 0, 0.20)'
	            strokeWidth={20}/>

            <MapView.Polyline
              coordinates={this.state.coords2}
              lineDashPattern={[25,1]}
	            strokeColor='rgba(0, 0, 0, 0.50)'
	            strokeWidth={20}/>
          </MapView>

          {render_assigned_order_info}
          {render_duration_and_distance}

        </View>
        )
    }
    else
    {
      console.log(`UNKNOWN PAGE: ${this.state.page}`)
      return (
        <View/>
      )
    }


}


}
// #fff'
const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',

    backgroundColor: 'rgba(0, 0, 0, 0)'

  },
  mapStyle: {
    position: 'absolute',
    width: Dimensions.get('window').width,
    height: Dimensions.get('window').height,
    backgroundColor: 'rgba(0, 0, 0, 0.25)'
  },
});


