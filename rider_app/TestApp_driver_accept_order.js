import React from 'react';
import MapView, { PROVIDER_GOOGLE } from 'react-native-maps';
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

const registration_pic_names = ['Ваш протрет', 'Главная страница паспорта', 'Прописка в паспорте', 'Права', 'Обратная сторона прав', 'СТС', 'Обратная сторона СТС', 'Чистая машина с номером'];

export default class DriverApp extends React.Component {



  constructor(props) {
    super(props);
    this.state = {
        isLoading : false,
        page : "best_order",

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

  }

  componentDidMount() {

  }

directionsQueryInProgressCounter = [0, 0];
directionsQueryInProgressTimers = [[], []];

counterOfGetDirectionCalls = [0, 0];

getDirections(startLoc, destinationLoc, number)
{

  console.log(`Inside getDirections: this.directionsQueryInProgressCounter[${number}]=${this.directionsQueryInProgressCounter[number]}, this.counterOfGetDirectionCalls[${number}]=${this.counterOfGetDirectionCalls[number]}`);

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

        //console.log(`ENCODED: ${respJson.trips[0].geometry}`);

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

buildDirections()
{
  this.getDirections({latitude: this.driver.lat, longitude: this.driver.lon}, {latitude: this.best_order.latA, longitude: this.best_order.lonA}, 0)
  this.getDirections({latitude: this.best_order.latA, longitude: this.best_order.lonA}, {latitude: this.best_order.latB, longitude: this.best_order.lonB}, 1)
}


API_on_message(e)
{  
  // If the driver is not created yet then create them
  if (this.API_driver_id === undefined)
        this.API_registerDriver();

  // Get current driver (it may not exist yet because the registraion above could be not finished yet)
  this.driver = this._API_helper_get_driver_by_id(e, this.API_driver_id);
  if (this.driver.driver_id != this.API_driver_id)
    return;

  //console.log(`API_on_message3: driver=${JSON.stringify(driver)}`)

  // Get the best order
  if (this.driver.best_rider_id !== undefined)
    this.best_order = this._API_helper_get_unassined_order_by_id(e, this.driver.best_rider_id);

  // If this driver is assigned then save his/her assigned order id
  this.assigned_order_id = this.driver.assigned_order_id;

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

  console.log(`best_order_id = ${this.state.best_order_id}`)

  // Build all needed directions and not to often
  this.buildDirections();

  // Don't call the address resolving to frequently
  var now = Date.now();
  if (!(this.directions_ts === undefined) && now - this.directions_ts < 1000)
  {
    console.log("SKIP DIRECTIONS AND ADDR RESOLVING IN on_message");
    return;
  }

  console.log("now2=" + now + ", diff2=" + (now - this.directions_ts));

  this.directions_ts = now;

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
  }

}

/*

tion: 'absolute',
top: 55,
height: 30,
left: 10,
right: 10,

fontFamily: 'Roboto',
fontStyle: 'bold',
fontWeight: 900,
fontSize: '16px',
alignItems: 'center',
letterSpacing: '0.04em',
textTransform: 'uppercase',

color: '#000000',

textShadowColor: 'rgba(0, 0, 0, 0.25)',
textShadowOffset: {width: 0, height: 4},
textShadowRadius: 4

       }}

*/

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

      // If there is no best order then this means that there are no orders at all
      if (this.state.best_order_id === undefined ||
          this.state.best_order_id == 0 ||
          this.state.best_order_id == -1)
          return (<View style={styles.container}>
              <MapView style={styles.mapStyle} provider={PROVIDER_GOOGLE} initialRegion={{
                latitude: 61.666594,
                longitude: 50.827201,
                latitudeDelta: 0.03,
                longitudeDelta: 0.03
              }} region={this.state.driver_region}>
                
              </MapView>

            <Text
       style={{fontFamily: 'Roboto', fontSize: '15px', fontWeight: 'bold', lineHeight: 23,
display: 'flex',
alignItems: 'center',
letterSpacing: '0.04em',
textTransform: 'uppercase',
color: '#000000',
textShadowColor: 'rgba(0, 0, 0, 0.25)',
textShadowOffset: {width: 0, height: 4}, textShadowRadius: 4
       }}
      >Ищем лучший заказ для вас ...</Text></View>
          );
      // If there is a best order then show it the driver
      else
        return (
      <View style={styles.container}>

        <MapView provider={PROVIDER_GOOGLE} style={styles.mapStyle}
        
showsBuildings
          ref={ref => { this.map = ref }}
          onLayout={() => {
            this.map.animateToBearing(125);
            this.map.animateToViewingAngle(45);
          }}


     initialRegion={{


            latitude: 61.666594,
            longitude: 50.827201,
            
            latitudeDelta: 0.03,
            longitudeDelta: 0.03

     }}

    region={this.state.driver_region}


          >

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


  <Text style={{ position: 'absolute', left : 10, right : 10, bottom: 80, height: 40, borderColor: 'gray', backgroundColor: 'green',
        borderWidth: 3, fontSize: '19px', fontWeight: 'bold'}}
        
        onPress={this.API_accept_best_order()}

        >Принять заказ</Text>

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


