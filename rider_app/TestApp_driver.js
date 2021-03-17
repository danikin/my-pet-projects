import React from 'react';
import MapView, { PROVIDER_GOOGLE } from 'react-native-maps';
import Marker from 'react-native-maps';
import { StyleSheet, Dimensions, FlatList, ActivityIndicator, Text, View, TextInput, Button, Image } from 'react-native';

import { useState, useEffect } from 'react';
import {TouchableOpacity } from 'react-native';
import { Camera } from 'expo-camera';

import MapViewDirections from 'react-native-maps';




export default class DriverApp extends React.Component {



  constructor(props) {
    super(props);
    this.state = {
        isLoading: false,
        page : "best_order",
        region2: {latitude: 61.666594, longitude: 50.827201},
        camera_type: Camera.Constants.Type.back
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

/*CameraApp()
{

  const [hasPermission, setHasPermission] = useState(null);
  {this.type, this.setType} = useState(Camera.Constants.Type.back);

useEffect(() => {
    (async () => {
      const { status } = await Camera.requestPermissionsAsync();
      setHasPermission(status === 'granted');
    })();
  }, []);

  if (hasPermission === null) {
    return <View />;
  }
  if (hasPermission === false) {
    return <Text>No access to camera</Text>;
  }
}*/


//takePictureAsync

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
  // If rider is not created yet then don't do anything
  if (this.API_driver_id == undefined)
    return;
  // Get current rider
  var driver = this._API_helper_get_driver_by_id(e, this.API_driver_id);
  if (driver == {})
    return;

  // Get the best order
  var best_order = this._API_helper_get_unassined_order_by_id(e, driver.best_rider_id);

  // Change the state from info from web socket
  this.setState({
        best_order_price: best_order.price,
        best_order_pickup_ETA: best_order.best_pickup_ETA,
        best_order_pickup_distance: best_order.best_pickup_distance,
        best_order_id: best_order.unassigned_order_id,
        best_order_A : {latitude: best_order.latA, longitude: best_order.lonA},
        best_order_B : {latitude: best_order.latB, longitude: best_order.lonB},
    })
}

  render()
  {

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
      return (
      <View style={styles.container}>

        <MapView provider={PROVIDER_GOOGLE} style={styles.mapStyle} initialRegion={{
            latitude: 61.666594,
            longitude: 50.827201,
            latitudeDelta: 0.01,
            longitudeDelta: 0.01
          }}>
        </MapView>
        <Text style={{ position: 'absolute', top: 30, fontSize: '15px'}}>Откуда</Text>
        <Text style={{ position: 'absolute', top: 380, fontSize: '15px'}}>Куда</Text>
      </View>
      );
    }
    else
    {
      return (
        <View/>
      )
    }


    /*
       source={{
          uri: 'https://reactnative.dev/img/tiny_logo.png',
        }}/>
 */



/*

source={this.state.photo}

{{
          uri: 'https://reactnative.dev/img/tiny_logo.png',
        }}
*/

/*

    if (this.state.isLoading) {
      return (
        <View style={{ flex: 1, padding: 20, separator: {
    marginVertical: 8,
    borderBottomColor: '#737373',
    borderBottomWidth: 1,
  } }}>
          <ActivityIndicator />
        </View>
      );
    }

    // Main screen with the best order
    if (this.state.page == "map")
    {
     
      return (
      <View style={styles.container}>

        <MapView provider={PROVIDER_GOOGLE} style={styles.mapStyle} initialRegion={{
            latitude: 61.666594,
            longitude: 50.827201,
            latitudeDelta: 0.01,
            longitudeDelta: 0.01
          }}>
        </MapView>
        <Text style={{ position: 'absolute', top: 30, fontSize: '15px'}}>Откуда</Text>
        <Text style={{ position: 'absolute', top: 380, fontSize: '15px'}}>Куда</Text>
      </View>
      );
    } // if (this.state.page == "map")


    */
  } // render()

}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#fff',
    alignItems: 'center',
    justifyContent: 'center',

  },
  mapStyle: {
    position: 'absolute',
    width: Dimensions.get('window').width,
    height: Dimensions.get('window').height,
  },
});


