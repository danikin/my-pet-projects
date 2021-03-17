import React from 'react';
import MapView, { PROVIDER_GOOGLE, PROVIDER_APPLE } from 'react-native-maps';
import Marker from 'react-native-maps';
import { StyleSheet, Dimensions, FlatList, Text, View, TextInput, Button, Image, ImageBackground, Icon } from 'react-native';

import { useState, useEffect } from 'react';
import {TouchableOpacity } from 'react-native';
import { Camera } from 'expo-camera';

import MapViewDirections from 'react-native-maps';

import { AntDesign } from '@expo/vector-icons';
import { Feather } from '@expo/vector-icons'; 

import * as route_cache from './components/route_cache.js'

import * as h3core from './components/uber-h3-js/lib/h3core.js';

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



const registration_pic_names = ['Ваш протрет', 'Главная страница паспорта', 'Прописка в паспорте', 'Права', 'Обратная сторона прав', 'СТС', 'Обратная сторона СТС', 'Чистая машина с номером'];

export default class DriverApp extends React.Component {



  constructor(props) {
    super(props);
    this.state = {
        isLoading : false,
        page :  "best_order",
        next_turn_direction: "",

        camera_type: Camera.Constants.Type.back,
        driver_region : {latitude : 61.666594 + (Math.random()-.5)*0.05, longitude : 50.827201 + (Math.random()-.5)*0.05, latitudeDelta: 0.03, longitudeDelta: 0.03},


      coords1 : []/*[{"latitude":61.65058,"longitude":50.8583},{"latitude":61.65096,"longitude":50.8587},{"latitude":61.65075,"longitude":50.85932},{"latitude":61.64985,"longitude":50.85991},{"latitude":61.65303,"longitude":50.88211},{"latitude":61.65723,"longitude":50.87968},{"latitude":61.65822,"longitude":50.87587},{"latitude":61.66329,"longitude":50.86906},{"latitude":61.66579,"longitude":50.8581},{"latitude":61.66576,"longitude":50.85613},{"latitude":61.66614,"longitude":50.85199},{"latitude":61.66803,"longitude":50.84614},{"latitude":61.66386,"longitude":50.83855},{"latitude":61.66546,"longitude":50.8346},{"latitude":61.66768,"longitude":50.83207},{"latitude":61.66518,"longitude":50.82155},{"latitude":61.66686,"longitude":50.81979},{"latitude":61.66778,"longitude":50.82372},{"latitude":61.66682,"longitude":50.82479},{"latitude":61.66686,"longitude":50.825}]*/,


      coords2 : []/*[{"latitude":61.66686,"longitude":50.825},{"latitude":61.66778,"longitude":50.82372},{"latitude":61.66633,"longitude":50.81757},{"latitude":61.66465,"longitude":50.81933},{"latitude":61.66398,"longitude":50.81653},{"latitude":61.66417,"longitude":50.81567},{"latitude":61.66385,"longitude":50.81514},{"latitude":61.66352,"longitude":50.81618},{"latitude":61.66114,"longitude":50.81868},{"latitude":61.66018,"longitude":50.81825},{"latitude":61.65675,"longitude":50.8058},{"latitude":61.65523,"longitude":50.80813},{"latitude":61.64986,"longitude":50.81914},{"latitude":61.64161,"longitude":50.80904},{"latitude":61.64165,"longitude":50.80977}]*/,

      pic_status: ["plussquareo", "plussquareo","plussquareo","plussquareo","plussquareo","plussquareo","plussquareo","plussquareo"]



    }

this.directions_ts = Date.now() - 10000;


  console.log(`new WebSocket`)
  
    this.ws = new WebSocket('ws://185.241.194.113:80');

    this.ws.onopen = () => {
        console.log(`this.ws.onopen`)

        // If the driver is not created yet then create them
        if (this.API_driver_id === undefined)
          this.API_registerDriver();

       console.log(`this.ws.onopen2: this.API_driver_id=${this.API_driver_id}`)
    };

    this.ws.onmessage = (e) => {
      //console.log("Message received: " + e.data);
      this.API_on_message(JSON.parse(e.data));
    };

    this.ws.onclose = function(e) {
      console.log('Socket is closed. Reconnect will be attempted in 1 second.', e.reason);
      setTimeout(function() {
        if (this.ws === undefined || this.ws.connect === undefined)
          this.ws = new WebSocket('ws://185.241.194.113:80');
        else
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
  } // constructor(props)

  componentDidMount() {}

// Determine the distance to the next turn and the kind of a turn
// current_point  - current point at the route
// route - the route
// returns {
//    next_turn_coordinate : where to show the buble,
//    next_turn_description_line1 : the text for the turn description line1,
//    next_turn_description_line2 : the text for the turn description line2
//  }
decorateRoute(current_point, route)
{
  var distance_to_turn = route_cache.getDistancePointToPoint(current_point, route[0]);

  var n = 0;
  var /*prev_r, */prev_good_r;
  var is_found = false;
  for (let point of route)
  {
    if (n >= 1)
    {
      distance_to_turn += route_cache.getDistancePointToPoint(route[n], route[n-1]);

      //var cur_r = 0;//route_cache.getRotation(route[n-1], route[n]);
      var cur_good_r = route_cache.getGoodRotation(route[n-1], route[n]);

      if (n >= 2)
      {
        //var r = Math.abs(prev_r - cur_r)  
        // If the angle is sharp enough then show the marker here
        //if (r > 30 && r < 150)

        // The angle between two adjacent segments of the route
        var angle_diff = prev_good_r - cur_good_r;

        // The "fastest" angle between two adjacent segments of the route
        // Meaning: angle 372 degree in long and the same 8 is shorter :-)
        //  or -178 is shorter than 182. Shorter means shorter to spin
        var shortest_angle_diff;
        if (angle_diff < -180)
          shortest_angle_diff = angle_diff + 360;
        else
        if (angle_diff > 180)
          shortest_angle_diff = angle_diff - 360;
        else
          shortest_angle_diff = angle_diff;

        //shortest_angle_diff)
        {
           
           // "R=" + (prev_r - cur_r), " Good R=" + (prev_good_r - cur_good_r));

          var sub_description
          var abs_diff = Math.abs(shortest_angle_diff)
          if (abs_diff < 10)
            sub_description = "возьмите " // левее, правее
          else
          if (abs_diff < 20)
            sub_description = "держитесь " // левее, правее
          else
          if (abs_diff < 30)
            sub_description = "возьмите слегка " // налево, направо
          else
          if (abs_diff < 45)
            sub_description = "небольшой поврот " // налево, направо
          else
          if (abs_diff < 110)
            sub_description = "поверните " // налево, направо
          else
          if (abs_diff < 130)
            sub_description = "крутой поворот " // налево, направо
          else
            sub_description = "очень крутой поворот " // налево, направо

          var left_right;
          if (abs_diff < 20)
            left_right = shortest_angle_diff < 0 ? "правее" : "левее";
          else
            left_right = shortest_angle_diff < 0 ? "направо" : "налево";

          /*var description = "пусто";
          if (prev_r - cur_r < 0)
          {
            if (prev_r - cur_r > -25)
              description = "возьмите правее"
            else
            if (prev_r - cur_r < -100)
              description = "резкий поворот направо"
            else
              description = "поверните направо"
          }
          else
          {
            if (prev_r - cur_r < 25)
              description = "возьмите левее"
            else
            if (prev_r - cur_r > 100)
              description = "резкий поворот налево"
            else
              description = "поверните налево"
          }*/

          var description_line_1 = `Через ${Math.round(distance_to_turn)} метров`;
          var description_line_2 =  sub_description + left_right;//`${description}`;

          //console.log(`WILL SHOW A CALLOUT(${(prev_r - cur_r)}): at ${JSON.stringify(route[n-1])}, distance_to_turn=${distance_to_turn}, description_line_1=${description_line_1}, description_line_2=${description_line_2}`);

          // Don't show a turn buble if it is too close to the driver - it messes things up :-)

          var arrow = "arrow-";
          if (abs_diff < 45)
            arrow +="up-";
          else
          if (abs_diff < 110)
            arrow += "";
          else
            arrow += "down-";
          
          if (shortest_angle_diff < 0)
            arrow += "right";
          else
            arrow += "left";

         // console.log(`ANGLE_DIFF=${angle_diff}, SHORTEST_ANGLE_DIFF=${shortest_angle_diff}, prev_r=${prev_r}, cur_r=${cur_r}, R=${prev_r - cur_r}, arrow=${arrow},distance_to_turn=${distance_to_turn},
           // prev_good_r=${prev_good_r}, cur_good_r=${cur_good_r}, Good R=${prev_good_r - cur_good_r}`);
 
          return{
            next_turn_coordinate : (distance_to_turn < 50) ? {} : route[n-1],
            next_turn_description_line1: description_line_1,
            next_turn_description_line2: description_line_2,
            next_turn_direction : (distance_to_turn < 50) ? "arrow-up" :
              arrow
              //(/*prev_r - cur_r < 0*/) ? "arrow-right" : "arrow-left")
            };
        }
      }
     // prev_r = cur_r;
      prev_good_r = cur_good_r;
    }
    ++n;
  }

  // No turns - no buble, but still there are line1 and line2
  return {
    next_turn_coordinate : {},
    next_turn_description_line1: "Двигайтесь прямо",
    next_turn_description_line2: `${distance_to_turn} метров`,
    next_turn_direction : ""           
  };
}


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

// Builds actual directions to A and A to B without decoration
buildDirections_best_order()
{
  // console.log("buildDirections_best_order");

  // Build the route from driver's position to A
  route_cache.get_route(
      {latitude: this.driver.lat, longitude: this.driver.lon},
      {latitude: this.best_order.latA, longitude: this.best_order.lonA},
      function(route_object)
      {
        // The route is ready! Set the state
        this.setState({
            coords1: route_object.route,
            distance1 : route_object.distance,
            duration1 : route_object.duration,

            // Note: we don't need navi info here - so leave it blank
            next_turn_direction : "",
            next_turn_coordinate : {},
            next_turn_description_line1 : "",
            next_turn_description_line2 : ""})
      }.bind(this)
    )
  
  // Build the route from A to B
  route_cache.get_route(
    {latitude: this.best_order.latA, longitude: this.best_order.lonA},
    {latitude: this.best_order.latB, longitude: this.best_order.lonB},
    function(route_object)
    {
        this.setState({
            coords2: route_object.route,
            distance2 : route_object.distance,
            duration2 : route_object.duration,

            // Note: we don't need navi info here - so leave it blank
            next_turn_direction : "",
            next_turn_coordinate : {},
            next_turn_description_line1 : "",
            next_turn_description_line2 : ""})
    }.bind(this)
  )
} // buildDirections_best_order

// Builds actual current directions either to A or from A to B and decorate them to show in Navigator
buildDirections_navigator(is_route_to_B)
{
  // Navigator is for the driver ...
  var from_coordinate = {latitude: this.driver.lat, longitude: this.driver.lon};

  // ... going either to A or to B
  var to_coordinate = is_route_to_B ?
    {latitude: this.best_order.latB, longitude: this.best_order.lonB} :
    {latitude: this.best_order.latA, longitude: this.best_order.lonA};

  // Build the route
  route_cache.get_route(from_coordinate, to_coordinate,
    function(route_object)
    {

      //console.log(`buildDirections_navigator(${is_route_to_B}): inside function(${JSON.stringify(route_object)})`);
      
      // We got the route! Decorate it with the turn info
      var decoration = this.decorateRoute(from_coordinate, route_object.route);

      // And set the state
      this.setState({

            // We have two difference set of states for two different kind of routes
            // Probably we need a single one if we never want to show them in a single navigator
            
            // Hack! We can't do an empty list of coordinates - it just does not change anything
            // in the polyline!!! Polyline needs t least TWO POINTS
            coords1 : (is_route_to_B ? [to_coordinate, to_coordinate] : route_object.route),
            distance1 : (is_route_to_B ? 0 : route_object.distance),
            duration1 : (is_route_to_B ? 0 : route_object.duration),

            coords2 : (is_route_to_B ? route_object.route : [to_coordinate, to_coordinate]),

            // Set both to 1 rather than to 0 to avoid race condition which results
            // in got B message at A. Dirty hack
            distance2 : (is_route_to_B ? route_object.distance : 1),
            duration2: (is_route_to_B ? route_object.duration : 1),
           
            next_turn_direction : decoration.next_turn_direction,
            next_turn_coordinate : decoration.next_turn_coordinate,
            next_turn_description_line1 : decoration.next_turn_description_line1,
            next_turn_description_line2 : decoration.next_turn_description_line2

      })

      //console.log(`states inside function: coords1=(${JSON.stringify(this.state.coords1)}), coords2=(${JSON.stringify(this.state.coords2)})`);
    }.bind(this)
  )


 // console.log(`In buildDirections_navigator: coords1=(${JSON.stringify(this.state.coords1)}), coords2=(${JSON.stringify(this.state.coords2)})`);
} // buildDirections_navigator

// Reaction on incoming message from the server
API_on_message(e)
{  
 // console.log(`API_on_message begins: e:${JSON.stringify(e)}`)

  // If the driver is not created yet then create them
  if (this.API_driver_id === undefined)
        this.API_registerDriver();

  // Store the prev driver info - for her/his coordinate
  if (this.driver !== undefined)
  {
    this.prev_one_driver = this.driver;
    //console.log(`this.prev_driver_info.length=${this.prev_driver_info.length}`)
  }

//  console.log(`API_on_message, this.API_driver_id: ${this.API_driver_id}, e:${JSON.stringify(e)}`)

  // Get current driver (it may not exist yet because the registraion above could be not finished yet)
  this.driver = this._API_helper_get_driver_by_id(e, this.API_driver_id);
  if (this.driver.driver_id != this.API_driver_id)
    return;

  //console.log(`API_on_message, driver: ${JSON.stringify(this.driver)}`)

  if (this.already_called_map_animate === undefined)
  {
    this.already_called_map_animate = 1;
    console.log(`Call this.map.setCamera`);
    this.map.setCamera({heading : 0,
              center: {latitude: this.driver.lat,longitude: this.driver.lon},
              pitch: 45, altitude: 300, zoom: 17}, {duration: 1100});
  }

  // If this driver is assigned then save his/her assigned order id
  this.assigned_order_id = this.driver.assigned_order_id;


  //console.log(`API_on_message3: driver=${JSON.stringify(driver)}`)

  // Get the best order
  // Note: if the driver has an assigned order then it will be in this.best_order 
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

    var distance = route_cache.getDistancePointToPoint(
          {latitude: this.prev_one_driver.lat, longitude: this.prev_one_driver.lon},
          {latitude: this.driver.lat,longitude: this.driver.lon}
        );

    // If the distance has not changed much (less than 1 meter) then do nothing
    if (distance < 1)
      return;

    //console.log(`distance=${distance}`)
    var rotation = route_cache.getGoodRotation(
                {latitude: this.prev_one_driver.lat,
                longitude: this.prev_one_driver.lon},

                {latitude: this.driver.lat,
                longitude: this.driver.lon}) + 90;

    if (rotation != 0)
      this.setState({rotation: rotation});

    this.setState({
      driver_region : {latitude: this.driver.lat, longitude: this.driver.lon, latitudeDelta :     this.state.driver_region.latitudeDelta, longitudeDelta: this.state.driver_region.longitudeDelta},

    })

    // Build all needed directions for the navigator (because it's an accepted order)
    this.buildDirections_navigator(
      (this.state.assigned_order_status == "ride_begins" ||
      this.state.assigned_order_status == "payment") ? true : false
      );

    if (this.map !== undefined && this.map !== null)
    {
      if (rotation != 0)
      {
        // If the distance from the driver and the current map is too long then
        // don't animate - just set

          /*if (distance > 1000)
          {
            console.log(`Call this.map.setCamera, distance=${distance}`);
            this.map.setCamera({heading :
              rotation,
              center: {latitude: this.driver.lat,longitude: this.driver.lon},
              pitch: 45, altitude: 300, zoom: 17}, {duration: 1100});
          }
          else*/
          {
            if (this != null && this.driver !== {} && this.driver !== undefined && this.driver.lat !== undefined && this.driver.lon !== undefined && rotation !== undefined)
            {

              // Position the camera 130 meters forward
              // Why? To have more space on screen to view where to go to :-)
              var camera_coordinate = route_cache.getPointTowardsTo(
                {latitude: this.driver.lat, longitude: this.driver.lon},
                {latitude: this.prev_one_driver.lat, longitude: this.prev_one_driver.lon},
                -130
                )

              if (this.nextTimeAllowAnimate === undefined || Date.now() >= this.nextTimeAllowAnimate)
                this.map.animateCamera({heading :
                  rotation,
                  center: camera_coordinate/*{latitude: this.driver.lat,longitude: this.driver.lon}*/,
                  pitch: 45, altitude: 300, zoom: 17}, {duration: 1100});
            }
          }
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

  // Build all needed directions
  // Note: just a map, no navigation
  if (this.state.page == "best_order")
    this.buildDirections_best_order()

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

// Sends a message from the driver that the ride has begun
API_ride_begins()
{
  this.ws.send(`{"event":"test_start_ride","data":{"driver_id":${this.API_driver_id}}}`);

  // Save ride begins timestamp as a starting time to go to B
  this.ride_begins_ts = Date.now();

  this.setState({assigned_order_status: "ride_begins"})
}

// Sends a message from the driver that the ride has finished
API_ride_finishes()
{
  this.ws.send(`{"event":"test_finish_ride","data":{"driver_id":${this.API_driver_id}}}`);

  this.setState({best_order_id : -1, assigned_order_status: "", page: "best_order"})
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
    //console.log(`API_accept_best_order:  this.state.best_order_id=${this.state.best_order_id}, this.API_driver_id=${this.API_driver_id}`);

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
              <MapView style={styles.mapStyle} provider={PROVIDER_GOOGLE} initialRegion={{
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
          showsBuildings
          ref={ref => { this.map = ref }}

          //region={this.state.driver_region} If this property is set the best order map sticks with the driver  - that's why commented out

          >

<MapView.Marker coordinate={this.state.driver_region}/>
<MapView.Marker pinColor='green' coordinate={this.state.best_order_A}/>
<MapView.Marker pinColor='black' coordinate={this.state.best_order_B}/>

<MapView.Polyline lineDashPattern={[25,25]}
coordinates={this.state.coords1}
	strokeColor='green'
	strokeWidth={10}/>

  <MapView.Polyline lineDashPattern={[25,25]}
coordinates={this.state.coords2}
	strokeColor='black'
	strokeWidth={10}/>

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

 // console.log(`In render(this.state.page=${this.state.page}) : coords1=(${JSON.stringify(this.state.coords1)}), coords2=(${JSON.stringify(this.state.coords2)})`);

      // Get miutes and seconds to go
      var minutes;
      var seconds;
      
      if (this.state.assigned_order_status == "ride_begins" ||
          this.state.assigned_order_status == "payment")
        seconds = Math.trunc(this.state.duration2)
      else
        seconds = Math.trunc(this.state.duration1)

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
      var distance_to_go = (((
        this.state.assigned_order_status == "ride_begins" ||
        this.state.assigned_order_status == "payment"
      ) ?
        this.state.distance2 : this.state.distance1)/1000).toFixed(1);


      var render_duration_and_distance;

      // If we have not got assigned order id yet then don't show anythins on the
      // assigned_order page because otherwise it would be like we got A which is not true
      if (this.assigned_order_id === undefined || this.assigned_order_id == -1)
        render_duration_and_distance = (<View></View>)
      else
      // Arrived either to A or to B - change text in the bottom
      if (!minutes && !seconds || !distance_to_go)
      {
        if (this.state.assigned_order_status == "ride_begins")
          render_duration_and_distance = (
          <View style={{flex: 1, position: 'absolute', bottom: 10, height: 40, flexDirection: 'row'}}>
            <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: '15'}}>приехали по адресу</Text>
          </View>
          )
        else
          render_duration_and_distance = (
          <View style={{flex: 1, position: 'absolute', bottom: 10, height: 40, flexDirection: 'row'}}>
            <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: '15'}}>приехали на точку подачи</Text>
          </View>
          )
      }
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

      /*
      *   Render various assigned order related buttons depending on its status
      */

      // Got B and wait for payment
      if (this.state.assigned_order_status == "payment")
      {
            render_assigned_order_info = (<TouchableOpacity style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 70, alignItems: "center", backgroundColor: "rgba(100,255,100,100)", padding: 10, borderColor: 'black', borderWidth: 1}} onPress={() => {this.API_ride_finishes()}}>
            <Text style={{fontFamaly: 'roboto', textTransform: 'uppercase', fontStyle: 'bold', fontSize: '15'}}> Закрываем заказ </Text>
            </TouchableOpacity>)
      }
      else
      // Ride in progress
      if (this.state.assigned_order_status == "ride_begins")
      {
        // If the driver is close to B then show her/him the in-place button
        // Note: we calc distance to B by bird flight just in case if something goes
        //  wrong with the route - for driver to close the order
        //if (this.state.distance2 < 100)
        if (route_cache.getDistancePointToPoint(
          {latitude: this.driver.lat, longitude: this.driver.lon},
          {latitude: this.best_order.latB, longitude: this.best_order.lonB}
        ) < 200)
          render_assigned_order_info = (<TouchableOpacity style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 70, alignItems: "center", backgroundColor: "rgba(100,255,100,100)", padding: 10, borderColor: 'black', borderWidth: 1}} onPress={() => { return this.setState({assigned_order_status: "payment"})}}>
            <Text style={{fontFamaly: 'roboto', textTransform: 'uppercase', fontStyle: 'bold', fontSize: '15'}}> Почти приехали - можно просить оплату </Text>
            </TouchableOpacity>)
        else
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
          // Note: we calc distance to A by bird flight just in case if something goes
          //  wrong with the route - for driver to close the order
          if (route_cache.getDistancePointToPoint(
            {latitude: this.driver.lat, longitude: this.driver.lon},
            {latitude: this.best_order.latA, longitude: this.best_order.lonA}
          ) < 200)
          {
          //if (this.state.distance1 < 200)
            render_assigned_order_info = (<TouchableOpacity style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 100, alignItems: "center", backgroundColor: "rgba(100,255,100,100)", padding: 10, borderColor: 'black', borderWidth: 1}} onPress={() => { return this.setState({assigned_order_status: "at_A"})}}>
            <Text style={{fontFamaly: 'roboto', textTransform: 'uppercase', fontStyle: 'bold', fontSize: '25'}}> На точке подачи </Text>
            </TouchableOpacity>)
          }
          else
            render_assigned_order_info = (<View></View>)
      }

             //   <Text style={{color: 'white', fontFamily: 'roboto', fontWeight:'bold', fontSize: '20'}}>{this.state.next_turn_description_line1}</Text>
               // <Text style={{color: 'white', fontFamily: 'roboto', fontWeight:'bold', fontSize: '20'}}>{this.state.next_turn_description_line2}</Text>

/* 

            <MapView.Marker coordinate={this.state.driver_region}
            ref={ref => { this.marker = ref}}
            
            >
              <View></View>
              <MapView.Callout><Text>Turn left</Text></MapView.Callout>


              onRegionChangeComplete={() => this.marker.showCallout()}
*/
      // Here should be a navigtor to the to pickup point and to the drop off point
      return (
          <View style={styles.container}>
            <MapView provider={PROVIDER_GOOGLE} style={styles.mapStyle}

              initialRegion={{
                latitude: 61.666594,
                longitude: 50.827201,
                latitudeDelta: 0.1,
                longitudeDelta: 0.1
              }}

           //   region={{
             //   latitude: 61.666594,
            //    longitude: 50.827201,
            //    latitudeDelta: 0.1,
             //   longitudeDelta: 0.1
            //  }}
             // region={this.state.driver_region}
             // initialRegion={this.state.driver_region}

              onRegionChange={() => {this.nextTimeAllowAnimate = Date.now() + 1000}}

        //      initialCamera={{
          //        heading : 0,
          //        center: {latitude: 61.666594,longitude: 50.827201},
          //        pitch: 45, altitude: 300, zoom: 17
            //  }}

             //onRegionChangeComplete={(region) => {this.HandleRegionChangeInAcceptedOrder(region)}}

              //region={this.state.driver_region}

                showsBuildings={true}
                showsMyLocationButton={true}
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
              <View></View>
              <MapView.Callout>
                <View/* style={{flex: 1, flexDirection: "column", backgroundColor: '#4ba9c8'}}*/>

                <Feather name={this.state.next_turn_direction} size={24} color="black"/>


                </View>
              </MapView.Callout>
            </MapView.Marker>

            <MapView.Polyline
              lineJoin="miter"
              coordinates={this.state.coords1}
              lineDashPattern={[25,1]}
	            strokeColor='rgba(0, 255, 0, 0.20)'
	            strokeWidth={20}/>

            <MapView.Polyline
              lineJoin="bevel"
              coordinates={this.state.coords2}
              lineDashPattern={[25,1]}
	            strokeColor='rgba(0, 0, 0, 0.50)'
	            strokeWidth={20}/>
          </MapView>


          <View style={{position: 'absolute', left : 10, right : 10, top: 30, alignItems: "center", padding: 10, borderColor: 'black', borderWidth: 1, flex: 1, flexDirection: "column", backgroundColor: '#4ba9c8'}}>
            <Text style={{color: 'white', fontFamily: 'roboto', fontWeight:'bold', fontSize: '20'}}>{this.state.next_turn_description_line1}</Text>
            <Text style={{color: 'white', fontFamily: 'roboto', fontWeight:'bold', fontSize: '20'}}>{this.state.next_turn_description_line2}</Text>
          </View>

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

