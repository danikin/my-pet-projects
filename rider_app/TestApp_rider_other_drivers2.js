import React from 'react';
import MapView, { PROVIDER_GOOGLE } from 'react-native-maps';
import Marker from 'react-native-maps';
import { StyleSheet, Dimensions, FlatList, ActivityIndicator, Text, View, TextInput, Button } from 'react-native';

//import MapViewDirections from './components/MapViewDirections';


import MapViewDirections from 'react-native-maps';

import flagPinkImg from './assets/flag-pink.png';

const token = 'MKYMO5VUWLWDAC1OZE0V1EAUVEKBKBE6';
const flow_id = 484014719;

//const GOOGLE_MAPS_APIKEY = 'AIzaSyAvB2OCDGIxauZ2KbnWqcsrKOkM76_-ixg';

export default class FetchExample extends React.Component {



  constructor(props) {
    super(props);
    this.state = {
        isLoading: false,
        page : "map",
        token : "",

        // Address that is in the B field
        addressInB  : "",
        // Address that is being suggested for B to chose from
        addressInSuggestedB : "",

        addrBPosition: 410,
        addrSuggestedBPosition: 460,
        order_price: -1.0,
        order_pickup_ETA: -1.0,
        order_pickup_distance: -1.0,
        order_best_driver_id: -1,
        order_suggested_price: -1.0,
        rider_to_driver_route: [],
        driver_markers: [],
        is_order_placed: false,

        region2: {latitude: 61.666594, longitude: 50.827201}
    };
    this.enteredText = "";
    this.phoneNumber = "";
    this.onChangeText = this.onChangeText.bind(this);
    this.sendSMSCode = this.sendSMSCode.bind(this);
    this.enterSMSCode = this.enterSMSCode.bind(this);
    this.onRegionChange = this.onRegionChange.bind(this);
    this.onChangeAddrB = this.onChangeAddrB.bind(this);
    this.autocompleteB = this.autocompleteB.bind(this); 
    this.onFocusAddrB = this.onFocusAddrB.bind(this);
    this.onBlurAddrB = this.onBlurAddrB.bind(this);

    // Does B field contain a full address?
    this.isFullAddressInB = false;
    // Does suggestion of B contain a full address?
    this.isFullAddressInSuggestedB = false;

    // Does B field contain at least a street?
    this.isAtLeastStreetInB = false;
    // Does suggestion of B contain at least a street?
    this.isAtLeastStreetInSuggestedB = false;

    // The most recent real street name received from the server in suggested B
    this.recentStreetNameInSuggestedB = "";

    this.AddrBInputRef = React.createRef();
    this.PinRef = React.createRef();

    this.ws = new WebSocket('ws://185.241.194.113:80');

    this.ws.onopen = () => {
      //this.ws.send('{"event":"auth_by_token", "data":{"token":"' + token + '"}}'); 
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
      //console.log(e.message);

      console.log('Socket encountered error: ', e.message, 'Closing socket');
      this.ws.close();
    };
    
}


  componentDidMount() {

  }


/*
    Abstract away all communication to a server. API is subject to change
*/

API_rider_id = undefined;

// Moves A point to specified coordinates
API_moveA(latitude, longitude)
{
  //console.log(latitude, longitude);
  if (this.API_rider_id != undefined)
    this.ws.send(`{"event":"test_update_price_view_A", "data":{"rider_id":${this.API_rider_id},"lat":${latitude},"lon":${longitude}}}`);
}

// Moves B point to specified coordinates
API_moveB(latitude, longitude)
{
  //console.log("API_moveB: this.API_rider_id=" + this.API_rider_id);

  //console.log(latitude, longitude);

  // If B is specified and there is no rider then create it
  if (this.API_rider_id == undefined)
  {
    this.API_rider_id = Math.round(Math.random() * 1000000000);
    this.ws.send(`{"event":"test_price_view", "data":{"id":${this.API_rider_id },"latA":${this.latA},"lonA":${this.lonA},"latB":${this.latB},"lonB":${this.lonB}}}`)
  }

  // Update B with the rider id that we have now
  this.ws.send(`{"event":"test_update_price_view_B", "data":{"rider_id":${this.API_rider_id},"lat":${latitude},"lon":${longitude}}}`);

    
//console.log("API_moveB: this.API_rider_id = " + this.API_rider_id);
}

// Places an order for current rider
API_place_order()
{
  if (this.API_rider_id != undefined)
    this.ws.send(`{"event":"test_order", "data":{"rider_id":${this.API_rider_id}}}`);
}

// A rider accepts suggested price after an order having been placed
API_accept_suggested_price(new_price)
{
  console.log(`API_accept_suggested_price(${new_price}`)

  if (this.API_rider_id != undefined)
    this.ws.send(`{"event":"test_accept_price_change","data":{"rider_id":${this.API_rider_id},"new_price":${new_price}}}`);
}

// Returns current rider if there is any
_API_helper_get_rider(e)
{
  
//console.log("this.API_rider_id = " + this.API_rider_id);


    if (this.API_rider_id == undefined)
      return {};

    for(let rider of e.data.riders)
    {
      if (rider.rider_id == this.API_rider_id)
        return rider;
    }
    return {};
}

_API_helper_get_driver_by_id(e, driver_id)
{
    for(let driver of e.data.drivers)
    {
      if (driver.driver_id == driver_id)
        return driver;
    }
    return {};
}

API_on_message(e)
{
  // If rider is not created yet then don't do anything
  if (this.API_rider_id == undefined)
    return;

  /*console.log("API_on_message2: e.event: " + e.event + ", " 
      + e.data + " e.data.drivers=" + e.data.drivers[0].driver_id + " e.data.riders=" + e.data.riders[0].rider_id +
      ", API_rider_id=" + this.API_rider_id );*/


  // Get current rider
  var rider = this._API_helper_get_rider(e);

 // console.log("API_on_message: rider=" + rider)

  if (rider == {})
    return;



  // Get the best driver
  var best_driver = this._API_helper_get_driver_by_id(e, rider.best_driver_id);

  console.log(`API_on_message2: best_driver: latitude: ${best_driver.lat}, longitude: ${best_driver.lon}`)


  /*this.latA = rider.latA;
  this.lonA = rider.lonA;
  this.latB = rider.latB;
  this.lonB = rider.lonB;*/

  // Change the state from info from web socket
  this.setState({
        order_price: rider.price,
        order_pickup_ETA: rider.best_pickup_ETA,
        order_pickup_distance: rider.best_pickup_distance,
        order_best_driver_id: rider.best_driver_id,
        order_suggested_price: rider.suggested_price,
        bestDriver : {latitude: best_driver.lat, longitude: best_driver.lon},
        rider_to_driver_route: [
          {latitude: this.latA, longitude: this.lonA},
          {latitude: best_driver.lat, longitude: best_driver.lon}
        ]
    })

 // console.log(`API_on_message2: state.bestDriver: latitude: ${this.state.bestDriver.latitude}, longitude: ${this.state.bestDriver.longitude}`)

    // Iterate all drivers except the best to show markers
    for (let driver of e.data.drivers)
    {
      if (driver.driver_id != best_driver.driver_id)
        this.setState(prevState => ({
  driver_markers: [...prevState.driver_markers, {latitude: driver.lat, longitude: driver.lon}]
}))
    }

}


onChangeText(s)
{
  this.enteredText = s;
}

findHouse(address)
{
    fetch("http://185.241.194.113:1234/geo/find_house?s=" + address)
    .then(response => response.json())
  .then(responseJson => {
      this.setState(
          {
            // Now save the full address with the house
            addressInSuggestedB: responseJson[0].full_address,
          }
      )
      this.isFullAddressInSuggestedB = true;
      this.isAtLeastStreetInSuggestedB = true;
      this.latB = responseJson[0].house_latitude;
      this.lonB = responseJson[0].house_longitude;

      /*
          TODO: a) combine two HTTP calls in one, b) suppor tokens and flow ids
      */

     // fetch("http://185.241.194.113:1234/rider/move_B?token=" + token + "&flow_id=" + flow_id + "&lat=" + this.latB + "&lon=" + this.lonB);

      this.API_moveB(this.latB, this.lonB);
  })
}

onFocusAddrB()
{
  this.setState({
      addrBPosition : this.state.addrBPosition - 200,
      addrSuggestedBPosition : this.state.addrSuggestedBPosition - 200
  })

  // If address B gets or loses focus and the street is there then try to find
  // a house suggestion
  if (this.isAtLeastStreetInB)
    this.findHouse(this.state.addressInB);
}

onBlurAddrB()
{
  this.setState({
      addrBPosition : this.state.addrBPosition + 200,
      addrSuggestedBPosition : this.state.addrSuggestedBPosition + 200
  })

  // If address B gets or loses focus and the street is there then try to find
  // a house suggestion
  if (this.isAtLeastStreetInB)
    this.findHouse(this.state.addressInB);
}

/*
 
 Hook for changing the address of B
 
*/
onChangeAddrB(newAddressInB)
{
    // If the new value of B is EXACT SAME of the value of the suggestion then
    // it has a street or a full address EXACT as the suggestion has
    if (newAddressInB === this.state.addressInSuggestedB)
    {
      this.isFullAddressInB = this.isFullAddressInSuggestedB;
      this.isAtLeastStreetInB = this.isAtLeastStreetInSuggestedB;
    }
    else
    // If the new value of B EXACTLY starts with the one from the suggestion and IF
    // the suggestion contains at least street then we can be sure that B contains
    // at least streeat
    if (newAddressInB.startsWith(this.state.addressInSuggestedB) && this.isAtLeastStreetInSuggestedB)
    {
      this.isAtLeastStreetInB = true;
    }
    else
    // Same thing is above if B starts EXACTLY with the recent real string gotten from the server
    if (newAddressInB.startsWith(this.recentStreetNameInSuggestedB))
    {
      this.isAtLeastStreetInB = true;
    }
    else
    // Otherwise we don't believe that B contains anything meaningful like a street and a full
    // address
    {
      this.isFullAddressInB = false;
      this.isAtLeastStreetInB = false;
    }

    // Use entered space as a request for auto completion
    //  Note: check if that's really entered space and not just space at the end of the
    //  string because of the backspace usage :-)
    if (this.state.addressInB &&
        newAddressInB.length > this.state.addressInB.length &&
        newAddressInB.endsWith(' '))
    {
      this.autocompleteB();
    }

    // Synchronize put in text and the state
    this.setState(
          {
            addressInB: newAddressInB
          }
      )



  // If street name is already there then search for a house
  //  Note: newAddressInB can contain not necessarily a bare sreet name it also
  //  can contain something beyond - but that's OK - we want to autocomplete it to the house
  //  Note: it will not change the B field - change only the suggestion
  if (this.isAtLeastStreetInB && this.isAtLeastStreetInSuggestedB)
    this.findHouse(newAddressInB);
  // If street name is not already there then search for a street
  else
  {
    fetch("http://185.241.194.113:1234/geo/find_street?s=" + newAddressInB)
      .then(response => response.json())
      .then(responseJson => {
      // Found a street - save it the the suggestion (it's just a street, not a full address)
      this.setState(
          {
            addressInSuggestedB: responseJson[0][1],
          }
      )
      this.isAtLeastStreetInSuggestedB = true; // Street name is there
      this.isFullAddressInSuggestedB = false; // Full address is not (TODO: handle POI! They will always have a full address)
      this.recentStreetNameInSuggestedB = responseJson[0][1];
  })
}
}

/*

  Hook: a user selects an address from the suggestion

*/

autocompleteB()
{
  // If the full address is there then the button "Request a Ride"
  // will be shown so lose the focus for the input of B to hide keyboard
  if (this.isFullAddressInSuggestedB &&
        this.state.addressInB === this.state.addressInSuggestedB)
  {
    this.AddrBInputRef.current.blur();
  }

  // Syncrhronize B address and suggested B address including the flag of "fullness" of an address
  this.setState(
            {
              addressInB: this.state.addressInSuggestedB,
            })
  this.isAtLeastStreetInB = this.isAtLeastStreetInSuggestedB;
  this.isFullAddressInB = this.isFullAddressInSuggestedB;
}

sendSMSCode()
{
  this.phoneNumber = this.enteredText;
  fetch ("http://185.241.194.113:1234/auth/sendsms?phone=" + this.phoneNumber)

  this.setState(
          {
            page: "enter_sms",
          })
}

enterSMSCode()
{
  var url = "http://185.241.194.113:1234/auth/by_smscode?phone="

  +this.phoneNumber+"&smscode="+
  
  this.enteredText;

  console.log(url);

  // Get and save the token
  fetch (url)
  .then(response => response.json())
      .then(responseJson => {
        this.setState(
          {
            token : responseJson.data.token
          },
          function() {
            console.log(this.state.token)

            // Register a rider
            fetch ("http://185.241.194.113:1234/rider/reg?token=" + this.state.token)

            // Go to the map
            this.setState(
            {
              page: "map",
            })
          }
        );
      })



  // Now show rider a map!!!
  // TODO
}

/*

  Hook: a user moves Pin

*/


onRegionChange(region)
{
  //console.log(region.latitude, region.longitude)

/*

  TODO: a) combine two requests in one, b) support tokens and flowd ids

*/

  // Make all local changes

  this.latA = region.latitude;
  this.lonA = region.longitude;
  this.setState({ region2 : {latitude: region.latitude, longitude: region.longitude} });

//if (this.state.bestDriver != undefined)
  //console.log(`onRegionChange2: ${this.state.bestDriver.latitude}, ${this.state.bestDriver.longitude}`)

  if (this.state.bestDriver != undefined)
    this.setState({
        rider_to_driver_route: [
          {latitude: this.latA, longitude: this.lonA},
          {latitude: this.state.bestDriver.latitude, longitude: this.state.bestDriver.longitude}
        ]
    })

  // Make all changes with server involved once a second

  var now = Date.now();
  if (now - this.onRegionChange_ts < 1000)
    return;
  //console.log("now=" + now + ", diff=" + (now - this.onRegionChange_ts));
  this.onRegionChange_ts = now;


fetch("http://185.241.194.113:1234/geo/get_houses_by_location?lat=" +
region.latitude + "&lon=" + region.longitude + "&resolution=11&ring_size=1")

.then(response => response.json())
      .then(responseJson => {

        this.setState(
          {
            addr_A : responseJson.data.nearest_house
          })
      }
      );

  this.API_moveA(this.latA, this.lonA);
}


  num_directions = 0;

  directionsReady(distance, duration, coordinates, fare, waypointOrder)
  {
    if (this.num_directions++ > 3)
      return;

    console.log(`directionsReady: ${distance}, ${duration}, ${coordinates}, ${fare}, ${waypointOrder}`)
  }

  directinsError(err)
  {
    console.log(`directionsError(${err})`)
  }

  render() {
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

    if (this.state.page == "enter_sms")
    {
          return (
      <View style={{ flex: 1, paddingTop: 20 }}>
        <Text >Введите код из смс</Text>
        <TextInput onChangeText={this.onChangeText}
         style={{ height: 40, borderColor: 'gray', borderWidth: 1 }}/>
        <Button onPress={this.enterSMSCode} title="Подтвердить код" color="#841584"/>
      </View>
    );
    }



    if (this.state.page == "map")
    {
      // Add Button for ordering if B is the real full address from the server
      if (this.state.addressInB === this.state.addressInSuggestedB &&
            this.isFullAddressInB)
      {

        // If order is placed then no need in the order taxi button
        if (this.state.is_order_placed)
          this.order_taxi = ( <Text></Text>  );
        // Otherwise render the order taxi button
        else
          this.order_taxi = (<Text color="black"
        
        title="Заказать такси" 
        style={{ position: 'absolute', top: 500, borderColor: 'black', borderWidth: 2, backgroundColor: 'yellow', color: 'black',
        fontSize: '30px'}}
        onPress={() => {
          this.API_place_order()
          this.setState({is_order_placed : true})
        }}
        
        >Заказать такси</Text>);

        // A line from the rider to the driver - to show how to decrease the price and distance
        this.polyline = (
<MapView.Polyline
            
            coordinates={this.state.rider_to_driver_route}
		

		strokeColor="#000" // fallback for when `strokeColors` is not supported by the map-provider
		strokeColors={[
			'#7F0000',
			'#00000000', // no color, creates a "long" gradient between the previous and next coordinate
			'#B24112',
			'#E5845C',
			'#238C23',
			'#7F0000'
		]}
		strokeWidth={6}
            
            />
        )





      // Information about the price and the distance
      if (this.state.is_order_placed)
      {
        this.price_and_distance = (

        <Text style={{ position: 'absolute', top: 150, borderWidth: 0, fontSize: '19px', fontWeight: 'bold'}}>Цена {Math.round(this.state.order_price)} Руб{"\n"}Подача {Math.round(this.state.order_pickup_ETA)} минут{"\n"}{"\n"}Ищем такси ...</Text>
      
        )

        // Suggestion about a bigger price
        if (this.state.order_suggested_price > this.state.order_price)
        {
          this.render_suggested_price = (
            <Text color="black"
        
        style={{ position: 'absolute', top: 200, borderColor: 'black', borderWidth: 2, backgroundColor: 'green', color: 'black',
        fontSize: '25px'}}
        onPress={() => {
          this.API_accept_suggested_price(this.state.order_suggested_price)
        }}
        
        >Согласны ехать за {Math.round(this.state.order_suggested_price)} Руб?</Text>
          )
        }
        else
          this.render_suggested_price = (<Text></Text>)
      }
      else
       this.price_and_distance = (

        <Text style={{ position: 'absolute', top: 150, borderWidth: 0, fontSize: '19px', fontWeight: 'bold'}}>Цена {Math.round(this.state.order_price)} Руб{"\n"}Подача {Math.round(this.state.order_pickup_ETA)} минут{"\n"}Хотите дешевле и быстрее?{"\n"}Идите по красной стрелке</Text>
      
      )


      }
      else 
        this.order_taxi = ( <Text></Text>  );

/*
             <MapViewDirections
  						origin={this.state.region2}
  						destination={this.state.bestDriver}
  						apikey={GOOGLE_MAPS_APIKEY}
              onReady={this.directionsReady}
  					/>
*/
/*
 <MapViewDirections
  						origin={this.state.region2}
  						destination={this.state.bestDriver}
  						apikey={GOOGLE_MAPS_APIKEY}
  						strokeWidth={3}
  						strokeColor="hotpink"
              onReady={this.directionsReady}
  					/>
*/


      /*if (this.state.region2 != undefined && this.state.bestDriver != undefined)
      {
        this.directions = (
          <Text style={{ position: 'absolute', top: 30}}>Маршрут</Text> 
         
         <MapViewDirections
  						origin={this.state.region2}
  						destination={this.state.bestDriver}
  						apikey={GOOGLE_MAPS_APIKEY}
  						strokeWidth={3}
  						strokeColor="hotpink"
              onReady={this.directionsReady}
  					/>
        )

        console.log(`DIRECTIIONS: ${this.directions}`);
    }*/
/*
  						onReady={this.onReady}
  						onError={this.onError}
              */
/*
    <MapViewDirections
  						origin={this.state.region2}
  						destination={this.state.bestDriver}
  						apikey={GOOGLE_MAPS_APIKEY}
  						strokeWidth={3}
  						strokeColor="hotpink"
              onReady={this.directionsReady}
              onError={this.directinsError}
  					/>
*/
      return (
      <View style={styles.container}>

        <MapView provider={PROVIDER_GOOGLE} style={styles.mapStyle} initialRegion={{
            latitude: 61.666594,
            longitude: 50.827201,
            latitudeDelta: 0.01,
            longitudeDelta: 0.01
          }}

          region={this.state.region}
      onRegionChange={this.onRegionChange}
      onMapReady={result => { 
        this.onRegionChange({latitude: 61.666594, longitude: 50.827201})
        }
      }
      
          
          >

            
            <MapView.Marker coordinate={this.state.region2}
            /> 

            
            <MapView.Marker pinColor='green' coordinate={this.state.bestDriver}/>

            {this.polyline}

{this.state.driver_markers.map
((marker) => (
    <MapView.Marker
      coordinate={{longitude: marker.longitude, latitude: marker.latitude}}
      title={marker.description}
      pinColor='yellow'
      description={marker.description}>
    </MapView.Marker>
  ))}



     
          </MapView>
      <Text style={{ position: 'absolute', top: 30}}>Откуда</Text>

        {this.price_and_distance}
        {this.render_suggested_price}

      <Text style={{ position: 'absolute', top: 380}}>Куда</Text>
      <TextInput style={{ position: 'absolute', left : 10, right : 10, top: 50, height: 40, borderColor: 'gray', 
        borderWidth: 1, fontSize: '19px', fontWeight: 'bold'}} value={ this.state.addr_A}/>


      <TextInput selectTextOnFocus={true} 
      
      onChangeText={this.onChangeAddrB}
      onBlur={this.onBlurAddrB}
      onFocus={this.onFocusAddrB}

      ref={this.AddrBInputRef}

      style={{ position: 'absolute', left : 10, right : 10, top: this.state.addrBPosition, height: 40, borderColor: 'gray',  borderWidth: 1, fontSize: '19px', fontWeight: 'bold'}}

        value={this.state.addressInB}
        
        />

        <Text style={{ position: 'absolute', top: this.state.addrSuggestedBPosition, backgroundColor: 'green',
        
        borderWidth: 2, color: 'black',
        fontSize: '19px'
        
        }}
        onPress={this.autocompleteB}
        >{this.state.addressInSuggestedB}</Text>

         {this.order_taxi}
        

    </View>
    );

      

    }

    


    return (
      <View style={{ flex: 1, paddingTop: 20 }}>
        <Text >Введите номер телефона</Text>
        <TextInput onChangeText={this.onChangeText}
         style={{ height: 40, borderColor: 'gray', borderWidth: 1 }}/>
        <Button onPress={this.sendSMSCode} title="Отправить код на телефон" color="#841584"/>
      </View>
    );
  }
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



