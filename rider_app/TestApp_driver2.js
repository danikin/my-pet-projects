import React from 'react';
import MapView, { PROVIDER_GOOGLE } from 'react-native-maps';
import Marker from 'react-native-maps';
import { StyleSheet, Dimensions, FlatList, Text, View, TextInput, Button, Image } from 'react-native';

import { useState, useEffect } from 'react';
import {TouchableOpacity } from 'react-native';
import { Camera } from 'expo-camera';

import MapViewDirections from 'react-native-maps';



export default class DriverApp extends React.Component {



  constructor(props) {
    super(props);
    this.state = {
        isLoading : false,
        page : "best_order",

        camera_type: Camera.Constants.Type.back,
        driver_region : {latitude : 61.666594 + (Math.random()-.5)*0.05, longitude : 50.827201 + (Math.random()-.5)*0.05, latitudeDelta: 0.03, longitudeDelta: 0.03}
    }

    this.ws = new WebSocket('ws://185.241.194.113:80');

    this.ws.onopen = () => {
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

  }

  componentDidMount() {

  }


render_camera()
{
  return (
    <View style={{ flex: 1 }}>
      <Camera style={{ flex: 1 }} type={this.state.camera_type}

      ref={this.cameraRef}
  
      >
        <View
          style={{
            flex: 1,
            backgroundColor: 'transparent',
            flexDirection: 'row',
          }}>
          
          
          <TouchableOpacity
            style={{
              flex: 0.1,
              alignSelf: 'flex-end',
              alignItems: 'center',
            }}
            onPress={() => {
              this.setState({
                camera_type: this.state.camera_type === Camera.Constants.Type.back ?
                  Camera.Constants.Type.front : Camera.Constants.Type.back
            })
            }}>
            <Text style={{ fontSize: 18, marginBottom: 10, color: 'white' }}> Перевернуть </Text>
          </TouchableOpacity>


           <TouchableOpacity
            style={{
              flex: 0.1,
              alignSelf: 'flex-end',
              alignItems: 'center',
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
            <Text style={{ fontSize: 18, marginBottom: 10, color: 'white' }}> Сделать фотку </Text>
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

API_on_message(e)
{  
  // If rider is not created yet then create them
  if (this.API_driver_id === undefined)
        this.API_registerDriver();

  // Get current rider
  var driver = this._API_helper_get_driver_by_id(e, this.API_driver_id);
  if (driver == {})
    return;

  console.log(`API_on_message2: driver.lat=${driver.lat}, driver.lon=${driver.lon}`)

  // Get the best order
  var best_order = this._API_helper_get_unassined_order_by_id(e, driver.best_rider_id);

  console.log(`API_on_message2: this.API_driver_id=${this.API_driver_id}, driver.best_rider_id=${driver.best_rider_id},best_order.latA=${best_order.latA},best_order.lonA=${best_order.lonA}`)

  // If best_order's lon/lon changed then resolve them to addresses
  if (this.state.best_order_A === undefined || this.state.best_order_A.latitude != best_order.latA ||
      this.state.best_order_A.longitude != best_order.lonA)
  {
    var url = "http://185.241.194.113:1234/geo/get_houses_by_location?lat=" +
best_order.latA + "&lon=" + best_order.lonA + "&resolution=11&ring_size=1";

    console.log(url);

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
  }
  // Change the state from info from web socket
  // Note: a driver can be changed outside by the user of the test map and
  //  that change will be shown here
  this.setState({
        best_order_price: best_order.price,
        best_order_pickup_ETA: best_order.best_pickup_ETA,
        best_order_pickup_distance: best_order.best_pickup_distance,
        best_order_id: best_order.unassigned_order_id,
        best_order_A : {latitude: best_order.latA, longitude: best_order.lonA},
        best_order_B : {latitude: best_order.latB, longitude: best_order.lonB},

        driver_region : {latitude: driver.lat, longitude: driver.lon, latitudeDelta : this.state.driver_region.latitudeDelta, longitudeDelta: this.state.driver_region.longitudeDelta}
    })
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
        <View style={styles.container}>

        <Image style={{width: 300, height:500}} 
 source={[   {uri: this.state.photo} ]} />
      </View>
      )
    }
    else
    if (this.state.page == "best_order")
    {
    
      var render_polyline_driver_A =
      (this.state.driver_region === undefined || this.state.best_order_A === undefined) ? (<Text/>) :
       (<MapView.Polyline lineDashPattern={[25,25]}
coordinates={[
  {latitude: this.state.driver_region.latitude, longitude: this.state.driver_region.longitude},
  {latitude: this.state.best_order_A.latitude, longitude: this.state.best_order_A.longitude}
  
  ]}
	strokeColor='green'
	strokeWidth={1}/>);

      var render_polyline_A_B = 
      (this.state.best_order_A === undefined || this.state.best_order_B === undefined) ? (<Text/>) :
       (<MapView.Polyline lineDashPattern={[25,25]}
coordinates={[
  {latitude: this.state.best_order_A.latitude, longitude: this.state.best_order_A.longitude},
  {latitude: this.state.best_order_B.latitude, longitude: this.state.best_order_B.longitude}
  
  ]}
	strokeColor='black'
	strokeWidth={1}/>);

      return (
      <View style={styles.container}>

        <MapView provider={PROVIDER_GOOGLE} style={styles.mapStyle}
        
     initialRegion={{


            latitude: 61.666594,
            longitude: 50.827201,
            
            latitudeDelta: 0.03,
            longitudeDelta: 0.03

     }}

    region={this.state.driver_region}


          onMapReady={result => { 
        
        }
      }
          >

<MapView.Marker coordinate={this.state.driver_region}/>
<MapView.Marker pinColor='green' coordinate={this.state.best_order_A}/>
<MapView.Marker pinColor='black' coordinate={this.state.best_order_B}/>


{render_polyline_driver_A}
{render_polyline_A_B}

        </MapView>

      <Text
       style={{

position: 'absolute',
width: 281,
height: 23,
left: 35,
top: 61,

fontFamily: 'Roboto',
fontStyle: 'normal',
fontWeight: 900,
fontSize: '20px',
lineHeight: 23,
display: 'flex',
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
top: 220,

fontFamily: 'Roboto',
fontStyle: 'normal',
fontWeight: 900,
fontSize: '50px',
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

}}>{Math.round(this.state.best_order_price)} <Text style={{fontSize: '25px'}}>руб</Text></Text>

<Text style={{ position: 'absolute', left : 10, right : 10, top: 350, height: 40,
        fontSize: '19px', fontWeight: 'bold'}}>Подача {Math.round(this.state.best_order_pickup_ETA)} минут, {Math.round(this.state.best_order_pickup_distance)}км</Text>

      </View>
      );
    }
    else
    {
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


