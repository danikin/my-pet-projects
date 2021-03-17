import React from 'react';
import MapView, { PROVIDER_GOOGLE, PROVIDER_APPLE } from 'react-native-maps';
import Marker from 'react-native-maps';
import { Alert, StyleSheet, Dimensions, FlatList, Text, View, ScrollView, TextInput, Image, ImageBackground, Icon } from 'react-native';

import {Call} from 'react-native'
import * as Linking from 'expo-linking';
import { Table, Row } from 'react-native-table-component';

//import { useState, useEffect } from 'react';
import {TouchableOpacity } from 'react-native';
import { Camera } from 'expo-camera';

import { RNCamera, FaceDetector } from 'react-native-camera';

import MapViewDirections from 'react-native-maps';

import { AntDesign } from '@expo/vector-icons';
import { Feather } from '@expo/vector-icons'; 
import { MaterialIcons } from '@expo/vector-icons';
import { Ionicons } from '@expo/vector-icons';
import { MaterialCommunityIcons } from '@expo/vector-icons';
import { FontAwesome5 } from '@expo/vector-icons';
import { FontAwesome } from '@expo/vector-icons'; 
import { Foundation } from '@expo/vector-icons';
import { Fontisto } from '@expo/vector-icons';
import { Entypo } from '@expo/vector-icons';

import * as route_cache from './components/route_cache.js'
import * as server_api_sdk from './components/server_api_sdk.js'

import * as h3core from './components/uber-h3-js/lib/h3core.js';

// TODO: take it from the server!
const TOLL_FREE_DURATION = 180;

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
        page :  "splash",
        next_turn_direction: "",

        camera_type: Camera.Constants.Type.back,
        driver_region : {latitude : 61.666594 + (Math.random()-.5)*0.05, longitude : 50.827201 + (Math.random()-.5)*0.05, latitudeDelta: 0.03, longitudeDelta: 0.03},


      coords1 : []/*[{"latitude":61.65058,"longitude":50.8583},{"latitude":61.65096,"longitude":50.8587},{"latitude":61.65075,"longitude":50.85932},{"latitude":61.64985,"longitude":50.85991},{"latitude":61.65303,"longitude":50.88211},{"latitude":61.65723,"longitude":50.87968},{"latitude":61.65822,"longitude":50.87587},{"latitude":61.66329,"longitude":50.86906},{"latitude":61.66579,"longitude":50.8581},{"latitude":61.66576,"longitude":50.85613},{"latitude":61.66614,"longitude":50.85199},{"latitude":61.66803,"longitude":50.84614},{"latitude":61.66386,"longitude":50.83855},{"latitude":61.66546,"longitude":50.8346},{"latitude":61.66768,"longitude":50.83207},{"latitude":61.66518,"longitude":50.82155},{"latitude":61.66686,"longitude":50.81979},{"latitude":61.66778,"longitude":50.82372},{"latitude":61.66682,"longitude":50.82479},{"latitude":61.66686,"longitude":50.825}]*/,


      coords2 : []/*[{"latitude":61.66686,"longitude":50.825},{"latitude":61.66778,"longitude":50.82372},{"latitude":61.66633,"longitude":50.81757},{"latitude":61.66465,"longitude":50.81933},{"latitude":61.66398,"longitude":50.81653},{"latitude":61.66417,"longitude":50.81567},{"latitude":61.66385,"longitude":50.81514},{"latitude":61.66352,"longitude":50.81618},{"latitude":61.66114,"longitude":50.81868},{"latitude":61.66018,"longitude":50.81825},{"latitude":61.65675,"longitude":50.8058},{"latitude":61.65523,"longitude":50.80813},{"latitude":61.64986,"longitude":50.81914},{"latitude":61.64161,"longitude":50.80904},{"latitude":61.64165,"longitude":50.80977}]*/,

      pic_status: ["plussquareo", "plussquareo","plussquareo","plussquareo","plussquareo","plussquareo","plussquareo","plussquareo"]



    }

    this.directions_ts = Date.now() - 10000;

    Camera.requestPermissionsAsync();

    this.cameraRef = React.createRef();

  } // constructor(props)






  componentDidMount() {

    console.log(`componentDidMount26`)

    this.on_message_received = this.on_message_received.bind(this);

    server_api_sdk.api_driver_set_position(this.state.driver_region, this.on_message_received);
  }

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
            sub_description = "резкий поворот " // налево, направо

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
            arrow +="top-";
          else
          if (abs_diff < 110)
            arrow += "";
          else
            arrow += "bottom-";
          
          if (shortest_angle_diff < 0)
            arrow += "right";
          else
            arrow += "left";

         // console.log(`ANGLE_DIFF=${angle_diff}, SHORTEST_ANGLE_DIFF=${shortest_angle_diff}, prev_r=${prev_r}, cur_r=${cur_r}, R=${prev_r - cur_r}, arrow=${arrow},distance_to_turn=${distance_to_turn},
           // prev_good_r=${prev_good_r}, cur_good_r=${cur_good_r}, Good R=${prev_good_r - cur_good_r}`);
 
          //console.log(`distance_to_turn: ${distance_to_turn}, route[n-1]=${JSON.stringify(route[n-1])}, arrow=${arrow}`);

          return{
            next_turn_coordinate : (distance_to_turn < 50) ? {} : route[n-1],
            next_turn_description_line1: description_line_1,
            next_turn_description_line2: description_line_2,
            next_turn_direction : (distance_to_turn < 50) ? "arrow-up" : arrow
              //(/*prev_r - cur_r < 0*/) ? "arrow-right" : "arrow-left")
            };
        }
      } // if (n >= 2)

      // It MUST be here before the previous if otherwise the distance to turn will
      // take into account the section right after the turn
      distance_to_turn += route_cache.getDistancePointToPoint(route[n], route[n-1]);

     // prev_r = cur_r;
      prev_good_r = cur_good_r;
    } // if (n >= 1)
    ++n;
  }

  // No turns - no buble, but still there are line1 and line2
  return {
    next_turn_coordinate : {},
    next_turn_description_line1: Math.round(distance_to_turn) ? "Двигайтесь прямо" : "Прихали",
    next_turn_description_line2: Math.round(distance_to_turn) ? `${Math.round(distance_to_turn)} метров` : "",
    next_turn_direction : "arrow-up"           
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
          <Text style={{ flex: 0.7, color: 'white', fontSize: 25, fontStyle: 'bold' }}>
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

// Builds actual directions to A and A to B without decoration
buildDirections_best_order(driver, order)
{
  // console.log("buildDirections_best_order");

  // Build the route from driver's position to A
  route_cache.get_route(
      {latitude: driver.lat, longitude: driver.lon},
      {latitude: order.latA, longitude: order.lonA},
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
            next_turn_description_line1 : "Выезжаем на подачу",
            next_turn_description_line2 : ""})
      }.bind(this)
    )
  
  // Build the route from A to B
  route_cache.get_route(
    {latitude: order.latA, longitude: order.lonA},
    {latitude: order.latB, longitude: order.lonB},
    function(route_object)
    {
        this.setState({
            coords2: route_object.route,
            distance2 : route_object.distance,
            duration2 : route_object.duration})
    }.bind(this)
  )
} // buildDirections_best_order

// Builds actual current directions either to A or from A to B and decorate them to show in Navigator
buildDirections_navigator(is_route_to_B, driver, order)
{
  // Navigator is for the driver ...
  var from_coordinate = {latitude: driver.lat, longitude: driver.lon};

  // ... going either to A or to B
  var to_coordinate = is_route_to_B ?
    {latitude: order.latB, longitude: order.lonB} :
    {latitude: order.latA, longitude: order.lonA};

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

prev_driver_coordinate_array = [];

on_message_received(driver, order)
{
  // This is to save the previous driver coordinate which is used for
  // determining the rotation
  this.prev_driver_coordinate_array.push({latitude: driver.lat, longitude: driver.lon});
  let driver_previous_coordinate;
  if (this.prev_driver_coordinate_array.length == 2)
    driver_previous_coordinate = this.prev_driver_coordinate_array.shift();

  //console.log(`on_message_received: driver=${JSON.stringify(driver)}, order=${JSON.stringify(order)}`);

  //  1.  Update driver's coordinate for simuation of motion between updates
  //  Note: we use coortinates from the server because for test reasons motion is
  //    simulated by a server. In a future the coordinate will be taken from GPS
  //    and sent to the server while update_object_coordinate will be called
  //    whenever we get the coordinate from GPS
  route_cache.update_object_coordinate(
    driver.driver_id,
    [{latitude: driver.lat, longitude: driver.lon}],
    5000,
    function(coord_array)
    {
        //console.log(`callback(${JSON.stringify(coord_array)}`)
        let new_marker_coordinate = coord_array[0].predicted_cooridinate;
        this.setState({
            driver_region : {
            latitude: new_marker_coordinate.latitude,
            longitude: new_marker_coordinate.longitude,
            latitudeDelta: this.state.driver_region.latitudeDelta,
            longitudeDelta: this.state.driver_region.longitudeDelta
  }})}.bind(this),
    200)

  // 2. Set camera on the driver for the first time
  if (this.already_called_map_animate === undefined && this.map !== undefined &&
        this.map.setCamera !== undefined)
  {
    this.already_called_map_animate = 1;
    this.map.setCamera({heading : 0,
      center: {latitude: driver.lat,longitude: driver.lon},
      pitch: 45, altitude: 300, zoom: 17}, {duration: 1100});
  }

  // 3. If the app is still on splash or best_order, but the driver is assigned then
  // this eaither means autoaccept or it means that the app is reloaded, but
  // the order is already accepted - set the state to assigned order
  if (order !== undefined && order.status == "assigned_order" &&
    (this.state.page == "best_order" || this.state.page == "splash"))
  {
    this.setState({page: "assigned_order", next_turn_description_line1: "Выезжаем на подачу", next_turn_description_line2 : ""});
    console.log(`on_message_received: AFTER setState({page: "assigned_order"}): this.state.page == ${this.state.page}`)
  }

  // 4. If the driver has been at A then form waiting seconds/minutes and toll wait flag
  // Note: this data is useful even when a driver leaves A and goes to B
  if (order !== undefined && order.ts_driver_at_A !== undefined &&
      order.ts_driver_at_A != -1)
  {
    // We implement this via a timeout because we need to constantly update
    // seconds without waiting a next message from the server
    var call_me = function() {
      let waiting_seconds = -1;
      let is_toll_waiting = 0;

      if (order.ts_ride_started === undefined || order.ts_ride_started == -1 || order.ts_ride_started == 0)
        waiting_seconds = Date.now()/1000 - order.ts_driver_at_A;
      else
        waiting_seconds = order.ts_ride_started - order.ts_driver_at_A;
    
      is_toll_waiting = (waiting_seconds > TOLL_FREE_DURATION) ? 1 : 0;

      let wait_time_so_far = Math.trunc(Date.now()/1000) - order.ts_driver_at_A;
      this.setState(
      {
          at_A_waiting_seconds : wait_time_so_far,
          at_A_waiting_toll_seconds: wait_time_so_far - TOLL_FREE_DURATION,
          at_A_is_toll_waiting : is_toll_waiting,
          price_for_toll_wait: order.price_for_toll_wait,
          wait_time_minutes : Math.trunc(wait_time_so_far/60),
          wait_time_seconds : wait_time_so_far%60,
          wait_time_toll_minutes : Math.trunc((wait_time_so_far - TOLL_FREE_DURATION)/60),
          wait_time_toll_seconds : (wait_time_so_far - TOLL_FREE_DURATION)%60
      })
    }.bind(this);

    // Clear the previous interval! Otherwise this function will be called for
    // every message recevied which will result in a) jumping price_for_toll_wait,
    // b) multitude of intervals and using of resourses at the mobile device
    if (this.interval_for_toll_wait_timer !== undefined)
      clearInterval(this.interval_for_toll_wait_timer);

    // Call it first time
    call_me();
    // Call it in 5 seconds
    this.interval_for_toll_wait_timer = setInterval(function() {
        call_me();
        // Clear the interval if we're not waiting for a rider at A anymore
        if (this.state.assigned_order_status != "at_A")
          clearInterval(this.interval_for_toll_wait_timer);
    }.bind(this), 5000)

  } // if driver has been at A

  // 5. If the driver has an assigned order then animate camera and build the route
  //    for the navigator
  if (order !== undefined && order.status == "assigned_order")
  {
    let distance = (driver_previous_coordinate === undefined) ? 0 : route_cache.getDistancePointToPoint(
        driver_previous_coordinate,
          {latitude: driver.lat, longitude: driver.lon}
    );

    //console.log(`on_message_received: order.status == "assigned_order"`);

    // Check if a driver close to A or B
    // Note: in a future when GSP is here we need to update this state as
    //  soon as we receive the new GSP coordinate
    this.setState({close_to_A: route_cache.getDistancePointToPoint(
          {latitude: driver.lat, longitude: driver.lon},
          {latitude: order.latA, longitude: order.lonA}
        ) < 200});
    this.setState({close_to_B: route_cache.getDistancePointToPoint(
          {latitude: driver.lat, longitude: driver.lon},
          {latitude: order.latB, longitude: order.lonB}
        ) < 200});
    
    // Without this "if" camera keeps moving around
    // That's probably because of too frequent change of coordinates of a car
    // inside update_object_coordinate simulator
    if (distance >= 1)
    {

    let rotation = route_cache.getGoodRotation(
        driver_previous_coordinate, {latitude: driver.lat, longitude: driver.lon}) + 90;

    // Set driver's coordinate on map
    this.setState({
      driver_region : {latitude: driver.lat, longitude: driver.lon, latitudeDelta :     this.state.driver_region.latitudeDelta, longitudeDelta: this.state.driver_region.longitudeDelta},

      best_order: order

    })

    // Build all needed directions for the navigator (because it's an accepted order)
    this.buildDirections_navigator(
        (this.state.assigned_order_status == "ride_begins" ||
        this.state.assigned_order_status == "payment") ? true : false,
        driver,
        order
      );

    if (this.map !== undefined && this.map !== null && rotation != 0)
    {
      // Position the camera 130 meters forward
      // Why? To have more space on screen to view where to go to :-)
      var camera_coordinate = route_cache.getPointTowardsTo(
          {latitude: driver.lat, longitude: driver.lon},
          driver_previous_coordinate,
          -130
      )

      if (this.first_time_interval_for_camera_set === undefined)
      {
          // TODO!!!! Remove the interval when driver stops moving and return it back when
          //  the motion starts!!!

          this.first_time_interval_for_camera_set = 1;
                
          console.log(`on_message_received(${driver}, ${order}): setInterval2 for driver coordinate update`)

      }

      if (this.nextTimeAllowAnimate === undefined || Date.now() >= this.nextTimeAllowAnimate)
      {
          this.map.animateCamera({heading :
                  rotation,
                  center: camera_coordinate,
                  pitch: 45, altitude: 300, zoom: 17}, {duration: 1100});
      }  // if (this.nextTimeAllowAnimate === undefined || Date.now() >= this.nextTimeAllowAnimate)
    } // if (this.map !== undefined && this.map !== null && rotation != 0)
    
    } // if (distance >= 1)

  } // if (order !== undefined && order.status == "assigned_order")

  // 6. If there is an unassigned order then update the state for that
  if (order !== undefined && order.status != "assigned_order")
  {
    this.setState({
        best_order_price: order.price,
        best_order_id: order.unassigned_order_id,
        best_order_A : {latitude: order.latA, longitude: order.lonA},
        best_order_B : {latitude: order.latB, longitude: order.lonB},

        best_order: order,

        driver_region : {latitude: driver.lat,
        longitude: driver.lon,
        latitudeDelta :   this.state.driver_region.latitudeDelta,
        longitudeDelta: this.state.driver_region.longitudeDelta}
    })

    // Build all needed directions for a map if there is no assigned order
    this.buildDirections_best_order(driver, order)

    // Don't call the address resolving to frequently
    var now = Date.now();
    if (!(this.directions_ts === undefined) && now - this.directions_ts < 1000)
    {
      return
    }

    this.directions_ts = now

    // Resolve best_order's lon/lon to addresses
    var url_A = `http://185.241.194.113:1234/geo/get_houses_by_location?lat=${order.latA}&lon=${order.lonA}&resolution=11&ring_size=1`;

    fetch(url_A)
          .then(response => response.json())
          .then(responseJson => {
            this.setState({addr_A : responseJson.data.nearest_house})
          });

    var url_B = `http://185.241.194.113:1234/geo/get_houses_by_location?lat=${order.latB}&lon=${order.lonB}&resolution=11&ring_size=1`;

    fetch(url_B)
          .then(response => response.json())
          .then(responseJson => {
            this.setState({addr_B : responseJson.data.nearest_house})
          });
  } // if (order !== undefined && order.status != "assigned_order"

} // on_message_received(driver, order)


render()
{


    /*
    *   Onboarding + splash part
    */

    if (this.state.page.startsWith("onboarding") || this.state.page == "splash")
    {
      // Common bottom part of onboarding
      let render_onboarding_bottom = (
          <View style={{flex:1, position: 'absolute', bottom: 10}}>
            <TouchableOpacity style={{flex :1, alignItems: "center", backgroundColor: "#d1ffd2", padding: 10, borderColor: 'black', borderWidth: 1}}
          onPress={() => this.setState({page: "best_order"})}>
              <Text style={{ fontSize: 21, textTransform: 'uppercase', fontFamily: 'roboto', fontWeight: 'bold'}}>Увидеть лучший заказ</Text>
            </TouchableOpacity>
            <View style={{flex: 1}}><Text></Text></View>
            <View style={{flex: 1, flexDirection: 'row', justifyContent: 'space-between'}}>
              <TouchableOpacity onPress={() => {this.setState({page: "onboarding_about"})}}><FontAwesome5 name="chalkboard-teacher" size={36} color="black" /></TouchableOpacity>
              <TouchableOpacity onPress={() => {this.setState({page: "onboarding_feedback"})}}><MaterialIcons name="feedback" size={36} color="black" /></TouchableOpacity>
              <TouchableOpacity onPress={() => {this.setState({page: "onboarding_QnA"})}}><Entypo name="help" size={36} color="black" /></TouchableOpacity>
              <TouchableOpacity><Feather name="settings" size={36} color="black" /></TouchableOpacity>
              <TouchableOpacity onPress={() => {this.setState({page: "onboarding_stat", stat_table_type: "ride"})}}><Ionicons name="md-stats" size={36} color="black" /></TouchableOpacity>
            </View>
          </View>
        )

      if (this.state.page == "onboarding_stat")
      {
        /*
        *   Stat descriptions
        */

        const stat_descriptions = {
          "ride_description" : "Все заказы (включая отмены)",
          "aggr_ride_description" : "Агрегация по заказам (включая отмены)",
          "fee_description" : "Комиссии (за поездки + покупка смен)",
          "aggr_fee_description" : "Агрегация комиссий",
          "payment_description" : "Все платежи",
          "withdrawal_description" : "Все выводы денег",
          "cashback_description" : "Все кэшбэки",
          "bonus_description" : "Поступления бонусов",
          "balance_description" : "Баланс на заданную дату"
        }

        /*
        *   Table header items
        */

        const stat_heads = {

        // All rides (included cancelled)
        "ride_head": [
          'Дата',
          'Выручка',  // 'self-cancelled', 'rider-cancelled' or amount
          'Км подача',
          'Мин подача',
          'Км поездка',
          'Мин поездка',
          'Выручка/час', // 0 if cancelled
          'Комиссия', // 0 if the shift was bought
        ],

        // Aggregated rides
        "aggr_ride_head" : [
          'Период',
          'Совершено',
          'Отменено мной',
          'Отменено пассажиром',
          'Выручка',
          'Км подача+поездка',
          'Мин подача+поездка',
          'Выручка/час',
          'Простой (час)',
          'Комиссия'
        ],

        // All fees
        "fee_head" : [
          'Дата',
          'Сумма',
          'Причина'
        ],

        // Aggregated fees
        "aggr_fee_head" : [
          'Период',
          'Сумма'
        ],

        // Payments
        "payment_head" : [
          'Дата',
          'Сумма',
          'Способ'
        ],

        // Withdrawals
        "withdrawal_head": [
          'Дата',
          'Сумма',
          'Способ',
          'Комиссия'
        ],

        // Cashbacks
        "cashback_head" : [
          'Дата',
          'Сумма',
          'Причина', // 10 rub/day for online app
        ],

        // Bonuses
        "bonus_head" : [
          'Дата',
          'Сумма',
          'Причина', // 4500 rub one-time bonus
        ],

        // Balance
        // Note: balance works like this:
        //  1.  Take aggregated fees up to date
        //  2.  Take aggregated cashbacks up to date
        //  3.  Take 2 minus 1
        //  4.  Take min(0, ${3}) - that's money to withdraw
        //  5.  Take aggregated bonuses up to date and add it to 3
        //  6.  Take aggregated payments up to date and add it to 5 - that's balance
        //  If the balance is negative then that's debt
        "balance_head" : [
          'Дата',
          'Долг',
          'Сумма к выводу', // It's no more than balance because a driver can only withdraw
                            //  the cashback part given that it covers all fees
          'Остаток на счету'
        ]

        };

        /*
        *   Table data rows
        */

        const stat_data = {

        "ride_data" : [
  ['10:10', '123', '1.5', '3', '5.2', '11', '550', '3.6'],
  ['12:11', '123', '1.5', '3', '5.2', '11', '550', '3.6'],
  ['12:45', '123', '1.5', '3', '5.2', '11', '550', '3.6'],
  ['13:45', '123', '1.5', '3', '5.2', '11', '550', '3.6'],
  ['14:10', '123', '1.5', '3', '5.2', '11', '550', '3.6'],
  ['16:33', '123', '1.5', '3', '5.2', '11', '550', '3.6'],
  ['17:45', '123', '1.5', '3', '5.2', '11', '550', '3.6'],
  ['18:20', '123', '1.5', '3', '5.2', '11', '550', '3.6'],
  ['19:33', '123', '1.5', '3', '5.2', '11', '554', '3.6'],
  ['21:15', '123', '1.5', '3', '5.2', '11', '612', '3.6'],
  ['21:35', '123', '1.5', '3', '5.2', '11', '811', '3.6']
        ],

        "aggr_ride_data" : [
              ['22/08/2020', '30','3','4', '4500', '35', '74', '789', '0.5', '145'],
        ],

        "fee_data" : [
              ['20/08/2020', '150', 'Смена'],
              ['21/08/2020', '150', 'Смена'],
              ['22/08/2020', '145', 'Комиссия'],
        ],

        "aggr_fee_data" : [
              ['20/07 - 26/07', '1100'],
              ['13/07 - 19/07', '1150'],
              ['20/07 - 02/08', '1055'],
        ],

        "payment_data" : [
              ['20/07 13:55', '300', 'QIWI'],
              ['31/07 21:17', '200', 'Sberbank.Online'],
              ['10/08 09:17', '600', 'Eleksnet']
        ],

        "withdrawal_data" : [
              ['22/07 18:11', '500', 'MTS', '15.55'],
              ['01/08 10:09', '400', 'Sberbank.Online', '10']
        ],

        "cashback_data" : [
              ['22/07 23:59', '10', 'Использование приложения'],
              ['23/07 23:59', '10', 'Использование приложения'],
              ['24/07 23:59', '10', 'Использование приложения'],
              ['25/07 23:59', '10', 'Использование приложения'],
              ['26/07 23:59', '10', 'Использование приложения'],
              ['27/07 23:59', '10', 'Использование приложения'],
              ['28/07 23:59', '10', 'Использование приложения']
        ],

        "bonus_data" : [
              ['22/07 17:15', '4500', 'Установка приложения'],
              ['02/08 18:09', '1500', 'Рефералка']
        ],

        "balance_data" : [
              ['22/07 11:05', '0', '150', '40', '300'],
              ['22/08 11:05', '-158', '0', '0', '0']
        ]
      }
/*
const styles = StyleSheet.create({
  stat_text: {
    textAlign: 'center'
  },
  stat_head: {
    height: 40,
    backgroundColor: 'blue'
  },
  stat_dataRow: {
    height: 30
  },
  stat_border: {
    borderWidth: 0.5,
    borderColor: 'red'
  },
  stat_table: {
    marginTop: 10,
    marginBottom: 10
  }
})*/

          return (
        <View style={styles.container}>

          <View style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 40, bottom: 150}}>

            <View style={{flex: 1.6, justifyContent: 'center'}}>
              <View style={{alignItems: 'center', justifyContent: 'center', flexDirection: 'row'}}>
                <View>
                  <Text style={{fontSize: 17, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}> INSTANT</Text>
                </View>
                <TouchableOpacity onPress={() => {this.setState({page: "best_order"})}}>
                  <MaterialCommunityIcons name="speedometer" size={36} color="black" />
                </TouchableOpacity>
                <View>
                  <Text style={{fontSize: 17, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>TAXI - статистика</Text>
                </View>
              </View>
              <View style={{alignItems: 'center', justifyContent: 'center', flexDirection: 'row'}}><Text style={{fontSize: 11, color: 'black', fontWeight: 'bold', fontFamily: 'roboto', fontStyle: 'italic'}}>driver</Text></View>
            </View>

            <View style={{flex: 10, justifyContent: 'flex-start'}}>

              <View style={{flex: 1, flexDirection: 'row', borderTopWidth: 1}}>
                <Text style={{flex: 10, fontSize: 15}}>{stat_descriptions[`${this.state.stat_table_type}_description`]}</Text>
              </View>

              <View style={{flex: 7.8, alignItems: 'stretch', justifyContent: 'flex-start'}}>
                <View style={{flex: 1}}><Text></Text></View>

                <ScrollView>
                <Table borderStyle={{borderWidth: 0.5, marginTop: 10, marginBottom: 10}}>
                  <Row data={stat_heads[this.state.stat_table_type + "_head"]} style={{ backgroundColor: '#DDDDDD'}} />
                    {(stat_data[this.state.stat_table_type + "_data"]).map((entry, index) => (
                      <Row key={index} data={entry} style={styles.stat_dataRow} textStyle={styles.stat_text}/>
                    ))}
                </Table>
                </ScrollView>
    


              </View>

              <View style={{flex: 1.5, borderBottomWidth: 0.5, flexDirection: 'row', justifyContent: 'space-between'}}>
                <TouchableOpacity onPress={()=>{this.setState({stat_table_type:"ride"})}}><MaterialCommunityIcons name="car" size={36} color="black" /></TouchableOpacity>
                <TouchableOpacity onPress={()=>{this.setState({stat_table_type:"aggr_ride"})}}><MaterialCommunityIcons name="car-multiple" size={36} color="black" /></TouchableOpacity>
                <TouchableOpacity onPress={()=>{this.setState({stat_table_type:"fee"})}}><FontAwesome5 name="hand-holding-usd" size={36} color="black" /></TouchableOpacity>
                
                <TouchableOpacity onPress={()=>{this.setState({stat_table_type:"aggr_fee"})}}><MaterialCommunityIcons name="cash-multiple" size={36} color="black" /></TouchableOpacity>
              </View>
              <View style={{flex: 1.5, borderBottomWidth: 0.5, flexDirection: 'row', justifyContent: 'space-between'}}>
                <TouchableOpacity onPress={()=>{this.setState({stat_table_type:"payment"})}}><MaterialCommunityIcons name="bank-transfer-in" size={42} color="black" /></TouchableOpacity>
                <TouchableOpacity onPress={()=>{this.setState({stat_table_type:"withdrawal"})}}><MaterialCommunityIcons name="bank-transfer-out" size={42} color="black" /></TouchableOpacity>
                <TouchableOpacity onPress={()=>{this.setState({stat_table_type:"cashback"})}}><MaterialCommunityIcons name="cash-refund" size={36} color="black" /></TouchableOpacity>
                <TouchableOpacity onPress={()=>{this.setState({stat_table_type:"bonus"})}}><AntDesign name="gift" size={36} color="black" /></TouchableOpacity>
                <TouchableOpacity onPress={()=>{this.setState({stat_table_type:"balance"})}}><MaterialIcons name="account-balance-wallet" size={36} color="black" /></TouchableOpacity>
              </View>

            </View>

          </View>

          {render_onboarding_bottom}

        </View>
          )
      }
      else
      if (this.state.page == "onboarding_feedback")
      {

        const DATA = [
  {
    id: 'bd7acbea-c1b1-46c2-aed5-3ad53abb28ba',
    title: 'First Item',
  },
  {
    id: '3ac68afc-c605-48d3-a4f8-fbd91aa97f63',
    title: 'Second Item',
  },
  {
    id: '58694a0f-3da1-471f-bd96-145571e29d72',
    title: 'Third Itemdddddddddddddd',
  },
];

const Item = ({ title }) => (
  <View>
    <Text>{title}</Text>
  </View>
);



          return (
        <View style={styles.container}>

          <View style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 40}}>

            <View style={{flex: 1, justifyContent: 'center'}}>
              <View style={{alignItems: 'center', justifyContent: 'center', flexDirection: 'row'}}>
                <View>
                  <Text style={{fontSize: 21, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}> INSTANT</Text>
                </View>
                <TouchableOpacity onPress={() => {this.setState({page: "best_order"})}}>
                  <MaterialCommunityIcons name="speedometer" size={48} color="black" />
                </TouchableOpacity>
                <View>
                  <Text style={{fontSize: 21, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>TAXI - фидбек</Text>
                </View>
              </View>
              <View style={{alignItems: 'center', justifyContent: 'center', flexDirection: 'row'}}><Text style={{fontSize: 11, color: 'black', fontWeight: 'bold', fontFamily: 'roboto', fontStyle: 'italic'}}>driver</Text></View>
            </View>

            <View style={{flex: 2}}><Text></Text></View>
            <View style={{flex: 10, justifyContent: 'flex-start'}}>

              <View style={{flex: 10, borderTopWidth: 1, alignItems: 'stretch', justifyContent: 'flex-start'}}>
                <View style={{flex: 1}}><Text></Text></View>
                <FlatList style={{flex: 10, alignItems: 'stretch'}}
                  data={DATA}
                  renderItem={({ item }) => (
                    <Item title={item.title} />
                  )}
                  keyExtractor={item => item.id}
                />
              </View>

              <View style={{flex: 1}}><Text></Text></View>
              <View style={{flex: 2, flexDirection: 'row', borderTopWidth: 1, borderBottomWidth: 1}}>
                <TextInput style={{flex: 10}} value="Тут будет ваше сообщение :-)"></TextInput>
                <TouchableOpacity><Feather style={{flex: 1}} name="send" size={24} color="black" /></TouchableOpacity>
              </View>

            </View>

          </View>

          {render_onboarding_bottom}

        </View>
          )
      }
      else
      // About the app
      if (this.state.page == "onboarding_QnA")
      {
        return (
        <View style={styles.container}>

          <View style={{flex: 1, alignItems: 'center', justifyContent: 'center', position: 'absolute', left : 10, right : 10, top: 40, bottom: 150}}>

          <ScrollView style={{flex: 1, alignItems: 'center', justifyContent: 'center'}} alwaysBounceVertical={false} bounces={false}>
            <View style={{flex: 1, flexDirection: 'row'}}>
              <Text style={{fontSize: 14, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}> INSTANT </Text><MaterialCommunityIcons name="speedometer" size={16} color="black"/><Text style={{fontSize: 14, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}> TAXI</Text>
            </View>
              <Text style={{fontSize: 9, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>часто задаваемые вопросы</Text>
            <View style={{flex: 4}}><Text></Text></View>
            <View style={{flex: 20, flexDirection: 'column', justifyContent: 'space-between', alignItems: 'flex-start'}}>
              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>Вопрос: почему я с вашим приложеием заработаю больше?</Text></View>
               <Text style={{fontSize: 11, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>Ответ: потому что а) у нас ниже комиссия чем у всех, б) в стоимости заказа заложена долгая подача или долгий возврат в место высокого спроса, в) вы не тратите время на вынужденные отказы, г) вы фильтруете заказы под себя, д) у не теряете деньги на штрафах, е) у нас нет рейтингов - хорошие заказы даются всегда всем водителям</Text>
              <View style={{flex: 1}}><Text></Text></View>

              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>Вопрос: если я заработаю больше, то пассажиры тоже будут платить больше? И если да, то откуда будет достаточный поток платящих пассажиров, чтобы загрузить меня заказами?</Text></View>
               <Text style={{fontSize: 11, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>Ответ: пассажиры будут платить не больше, они будут платить рыночную цену. Но наш сервис им выгодней остальных из-за быстой и надежной подачи, как раз потому что из-за отсутствия обязаловок и справедливого учета всех факторов в цене у вас не будет причин отменять заказы</Text>
              <View style={{flex: 1}}><Text></Text></View>

              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>Вопрос: не является ли ваша низкая комисся заманухой? Не планируете ли вы ее повышать?</Text></View>
              <Text style={{fontSize: 11, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>Ответ: нет. Наша низкая комиссия - не демпинг, а следствие эффективнсти нашего бизнеса и минимальности трат. У нас нет скидок неплатящей аудитории - это просто потеря денег. У нас нет дотаций водителям - т.к. если в одном регионе включена дотация, то для компенсации в другом включается повышенная комиссия - что приводит к потерям водителей. У нас нет маркетинга, мы не снимаем дорогие видео ролики, не платим за размещение рекламы на ТВ, у нас нет оплаты через карты (а это 2% от заказа - сумма соизмеримая с нашей комиссией - в других сервисах эти 2%, по сути, компенсиируются за счет водителей). У нас нет толпы бесполезных сотрудников: директоров, менеджеров, пиарщиков, продуктологов, маркетологов, эйчаров, дизайнеров, аналитиков. Мы все - программисты. У нас нет офиса и трат на его аренду. Мы все работаем из дома.</Text>
              <View style={{flex: 1}}><Text></Text></View>

              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>Вопрос: вы сможете гарантировать мне наличие заказов сразу после установки приложения?</Text></View>
               <Text style={{fontSize: 11, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>Ответ: не сразу. Мы сначала договариваемся с достаточным количеством водителей в городе, чтобы они все поставили себе приложение и включили бы его. Дальше мы смотрим на карту города, видим, плотность водителей с приложением, моделируем ситуацию - что будет, если массово раздать приложение пассажирам, какая будет цена и время подачи. Как только наша модель говорит нам, что город готов к запуску, мы начинаем массово раздавать приложение пассажирам. Это происходит в течении суток. И вы тут же начинаете видеть заказы.</Text>
              <View style={{flex: 1}}><Text></Text></View>

            </View>
          </ScrollView>
          </View>

          {render_onboarding_bottom}

        </View>
      );
    }
    else
    // About the app
    if (this.state.page == "onboarding_about")
    {
      return (
        <View style={styles.container}>


          <View style={{flex: 1, alignItems: 'center', justifyContent: 'center', position: 'absolute', left : 10, right : 10, top: 40, bottom: 130}}>

          <ScrollView style={{flex: 1, alignItems: 'center', justifyContent: 'center'}}>

            <View style={{flex: 1, flexDirection: 'row'}}>
              <Text style={{fontSize: 14, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}> Чем INSTANT </Text><MaterialCommunityIcons name="speedometer" size={16} color="black"/><Text style={{fontSize: 14, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}> TAXI лучше аналогов?</Text>
            </View>
            <View style={{flex: 2}}><Text></Text></View>
            <View style={{flex: 2}}><Text></Text></View>
            <View style={{flex: 20, flexDirection: 'column', justifyContent: 'space-between', alignItems: 'flex-start'}}>
              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>1. Пассажиры платят только наличными или через Сбербанк.Онлайн, вы сразу получаете деньги в карман, а не ждете их неделю.</Text></View>
              <View style={{flex: 1}}><Text></Text></View>
              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>2. Низкая комиссия, всего 3% или покупка смены за 150 руб.</Text></View>
              <View style={{flex: 1}}><Text></Text></View>
              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>3. Вы получаете кэшбэк по 10 рублей за каждый день, пока приложение у вас установлено и пуши не отключены. Кэшбэк можно вывести в наличные в любой момент.</Text></View>
              <View style={{flex: 1}}><Text></Text></View>
              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>4. Вы также получаете 15000 рублей бонус за установку приложения, который можно использовать для оплаты комиссии и покупку смен.</Text></View>
              <View style={{flex: 1}}><Text></Text></View>
              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>5. Вы всегда видите самый выгодный заказ, с автоматическим учетом в цене долгой подачи и возврата в место высокого спроса</Text></View><View style={{flex: 1}}><Text></Text></View>
              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>6. Вы всегда видите цену и место откуда, куда</Text></View><View style={{flex: 1}}><Text></Text></View>
              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>7. Никаких штрафов за отказ, роботов, обязаловок</Text></View><View style={{flex: 1}}><Text></Text></View>
              <View><Text  style={{fontSize: 12, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>8. Свой тариф, фильтр районов, заказы по пути, фильтр по дистанции подачи и поездки</Text></View>
            </View>
          </ScrollView>
          </View>

          {render_onboarding_bottom}

        </View>
      );
    }
    else
    // Splash screen
    if (this.state.page == "splash")
    {
      return (
        <View style={styles.container}>
          <View style={{flex: 1, alignItems: 'center', justifyContent: 'center'}}>
            <View><Text style={{fontSize: 21, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}> INSTANT</Text></View>
            <TouchableOpacity onPress={() => {this.setState({page: "best_order"})}}>
              <MaterialCommunityIcons name="speedometer" size={48} color="black" />
            </TouchableOpacity>
            <View style={{alignItems: 'flex-start'}}>
              <Text style={{fontSize: 21, color: 'black', fontWeight: 'bold', fontFamily: 'roboto'}}>TAXI</Text>
              <Text style={{fontSize: 11, color: 'black', fontWeight: 'bold', fontFamily: 'roboto', fontStyle: 'italic'}}>driver</Text>
              </View>
          </View>

          {render_onboarding_bottom}

        </View>
      );
    } // if (this.state.page == "splash")

    } // if (this.state.page.startsWith("onborading") || this.state.page == "splash")
    else

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
textShadowOffset: {width: 0, height: 4},
textShadowRadius: 4,
fontFamily: 'roboto',
fontStyle: 'bold',
fontWeight: '900',
alignItems: 'center',
textTransform: 'uppercase',
        fontSize: 30}}
        
        >Начать работать</Text>);
      else
        button_next = (<Text
        
        style={{flex: 0.8, borderColor: 'black', borderWidth: 1, backgroundColor: '#FFFFFF', color: 'black', textShadowColor: 'rgba(0, 0, 0, 0.25)',textAlign: 'center',
textShadowOffset: {width: 0, height: 4},
textShadowRadius: 4,
fontFamily: 'roboto',
fontStyle: 'bold',
fontWeight: '900',
alignItems: 'center',
textTransform: 'uppercase',
        fontSize: 17}}
        
        >Ждем ваши фотки :-)</Text>);


        // Show a registration form
        return (
<View style={{flex: 1, justifyContent: 'space-around'}}>

    <View style={{flex: 0.8}}></View>


    <Text  style={{flex: 0.8, backgroundColor: 'white', color: 'black',
            textShadowColor: 'rgba(0, 0, 0, 0.25)',textAlign: 'center',
textShadowOffset: {width: 0, height: 4},
textShadowRadius: 4,
fontFamily: 'roboto',
fontStyle: 'bold',
fontWeight: '900',
alignItems: 'center',
textTransform: 'uppercase',
        fontSize: 25}}>Регистрация</Text>



        <View style={{flex: 0.8, flexDirection: 'row', justifyContent: 'flex-end'}}>
          <Text style={{borderColor: 'gray', backgroundColor: 'white', borderWidth: 1, fontSize: 17, fontWeight: 'bold', flex: 0.9, height: 45}}>Ваш портрет</Text>
          <TouchableOpacity style={{flex:0.2}} onPress={() => {this.setState({page: "camera", page_camera_n: 0, camera_type: Camera.Constants.Type.front})}}><AntDesign name={this.state.pic_status[0]} size={48} color="black"/></TouchableOpacity>
        </View>

        <View style={{flex: 0.8, flexDirection: 'row', justifyContent: 'center'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth: 1,fontSize: 17,fontWeight: 'bold', flex: 0.9, height: 45 }}>Главная страница паспорта</Text>
          <TouchableOpacity style={{flex:0.2}} onPress={() => {this.setState({page: "camera", page_camera_n: 1, camera_type: Camera.Constants.Type.back})}}><AntDesign name={this.state.pic_status[1]} size={48} color="black"/></TouchableOpacity>
        </View>       

<View style={{flex: 0.8, flexDirection: 'row', justifyContent: 'space-around'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth:1,fontSize: 17,fontWeight: 'bold', flex: 0.9 , height: 45}}>Прописка в паспорте</Text>
          <TouchableOpacity style={{flex:0.2}} onPress={() => {this.setState({page: "camera", page_camera_n: 2, camera_type: Camera.Constants.Type.back})}}><AntDesign name={this.state.pic_status[2]} size={48} color="black"/></TouchableOpacity>
        </View>

        <View style={{flex: 0.8, flexDirection: 'row', justifyContent: 'space-between'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth: 1,fontSize: 17,fontWeight: 'bold', flex: 0.9,  height: 45}}>Права</Text>
          <TouchableOpacity style={{flex:0.2}} onPress={() => {this.setState({page: "camera", page_camera_n: 3, camera_type: Camera.Constants.Type.back})}}><AntDesign name={this.state.pic_status[3]} size={48} color="black"/></TouchableOpacity>
        </View>

        <View style={{flex: 0.8, flexDirection: 'row', justifyContent: 'space-evenly'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth: 1,fontSize: 17,fontWeight: 'bold', flex: 0.9,  height: 45}}>Обратная сторона прав</Text>
          <TouchableOpacity style={{flex:0.2}} onPress={() => {this.setState({page: "camera", page_camera_n: 4, camera_type: Camera.Constants.Type.back})}}><AntDesign name={this.state.pic_status[4]} size={48} color="black"/></TouchableOpacity>
        </View>


        <View style={{flex: 0.8, flexDirection: 'row'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth: 1,fontSize: 17,fontWeight: 'bold', flex: 0.9,  height: 45 }}>СТС</Text>
          <TouchableOpacity style={{flex:0.2}} onPress={() => {this.setState({page: "camera", page_camera_n: 5, camera_type: Camera.Constants.Type.back})}}><AntDesign name={this.state.pic_status[5]} size={48} color="black"/></TouchableOpacity>
        </View>

        <View style={{flex: 0.8, flexDirection: 'row'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth:1,fontSize: 17,fontWeight: 'bold', flex: 0.9,  height: 45 }}>Обратная сторона СТС</Text>
          <TouchableOpacity style={{flex:0.2}} onPress={() => {this.setState({page: "camera", page_camera_n: 6, camera_type: Camera.Constants.Type.back})}}><AntDesign name={this.state.pic_status[6]} size={48} color="black"/></TouchableOpacity>
        </View>

        <View style={{flex: 0.8, flexDirection: 'row'}}>
          <Text style={{borderColor:'gray',backgroundColor:'white',borderWidth: 1,fontSize: 17,fontWeight: 'bold', flex: 0.9,  height: 45 }}>Чистая машина с номером</Text>
          <TouchableOpacity style={{flex:0.2}} onPress={() => {this.setState({page: "camera", page_camera_n: 7, camera_type: Camera.Constants.Type.back})}}><AntDesign name={this.state.pic_status[7]}size={48} color="black"/></TouchableOpacity>
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
        <View style={{flex:1, alignItems: 'center', flexDirection: 'column'}}>
         
            <ImageBackground source={[   {uri: this.state.photo} ]}>

              <View style={{flex:0.7}}>
                <View style={{flex:0.1}}/>
                <Text style={{ flex: 0.9, color: 'white', fontSize: 25, fontStyle: 'bold' }}>
            {registration_pic_names[this.state.page_camera_n]}
                </Text>
              </View>

              <View style={{flex:0.3, flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between'}}>
                
                <View style={{flex:0.5, flexDirection: 'column'}}>
                  <TouchableOpacity style={{flex:0.6}} onPress={() => {
                    this.state.pic_status[this.state.page_camera_n] = 'checksquare';
                    this.setState({page: "registration"
                    })}}>
                    <AntDesign name="checksquare" size={96} color="white"/>
                  </TouchableOpacity>
                  <Text style={{flex:0.4, fontSize: 25, color: 'white'}}>Сохранить</Text>
                </View>

                <View style={{flex:0.5, flexDirection: 'column'}}>
                  <TouchableOpacity style={{flex:0.6}} onPress={() => {
                    this.setState({page: "camera"
                    })}}>
                    <AntDesign name="camera" size={96} color="white"/>
                  </TouchableOpacity>
                  <Text style={{flex:0.4, fontSize: 25, color: 'white'}}>Переснять</Text>
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
       style={{fontFamily: 'roboto', fontSize: 16, fontWeight: 'bold', lineHeight: 23,
display: 'flex',
alignItems: 'flex-start',
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
      {
        return (
      <View style={styles.container}>
        <MapView provider={PROVIDER_GOOGLE} style={styles.mapStyle}
          initialRegion={{
            latitude: 61.666594,
            longitude: 50.827201,
            latitudeDelta: 0.05,
            longitudeDelta: 0.05
          }}
          showsBuildings
          ref={ref => { this.map = ref }}

          //region={this.state.driver_region} If this property is set the best order map sticks with the driver  - that's why commented out

          >

<MapView.Marker coordinate={this.state.driver_region}>
  <View><MaterialIcons name="local-taxi" size={36} color="#4B470C"/></View>
</MapView.Marker>
<MapView.Marker pinColor='green' coordinate={this.state.best_order_A}>
  <View><MaterialCommunityIcons name="human-greeting" size={36} color="#013220"/></View>
</MapView.Marker>
<MapView.Marker pinColor='black' coordinate={this.state.best_order_B}>
  <View><FontAwesome5 name="flag-checkered" size={36} color="black"/></View>
</MapView.Marker>

<MapView.Polyline lineDashPattern={[25,3]}
coordinates={this.state.coords1}
	strokeColor='#999999'
	strokeWidth={10}/>

  <MapView.Polyline lineDashPattern={[25,3]}
coordinates={this.state.coords2}
	strokeColor='#CCCCCC'
	strokeWidth={10}/>

        </MapView>

      <View style={{flex:1, position: 'absolute', top: 30}}>

        <View style={{flex: 10, alignItems: "center"}}><Text style={{color: 'black', fontSize: 21, fontWeight: 'bold'}}>Лучший заказ</Text></View>
        <View style={{flex: 5}}><Text></Text></View>

        <View style={{flex: 10, flexDirection: 'row'}}>
          <MaterialCommunityIcons name="human-greeting" size={24} color="black" />
          <View><Text></Text></View>
          <Text style={{color: 'black', fontSize: 17, fontWeight: 'bold'}}>{(this.state.addr_A === "" || this.state.addr_A === undefined) ?"Неизвестный адрес" : this.state.addr_A}</Text>
        </View>
        <View style={{flex: 2}}>
          <Text style={{color: 'black', fontSize: 11, fontWeight: 'bold'}}>{(this.state.duration1/60.0).toFixed(1)} мин, {(this.state.distance1/1000.0).toFixed(1)}км</Text>
        </View>

        <View style={{flex: 10, flexDirection: 'row'}}>
          <FontAwesome5 name="flag-checkered" size={24} color="black" />
          <View><Text></Text></View>
          <Text style={{color: 'black', fontSize: 17, fontWeight: 'bold'}}>{(this.state.addr_B === "" || this.state.addr_B === undefined) ?"Неизвестный адрес" : this.state.addr_B}</Text>
        </View>
        <View style={{flex: 2}}>
          <Text style={{color: 'black', fontSize: 11, fontWeight: 'bold'}}>{(this.state.duration2/60.0).toFixed(1)} мин, {(this.state.distance2/1000.0).toFixed(1)}км</Text>
        </View>
        <View style={{flex: 5}}><Text></Text></View>

        <View style={{flex: 5, flexDirection: 'row'}}>

          <FontAwesome5 style={{flex: 1}} name="money-bill-alt" size={24} color={this.state.best_order !== undefined && this.state.best_order.rsf_only_cash === undefined ? 'gray' : 'black'} />
          <MaterialIcons style={{flex: 1}} name="smoke-free" size={24} color={this.state.best_order !== undefined && this.state.best_order.rsf_no_smoking === undefined ? 'gray' : 'black'} />
          <MaterialCommunityIcons style={{flex: 1}} name="bank-transfer" size={24} color={this.state.best_order !== undefined && this.state.best_order.rsf_only_sbrebank_online === undefined ? 'gray' : 'black'} />
          <MaterialCommunityIcons style={{flex: 1}} name="human-male-boy" size={24} color={this.state.best_order !== undefined && this.state.best_order.rsf_need_childseat === undefined ? 'gray' : 'black'} />
          <FontAwesome5 style={{flex: 1}} name="dog" size={24} color={this.state.best_order !== undefined && this.state.best_order.rsf_with_pet === undefined ? 'gray' : 'black'} />

          <View style={{flex: 1}}><Text></Text></View>
          <Text style={{flex: 1, color: 'black', fontSize: 19, fontWeight: 'bold'}}>{Math.round(this.state.best_order_price) == 1000000000 ? 123 : Math.round(this.state.best_order_price)}</Text>
          <MaterialCommunityIcons style={{flex: 1}} name="currency-rub" size={24} color="black" />
        </View>
      </View>

      <View style={{flex:1, position: 'absolute', bottom: 10}}>
        <TouchableOpacity style={{flex :1, alignItems: "center", backgroundColor: "#d1ffd2", padding: 10, borderColor: 'black', borderWidth: 1}}
          onPress={() => {
            server_api_sdk.api_driver_accept_order(this.state.best_order_id);
            //this.setState({page: "assigned_order", next_turn_description_line1: "Выезжаем на подачу", next_turn_description_line2 : ""});
          }}>
          <Text style={{ fontSize: 25, textTransform: 'uppercase', fontFamily: 'roboto', fontWeight: 'bold'}}>Принять заказ</Text>
        </TouchableOpacity>
        <View style={{flex: 1}}><Text></Text></View>
        <View style={{flex: 1, flexDirection: 'row', justifyContent: 'space-between'}}>
          <TouchableOpacity onPress={()=>{this.setState({is_in_chat: !this.state.is_in_chat})}}><MaterialIcons name="feedback" size={36} color="black" /></TouchableOpacity>
          <TouchableOpacity><Entypo name="help" size={36} color="black" /></TouchableOpacity>
          <TouchableOpacity><FontAwesome name="list-ol" size={36} color="black" /></TouchableOpacity>
          <TouchableOpacity><Feather name="settings" size={36} color="black" /></TouchableOpacity>
          <TouchableOpacity><AntDesign name="filter" size={36} color="black" /></TouchableOpacity>
          <TouchableOpacity><Ionicons name="md-stats" size={36} color="black" /></TouchableOpacity>
        </View>
      </View>


      </View>
      );

      } // // If there is a best order then show it the driver

    } // if (this.state.page == "best_order")
    else
    // The driver has an assigned order
    if (this.state.page == "assigned_order")
    {
      //console.log(`IN RENDER: this.state.page == "assigned_order"`);

      // If the driver is in the chat vs a rider or a support then
      // show the chat form above the map
      var render_chat;
      if (this.state.is_in_chat)
      {

          const DATA = [
          {
            id: 'bd7acbea-c1b1-46c2-aed5-3ad53abb28ba',
            title: 'First Item',
          },
          {
            id: '3ac68afc-c605-48d3-a4f8-fbd91aa97f63',
            title: 'Second Item',
          },
          {
            id: '58694a0f-3da1-471f-bd96-145571e29d72',
            title: 'Third Itemdddddddddddddd',
          },
          ];

          const Item = ({ title }) => (
            <View><Text style={{backgroundColor: 'white'}}>{title}</Text></View>);

          render_chat = (
            <View style={{flex: 10, position: 'absolute', left : 10, right : 10, alignItems: "center", justifyContent: 'flex-start'}}>
              <View style={{flex: 10, borderTopWidth: 1, alignItems: 'stretch', justifyContent: 'flex-start'}}>
                <FlatList style={{flex: 10, alignItems: 'stretch'}}
                  data={DATA}
                  renderItem={({ item }) => (
                    <Item title={item.title} />
                  )}
                  keyExtractor={item => item.id}
                />
              </View>
              <View style={{flex: 1}}><Text></Text></View>
              <View style={{flex: 2, flexDirection: 'row', borderTopWidth: 1, borderBottomWidth: 1}}>
                <TextInput style={{flex: 10, backgroundColor: 'white'}} value="Тут будет ваше сообщение :-)"></TextInput>
                <TouchableOpacity><Feather style={{flex: 1}} name="send" size={24} color="black" /></TouchableOpacity>
              </View>
            </View>
        )
      } // if (this.state.is_in_chat)

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


      var render_minutes_and_seconds
      var render_distance_to_go = "";

      // Arrived either to A or to B - change text in the bottom
      if (!minutes && !seconds || !distance_to_go)
      {
        if (this.state.assigned_order_status == "ride_begins" || this.state.assigned_order_status == "payment")
          render_minutes_and_seconds = "приехали по адресу"
        else
          render_minutes_and_seconds = "приехали на точку подачи"
      }
      else
      if (!minutes)
      {
        render_minutes_and_seconds = `${seconds} сек, `
        render_distance_to_go = `${distance_to_go} км`
      }
      else
      {
        render_minutes_and_seconds = `${minutes} мин`
        if (seconds)
          render_minutes_and_seconds += ` ${seconds} сек, `
        render_distance_to_go = `${distance_to_go} км`
      }


      var render_assigned_order_info = (<View></View>)

      /*
      *   Render various assigned order related buttons depending on its status
      */

      // Got B and wait for payment
      if (this.state.assigned_order_status == "payment")
      {
        render_assigned_order_info = (<TouchableOpacity style={{flex: 1, position: 'absolute', left : 10, right : 10, alignItems: "center", backgroundColor: "#d1ffd2", padding: 10, borderColor: 'black', borderWidth: 1}} onPress={() => {
          server_api_sdk.api_driver_ride_finishes();
          this.setState({best_order_id : -1, assigned_order_status: "", page: "best_order"});
          }}>
        <Text style={{fontFamaly: 'roboto', textTransform: 'uppercase', fontStyle: 'bold', fontSize: 15}}> Закрываем заказ </Text></TouchableOpacity>)
      } // if (this.state.assigned_order_status == "payment")
      else
      // Ride in progress
      if (this.state.assigned_order_status == "ride_begins")
      {
        // If the driver is close to B then show her/him the in-place button
        // Note: we calc distance to B by bird flight just in case if something goes
        //  wrong with the route - for driver to close the order
        //if (this.state.distance2 < 100)
        if (this.state.close_to_B)
          render_assigned_order_info = (<TouchableOpacity style={{flex: 1, position: 'absolute', left : 10, right : 10, alignItems: "center", backgroundColor: "#d1ffd2", padding: 10, borderColor: 'black', borderWidth: 1}} onPress={() => { return this.setState({assigned_order_status: "payment"})}}>
            <Text style={{fontFamaly: 'roboto', textTransform: 'uppercase', fontStyle: 'bold', fontSize: 15}}> Почти приехали - можно просить оплату </Text>
            </TouchableOpacity>)
        else
          render_assigned_order_info = (<View></View>)
      } // if (this.state.assigned_order_status == "ride_begins")
      else
      if (this.state.assigned_order_status == "at_A")
      {
        /*
        *   These are possible states here:
        *
        *   1.  Driver is waiting for a rider for free
        *         info: seconds of waiting
        *         actions: start a ride
        *   2.  Toll is toll-waiting for a rider
        *         info: seconds of toll waiting
        *         actions:
        *           start a ride without toll waiting,
        *           start a ride with acceptance of toll waiting
        *           cancel a ride
        */

        if (this.state.at_A_is_toll_waiting)
        {
          render_assigned_order_info = (
            <View style={{flex: 1, position: 'absolute', left : 10, right : 10, alignItems: "center"}}>
              <TouchableOpacity style={{flex: 1, alignItems: "center", backgroundColor: "#d1ffd2", padding: 10, borderColor: 'black', borderWidth: 1}} onPress={() => {
              // Begin the ride
              server_api_sdk.api_driver_ride_begins();
              this.setState({assigned_order_status: "ride_begins", next_turn_description_line1: "Едем с пассажиром", next_turn_description_line2: ""})
            }}>
                <Text style={{fontFamaly: 'roboto', textTransform: 'uppercase', fontStyle: 'bold', fontSize: 25}}> Начать поездку </Text>
              </TouchableOpacity>

              <View style={{flex: 1}}><Text></Text></View>
              <View style={{flex: 1, justifyContent: 'center', alignItems: 'center'}}>
                <Text style={{flex: 1, fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: 15}}>Платное ожидание {this.state.wait_time_toll_minutes} мин, {this.state.wait_time_toll_seconds} сек</Text>
                <View style={{flex: 1}}><Text></Text></View>
                <View style={{flex: 1, flexDirection: 'row'}}>
                  <TouchableOpacity onPress={() =>
                  {

                    Alert.alert(
                    'Подтверждение платного ожидания',
                    `Подтверждаете учет в цене платного ожидания ${Math.ceil(this.state.at_A_waiting_toll_seconds/60)} минут?`,
                    [
                      {
                        text: "Да",
                        onPress: () => {
                          server_api_sdk.api_driver_accepted_toll_wait(this.state.at_A_waiting_toll_seconds);
                        }
                      },
                      {
                        text: "Пока нет",
                        onPress: () => console.log("Cancel Pressed")
                      }
                    ], { cancelable: false })

                  }}><AntDesign name="checksquareo" size={48} color="black"/></TouchableOpacity>

                  <TouchableOpacity onPress={() => {
                    
                    Alert.alert(
                    'Отмена платного ожидание',
                    `Отменяем учет в цене платного ожидания ${Math.ceil(this.state.at_A_waiting_toll_seconds/60)} минут?`,
                    [
                      {
                        text: "Да",
                        onPress: () => {
                          server_api_sdk.api_driver_accepted_toll_wait(0);
                        }
                      },
                      {
                        text: "Пока нет",
                        onPress: () => console.log("Cancel Pressed")
                      }
                    ], { cancelable: false })
                    
                    }}><MaterialIcons name="cancel" size={48} color="black"/></TouchableOpacity>
                </View>
              </View>
            </View>
          )
        }
        else
        {
          render_assigned_order_info = (
            <View style={{flex: 1, position: 'absolute', left : 10, right : 10, alignItems: "center"}}>
              <TouchableOpacity style={{flex: 1, alignItems: "center", backgroundColor: "#d1ffd2", padding: 10, borderColor: 'black', borderWidth: 1}} onPress={() => {
              // Begin the ride
              server_api_sdk.api_driver_ride_begins();
              this.setState({assigned_order_status: "ride_begins", next_turn_description_line1: "Едем с пассажиром", next_turn_description_line2: ""})
            }}>
                <Text style={{fontFamaly: 'roboto', textTransform: 'uppercase', fontStyle: 'bold', fontSize: 25}}> Начать поездку </Text>
              </TouchableOpacity>
              <View style={{flex: 1}}><Text></Text></View>
              <View style={{flex: 1, flexDirection: 'row'}}>
                <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: 15}}>Ожидаю пассажира {this.state.wait_time_minutes} мин, {this.state.wait_time_seconds} сек</Text>
              </View>
            </View>
          )
        }


      } // if (this.state.assigned_order_status == "at_A")
      else
      // On the way to A
      {
          // If the driver is close to A then show her/him the in-place button
          // Note: we calc distance to A by bird flight just in case if something goes
          //  wrong with the route - for driver to close the order
          if (this.state.close_to_A)
          {
          //if (this.state.distance1 < 200)
            render_assigned_order_info = (<TouchableOpacity style={{flex: 1, position: 'absolute', left : 10, right : 10, alignItems: "center", backgroundColor: "#d1ffd2", padding: 10, borderColor: 'black', borderWidth: 1}} onPress={() => {
                server_api_sdk.api_driver_got_A();

                // TODO! If the server slows down then we will re-render the app without
                //  having received info from the sever which will result in blank
                //  waiting minutes and seconds

                return this.setState({assigned_order_status: "at_A", next_turn_description_line1: "Пассажир где-то рядом :-)", next_turn_description_line2: ""})
            }}>
            <Text style={{fontFamaly: 'roboto', textTransform: 'uppercase', fontStyle: 'bold', fontSize: 25}}>Приехал на подачу</Text>
            </TouchableOpacity>
            )

        

          }
          else
            render_assigned_order_info = (<View></View>)
      }

      // Continue building thigs to render a page "assigned_order"

      // Render the price
      let price;
      if (Math.round(this.state.best_order_price) == 1000000000)
        price = 123;
      else
        price = Math.round(this.state.best_order_price);

      let is_price_with_toll_wait = false;
      if (this.state.price_for_toll_wait && this.state.price_for_toll_wait != -1)
      {
        is_price_with_toll_wait = true;
        price = `${price} (+ ${this.state.price_for_toll_wait})`;
      }

      // Price with extras is rendered with a smaller font size to fit the screen
      let render_price;
      if (is_price_with_toll_wait)
        render_price = (<Text style={{flex: 2, color: 'black', fontSize: 15, fontWeight: 'bold'}}>{price}</Text>);
      else
        render_price = (<Text style={{flex: 2, color: 'black', fontSize: 19, fontWeight: 'bold'}}>{price}</Text>);



      /*
          <MapView.Marker coordinate={this.state.next_turn_coordinate} ref={ref => { this.marker = ref}}>
              <View></View>
              <MapView.Callout tooltip={true}>
                <View>

                <MaterialCommunityIcons name={this.state.next_turn_direction} size={36} color="black"/>


                </View>
              </MapView.Callout>
            </MapView.Marker>

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
              // onRegionChangeComplete={() => this.marker.showCallout()}
          >

            <MapView.Marker coordinate={this.state.driver_region}/*coordinate={this.state.driver_marker_coordinate}*/>

              <View><MaterialIcons name="local-taxi" size={36} color="black"/></View>
            </MapView.Marker>

<MapView.Marker pinColor='green' coordinate={this.state.best_order_A}>
  <View><MaterialCommunityIcons name="human-greeting" size={36} color="black"/></View>
</MapView.Marker>
<MapView.Marker pinColor='black' coordinate={this.state.best_order_B}>
  <View><FontAwesome5 name="flag-checkered" size={36} color="black" /></View>
</MapView.Marker>






        

      


<MapView.Polyline lineDashPattern={[25,3]}
coordinates={this.state.coords1}
	strokeColor='#999999'
	strokeWidth={10}/>

  <MapView.Polyline lineDashPattern={[25,3]}
coordinates={this.state.coords2}
	strokeColor='#CCCCCC'
	strokeWidth={10}/>
</MapView>

        <View style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 30}}>
          <View style={{alignItems: "center", padding: 10, borderColor: 'black', borderWidth: 1, flex: 1, flexDirection: "column", backgroundColor: 'white'}}>
            <Text style={{color: 'black', fontFamily: 'roboto', fontWeight:'bold', fontSize: 20}}>{this.state.next_turn_description_line1}</Text>
            <Text style={{color: 'black', fontFamily: 'roboto', fontWeight:'bold', fontSize: 20}}>{this.state.next_turn_description_line2}</Text>
          </View>

          <View style={{flex: 1}}><Text></Text></View>
          <View style={{flex: 1}}>
            <Text style={{color: 'black', fontSize: 11, fontWeight: 'bold'}}>
            {(this.state.assigned_order_status == "ride_begins" ||
              this.state.assigned_order_status == "payment") ? this.state.addr_B : this.state.addr_A
              }</Text>
          </View>
          <View style={{flex: 1}}><Text></Text></View>

          <View style={{flex: 5, flexDirection: 'row'}}>

            <FontAwesome5 style={{flex: 1}} name="money-bill-alt" size={24} color={this.state.best_order !== undefined && this.state.best_order.rsf_only_cash === undefined ? '#d3d3d3' : 'black'} />
            <MaterialIcons style={{flex: 1}} name="smoke-free" size={24} color={this.state.best_order !== undefined && this.state.best_order.rsf_no_smoking === undefined ? '#d3d3d3' : 'black'} />
            <MaterialCommunityIcons style={{flex: 1}} name="bank-transfer" size={24} color={this.state.best_order !== undefined && this.state.best_order.rsf_only_sbrebank_online === undefined ? '#d3d3d3' : 'black'} />
            <MaterialCommunityIcons style={{flex: 1}} name="human-male-boy" size={24} color={this.state.best_order !== undefined && this.state.best_order.rsf_need_childseat === undefined ? '#d3d3d3' : 'black'} />
            <FontAwesome5 style={{flex: 1}} name="dog" size={24} color={this.state.best_order !== undefined && this.state.best_order.rsf_with_pet === undefined ? '#d3d3d3' : 'black'} />
  
            <View style={{flex: 1}}><Text></Text></View>
            <View style={{flex: 3, flexDirection: 'row', justifyContent: 'flex-end'}}>
              {render_price}
              <MaterialCommunityIcons style={{flex: 1}} name="currency-rub" size={24} color="black" />
            </View>
          </View>
        
        </View>

          {render_assigned_order_info}
          {render_chat}

        <View style={{flex: 1, position: 'absolute', left : 10, right : 10, bottom: 20, alignItems: "center"}}>
          <View style={{flex: 0.4, flexDirection: 'row'}}>
            <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: 15}}>{render_minutes_and_seconds}</Text>
            <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: 15}}>{render_distance_to_go}</Text>
          </View>

          <View style={{flex: 0.2}}><Text></Text></View>
          <View style={{flex: 0.4, flexDirection: 'row'}}>
            <TouchableOpacity onPress={()=>{this.setState({is_in_chat: !this.state.is_in_chat})}}><MaterialIcons name="feedback" size={36} color="black"/></TouchableOpacity>
            <TouchableOpacity onPress={()=>{this.setState({is_in_chat: !this.state.is_in_chat})}}><MaterialIcons name="chat" size={36} color="black"/></TouchableOpacity>
            <TouchableOpacity onPress={()=>{Linking.openURL("tel:+79167098111")}} ><AntDesign name="phone" size={36} color="black"/></TouchableOpacity>
            <TouchableOpacity onPress={() => {
                
                // If ride is in progress then "flag" goes to payment
                if (this.state.assigned_order_status == "ride_begins")
                {
                  Alert.alert(
                    'Завершение поездки - оплата',
                    'Вы уверены, что поездка завершена?',
                    [
                      {
                        text: "Да",
                        onPress: () => {
                          // Go to the payment
                          this.setState({assigned_order_status: "payment"})
                        }
                      },
                      {
                        text: "Нет",
                        onPress: () => console.log("Cancel Pressed")
                      }
                    ], { cancelable: false })
                }
                // If ride is not in progress yet then "flag" goes to at A
                else
                {
                   Alert.alert(
                    'Приехали на точку подачи',
                    'Вы уверены, что вы приехали на точку подачи?',
                    [
                      {
                        text: "Да",
                        onPress: () => {
                          server_api_sdk.api_driver_got_A();
                          this.setState({assigned_order_status: "at_A", next_turn_description_line1: "Пассажир где-то рядом :-)", next_turn_description_line2: ""})
                        }
                      },
                      {
                        text: "Нет",
                        onPress: () => console.log("Cancel Pressed")
                      }
                    ], { cancelable: false })
                }}}><FontAwesome5 name="flag-checkered" size={36} color="black"/></TouchableOpacity>
                
            <TouchableOpacity onPress={()=>{this.setState({is_in_chat: !this.state.is_in_chat})}}><MaterialIcons name="watch-later" size={36} color="black"/></TouchableOpacity>

            <TouchableOpacity onPress={() => {
                
                Alert.alert(
                  'Отмена заказа',
                  'Вы точно хотите отменить заказ?',
                  [
                    {
                      text: "Да",
                      onPress: () => {
                        // Cancel the ride
                        server_api_sdk.api_driver_cancelled_order();
                        // After the cancellation we go to the next order
                        this.setState({page: "best_order"})
                      },
                      style: "cancel"
                    },
                    {
                      text: "Нет",
                      onPress: () => console.log("Cancel Pressed")
                    }
                ], { cancelable: false })


              }}><MaterialIcons name="cancel" size={36} color="black"/></TouchableOpacity>
            <TouchableOpacity><MaterialIcons name="done" size={36} color="#d3d3d3"/></TouchableOpacity>
          </View>
        </View>

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


