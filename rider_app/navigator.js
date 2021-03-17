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
import { SimpleLineIcons } from '@expo/vector-icons';

import * as route_cache from './components/route_cache.js'
import * as localtion_cache from './components/location_cache.js'
import * as server_api_sdk from './components/server_api_sdk.js'

import * as h3core from './components/uber-h3-js/lib/h3core.js';


export default class Navigator extends React.Component {

  constructor(props) {
    super(props);
    this.state = {
      duration: 0,
      distance: 0,
      page: "change_B",
      driver_region: {
                latitude: 55.5442069,
                longitude: 37.5213535,
                latitudeDelta: 0.1,
                longitudeDelta: 0.1
              },
      pointA: {
                latitude: 55.5442069,
                longitude: 37.5213535},
      pointB: undefined,
      coords: [],
      next_turn_description_line1: "",
      next_turn_description_line2: ""
    };

    //this.onPointA_drag = this.onPointA_drag.bind(this);
    this.changeA_address_and_point = this.changeA_address_and_point.bind(this);

    this.addrA_InputRef = React.createRef();
    this.addrB_InputRef = React.createRef();

    this.on_message_received = this.on_message_received.bind(this);
  }

  componentDidMount()
  {

  }

  // Handles a message from the marketplace to update the state
  on_message_received()
  {
    let rider = server_api_sdk.api_get_current_order();
    let best_driver = server_api_sdk.api_get_current_driver();

    if (best_driver !== undefined)
    {
      this.setState({
        best_driver_coordinate: {latitude: best_driver.lat, longitude: best_driver.lon},
      });
    }

    if (rider !== undefined)
    {
      this.setState({
        order_price: rider.price,
        suggested_price: rider.suggested_price,
        best_pickup_ETA: rider.best_pickup_ETA,
        order_status: rider.status // "price_view", "order", "assigned_order"
      });
    }

    //console.log(`on_message_received: rider=${JSON.stringify(rider)}, best_driver=${JSON.stringify(best_driver)}`);
  }

  changeA_address_and_point(address)
  {
      //console.log(`address resolved3: ${address}`);

                      this.setState({
                        addrA: address//,
                        //addrA_suggestion: address,
                      });

                      //console.log(`address resolved2: ${address}, addrA=${this.state.addrA}, addrA_suggestion=${this.state.addrA_suggestion}, pointA=${this.state.pointA}`);
  }


  buildDirections_navigator(from, to)
  {
    //console.log(`buildDirections_navigator(${JSON.stringify(from)}, ${JSON.stringify(to)})`);


    // Build the route
    route_cache.get_route(from, to,
      function(route_object)
      {

        // Remove the first point which is "form" to prevent route from going
        // not on the road network
        route_object.route.shift();

      // We got the route! Decorate it with the turn info
//      var decoration = this.decorateRoute(this.state.pointA, route_object.route);
        this.setState({

            coords: route_object.route,
            distance: route_object.distance,
            duration: route_object.duration,           
  //          next_turn_direction: decoration.next_turn_direction,
    //        next_turn_coordinate: decoration.next_turn_coordinate,
      //      next_turn_description_line1: decoration.next_turn_description_line1,
        //    next_turn_description_line2: decoration.next_turn_description_line2
        })

      }.bind(this)
    );

    /*this.map.animateCamera({heading :
                  0,
                  center: from,
                  pitch: 45, altitude: 300, zoom: 17}, {duration: 1100});*/
  } // buildDirections_navigator

  render()
  {
      // Get miutes and seconds to go
      var minutes;
      var seconds = Math.trunc(this.state.duration)
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
      var distance_to_go = ((this.state.distance)/1000).toFixed(1);
      var render_minutes_and_seconds
      var render_distance_to_go = "";

      // Arrived either to A or to B - change text in the bottom
      if (!minutes && !seconds || !distance_to_go)
      {
          render_minutes_and_seconds = "приехали"
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

      let render_change_address1;
      let render_change_address2;

      // This is for price view
      if (this.state.page == "price_view")
      {
        if (this.state.order_price !== undefined && this.state.best_pickup_ETA !== undefined)
          render_change_address1 = (
          <View style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 30, alignItems: 'center', justifyContent: 'center'}}>
            <Text style={{fontSize: 15}}>Поездка займет {render_minutes_and_seconds}</Text>
            <Text style={{fontSize: 30, fontWeight: 'bold'}}>{this.state.order_price} рублей</Text>
            <Text style={{fontSize: 15, fontWeight: 'bold'}}>Время подачи {this.state.best_pickup_ETA.toFixed(0)} минут</Text>
          </View>
        )
        else
          render_change_address1 = (
          <View style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 30, alignItems: 'center', justifyContent: 'center'}}>
            <Text style={{fontSize: 21}}>{render_minutes_and_seconds}</Text>
          </View>
          );

        render_change_address2 = (
          <View style={{flex: 1, position: 'absolute', left : 10, right : 10, bottom: 10, alignItems: 'center', justifyContent: 'center'}}>
            <TouchableOpacity style={{flex: 1}}
            
            onPress={()=>{     
              // Go to change the A + undef the location
              this.setState({page: "change_A_street", got_a_location: undefined})
            }}
            
            ><Text style={{textAlignVertical: 'center', width: Dimensions.get('window').width, height: 45, fontSize: 21, borderWidth: 0.5, backgroundColor: 'white'}}>{this.state.addrA}</Text></TouchableOpacity>
            <View style={{flex: 1}}></View>
            <TouchableOpacity style={{flex: 1, width: Dimensions.get('window').width, height: 45, borderWidth: 0.5, backgroundColor: 'white', justifyContent: 'center'}}
            
            onPress={()=>{
              // Go to the entering the address of B
              this.setState({page: "enter_B_address"});
            }}

            ><Text style={{fontSize: 21}}>{this.state.addrB}</Text></TouchableOpacity>
            <View style={{flex: 1}}></View>
            <TouchableOpacity style={{flex: 1,  height: 45, borderWidth: 0.5, backgroundColor: '#FDFD96', width: Dimensions.get('window').width, alignItems: 'center', justifyContent: 'center'}}><Text style={{textAlign: 'center', fontSize: 21}}>Заказать</Text></TouchableOpacity>
          </View>
        )
      }
      else
      // This is for entering the address of B
      if (this.state.page == "enter_B_address")
      {
          let render_suggest = (<View></View>);

          // Render a B suggest if it's ready
          if (this.state.street_suggest_ready !== undefined)
          {

            render_suggest =(
            <TouchableOpacity style={{flex: 1}} onPress={()=>{

              // If a user selects a suggest then change the text and remove the suggest
              // also blur the focus from TextInput
              this.setState({addrB: this.state.street_suggest_ready, street_suggest_ready: undefined});

              this.addrB_InputRef.current.blur();

              // If additionally suggest comes with a location then go to the price view page
              if (this.state.got_a_location !== undefined)
              {
                this.setState({page: "price_view", point_B_location: this.state.got_a_location});

                  // Everytime point B changes - notify the marketplace to get
                  // price/ETA
                server_api_sdk.api_rider_set_B(this.state.got_a_location, this.on_message_received);

                let point_A_real_location = this.state.point_A_location;
                if (point_A_real_location === undefined)
                  point_A_real_location = this.map.region;

                this.map.animateCamera({heading :
                      0,
                      center: point_A_real_location,
                      pitch: 45, altitude: 300, zoom: 17}, {duration: 1100});

                console.log(`point_A_real_location=${point_A_real_location}, this.state.point_A_location=${this.state.point_A_location}`);

                this.buildDirections_navigator(point_A_real_location, this.state.got_a_location);
              }

            }}><Text style={{width: Dimensions.get('window').width, height: 45, fontSize: 21, borderWidth: 0.5, backgroundColor: 'white'}}>{this.state.street_suggest_ready}</Text></TouchableOpacity>
            )
          }

           // If additionally suggest comes with a location then animate camera to that
              // and return to point B selection
              /*if (this.state.got_a_location !== undefined)
              {
                this.setState({page: "change_B"});

                this.map.animateCamera({heading :
                      0,
                      center: this.state.got_a_location,
                      pitch: 45, altitude: 300, zoom: 17}, {duration: 1100});
              }*/

          render_change_address1 = (
          <View style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 30, alignItems: 'center', justifyContent: 'center'}}>
            <Text style={{flex: 1, fontSize: 19, height: 30}}>Ваш адрес ></Text>
            <Text style={{flex: 1, fontSize: 19, fontWeight: 'bold', height: 30}}>{this.state.addrA}</Text>
            <Text style={{flex: 1, fontSize: 19, height: 30}}>Куда поедем ></Text>
            <TextInput selectTextOnFocus={true} style={{flex: 1, height: 45, width: Dimensions.get('window').width, fontSize: 21, borderWidth: 0.5, backgroundColor: 'white'}} value={this.state.addrB} ref={this.addrB_InputRef} onChangeText={
              function(text) {
                this.setState({addrB: text});
                localtion_cache.auto_complete(text, this.state.point_A_location, function(name, location)
                {
                  // Got a location - go to change_B + change map position to that location
                  if (location !== undefined)
                  {
                    this.setState({got_a_location: location});
                  }
                  else
                  {
                    // If we haven't got a location then at least we got a street 
                  }

                  this.setState({
                      street_suggest_ready: name,
                      addrB_suggestion: name,
                      pointB_from_suggestion: location
                  });
                }.bind(this));
              }.bind(this)
            }/>
            {render_suggest}
          </View>
        );
      }
      else
      //  On this page we try to auto complete a street name or a POI name
      if (this.state.page === "change_A_street")
      {

        // Render an A suggest if it's ready
        let render_suggest = (<View></View>);
        if (this.state.street_suggest_ready !== undefined)
        {

          render_suggest =(
            <TouchableOpacity style={{flex: 1}} onPress={()=>{

              // If a user selects a suggest then change the text and remove the suggest
              // also blur the focus from TextInput
              this.setState({addrA: this.state.street_suggest_ready, street_suggest_ready: undefined});

              this.addrA_InputRef.current.blur();

              // If additionally suggest comes with a location then animate camera to that
              // and return to point B selection
              if (this.state.got_a_location !== undefined)
              {
                this.setState({page: "change_B", point_A_location: this.state.got_a_location});

                  // Everytime point A changes - notify the marketplace to get
                  // price/ETA
                server_api_sdk.api_rider_set_A(this.state.got_a_location, this.on_message_received);

                this.map.animateCamera({heading :
                      0,
                      center: this.state.got_a_location,
                      pitch: 45, altitude: 300, zoom: 17}, {duration: 1100});
              }

            }}><Text style={{width: Dimensions.get('window').width, height: 45, fontSize: 21, borderWidth: 0.5, backgroundColor: 'white'}}>{this.state.street_suggest_ready}</Text></TouchableOpacity>
          )
        }

        render_change_address1 = (
          <View style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 30, alignItems: 'center', justifyContent: 'center'}}>
            <Text style={{flex: 1, fontSize: 19, height: 30, fontWeight: 'bold'}}>Ваш адрес ></Text>
            <TextInput selectTextOnFocus={true} style={{flex: 1, height: 45, width: Dimensions.get('window').width, fontSize: 21, borderWidth: 0.5, backgroundColor: 'white'}} value={this.state.addrA} ref={this.addrA_InputRef} onChangeText={
              function(text) {
                this.setState({addrA: text});
                localtion_cache.auto_complete(text, this.state.point_A_location, function(name, location)
                {
                        //console.log(`autocomplete(${text}) result=${JSON.stringify(name)}, ${JSON.stringify(location)}`);
                  // Got a location - go to change_B + change map position to that location
                  if (location !== undefined)
                  {
                    this.setState({got_a_location: location});
                  }
                  else
                  {
                    // If we haven't got a location then at least we got a street 
                  }

                  this.setState({
                      street_suggest_ready: name,
                      addrA_suggestion: name,
                      pointA_from_suggestion: location
                  });
                }.bind(this));
              }.bind(this)
            }/>
            {render_suggest}
          </View>
        )
      }
      else
      if (this.state.page === "change_B")
      {
        render_change_address1 = (

          <View style={{flex: 1, position: 'absolute', left : 10, right : 10, top: 30, alignItems: 'center', justifyContent: 'center'}}>
            <TouchableOpacity style={{flex: 1}} onPress={()=>{
              
              // Change the A + undef the location
              this.setState({page: "change_A_street", got_a_location: undefined})
            }}><Text style={{fontSize: 19, height: 30}}>Ваш адрес ></Text></TouchableOpacity>
            <Text style={{flex: 1, fontSize: 19, fontWeight: 'bold', height: 30}}>{this.state.addrA}</Text>
          </View>);


        render_change_address2 = (
          <View style={{flex: 1, position: 'absolute', left : 10, right : 10, bottom: 30, alignItems: 'center', justifyContent: 'center'}}>
            <TouchableOpacity style={{flex: 1, height: 45}} onPress={()=>{
              // Go to the entering the address of B
              this.setState({page: "enter_B_address"});
            }}>
            <Text style={{flex: 1, fontSize: 19, fontWeight: 'bold'}}>Куда поедем ></Text></TouchableOpacity>
          </View>

          )
      }


      /*let render_suggestion_A = undefined;
      if ((this.state.pointA === undefined
        || this.state.addrA != this.state.addrA_suggestion) &&
          this.state.addrA_suggestion !== undefined)
      {
        render_suggestion_A = (
          <TouchableOpacity style={{flex: 1}} onPress={function (){

                  if (this.state.pointA_from_suggestion === undefined)
                   this.setState({
                    addrA: this.state.addrA_suggestion
                   
                  })
                  else
                  this.setState({
                    addrA: this.state.addrA_suggestion,
                    pointA: this.state.pointA_from_suggestion
                  })

                  //this.map.region = this.state.pointA_from_suggestion;
                  
                 // if (this.pointA_from_suggestion !== undefined &&
                   //   this.state.pointB !== undefined)
                    //  this.buildDirections_navigator();

                  }.bind(this)
                }><Text style={{fontSize: 19, backgroundColor: '#9f89d1'}}>{this.state.addrA_suggestion}</Text></TouchableOpacity>
        )
      }*/


      /*let render_suggestion_B = undefined;
      if ((this.state.pointB === undefined
          || this.state.addrB != this.state.addrB_suggestion) &&
            this.state.addrB_suggestion !== undefined)
      {
        render_suggestion_B = (
          <TouchableOpacity style={{flex: 1}} onPress={function (){
                  this.setState({
                    addrB: this.state.addrB_suggestion,
                    pointB: this.state.pointB_from_suggestion
                  })
                  
                 // if (this.pointA !== undefined &&
                   //   this.state.pointB_from_suggestion !== undefined)
                      this.buildDirections_navigator();

                  }.bind(this)
                }><Text style={{fontSize: 19, backgroundColor: '#9f89d1'}}>{this.state.addrB_suggestion}</Text></TouchableOpacity>
        )
      }*/

      // Here should be a navigtor to the to pickup point and to the drop off point
      return (
          <View style={styles.container}>
            <MapView provider={PROVIDER_GOOGLE} style={styles.mapStyle}
              initialRegion={{
                latitude: this.state.pointA.latitude,
                longitude: this.state.pointA.longitude,
                latitudeDelta: 0.01,
                longitudeDelta: 0.01
              }}

             
                showsBuildings={true}
                showsMyLocationButton={true}

                onRegionChangeComplete={region => {
                //  this.setState({
                  //  pointA: region, pointA_from_suggestion: region});
                  //console.log(`region=${JSON.stringify(region)}`);

                  this.setState({point_A_location: region});

                  // Everytime point A changes - notify the marketplace to get
                  // price/ETA
                  server_api_sdk.api_rider_set_A(region, this.on_message_received);

                  // When a user changes region we resolve new coordinate to address to
                  // show it as an A point
                  localtion_cache.resolve_coordinate_to_address(region, this.changeA_address_and_point);

                  if (this.state.page == "price_view")
                    this.buildDirections_navigator(region, this.state.point_B_location);
                }}

                ref={ref => { this.map = ref }}>

              <MapView.Marker pinColor='black' coordinate={this.state.pointB}>
                <View><FontAwesome5 name="flag-checkered" size={48} color="red"/></View>
              </MapView.Marker>
              <MapView.Polyline lineDashPattern={[25,3]}
                coordinates={this.state.coords}
	              strokeColor='#999999'
	              strokeWidth={10}/>
            </MapView>

             <View pointerEvents="none" style={{position: 'absolute', top: 0, bottom: 0, left: 0, right: 0, alignItems: 'center', justifyContent: 'center', backgroundColor: 'transparent'}}>
              <FontAwesome5 name="map-pin" size={48} color="#AF4035"/>
             </View>

            {render_change_address1}
            {render_change_address2}

          </View>
        )


  }

} // export class Navigator extends React.Component

/*

              <View style={{flex: 1, position: 'absolute', left : 10, right : 10, bottom: 20, alignItems: "center"}}>
                <View style={{flex: 0.4, flexDirection: 'row'}}>
                  <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: 15}}>{render_minutes_and_seconds}</Text>
                  <Text style={{fontFamily: 'roboto', textTransform: 'uppercase', fontWeight: 'bold', fontSize: 15}}>{render_distance_to_go}</Text>
                </View>
              </View>
*/

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


:
