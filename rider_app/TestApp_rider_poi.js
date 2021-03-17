import React from 'react';
import MapView, { PROVIDER_GOOGLE } from 'react-native-maps';
import Marker from 'react-native-maps';
import { StyleSheet, Dimensions, FlatList, ActivityIndicator, Text, View, TextInput, Button } from 'react-native';

//import MapViewDirections from './components/MapViewDirections';

import MapViewDirections from 'react-native-maps';
import DriverApp from './DriverApp.js';

//import flagPinkImg from './assets/flag-pink.png';

const token = 'MKYMO5VUWLWDAC1OZE0V1EAUVEKBKBE6';
const flow_id = 484014719;

//const GOOGLE_MAPS_APIKEY = 'AIzaSyAvB2OCDGIxauZ2KbnWqcsrKOkM76_-ixg';

//export default DriverApp;

export default class RiderApp extends React.Component {



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
        A_to_B_route: [],
        A_to_best_price_route: [],
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

  // Get current rider
  var rider = this._API_helper_get_rider(e);

  if (rider == {})
    return;
  // Get the best driver
  var best_driver = this._API_helper_get_driver_by_id(e, rider.best_driver_id);


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
        ],
        A_to_B_route: [
          {latitude: this.latA, longitude: this.lonA},
          {latitude: this.latB, longitude: this.lonB}
        ]
    })

  // Update coordinates for the best price point
  // Note: this MUST be done after previous setState because this function
  //  leverages bestDriver state under the hood. And this state is set above
  this.updatBestPriceCoordinate();

    if (this.state.bestPrice_coordinate != undefined)
      this.setState({
         A_to_best_price_route: [
          {latitude: this.latA, longitude: this.lonA},
          {latitude: this.state.bestPrice_coordinate.latitude, longitude: this.state.bestPrice_coordinate.longitude}
        ]
    })

    // Iterate all drivers except the best to show markers
    this.setState ({ driver_markers: [] })
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
            B_coordinate : {latitude: responseJson[0].house_latitude, longitude: responseJson[0].house_longitude}
          }
      )
      this.isFullAddressInSuggestedB = true;
      this.isAtLeastStreetInSuggestedB = true;
      this.latB = responseJson[0].house_latitude;
      this.lonB = responseJson[0].house_longitude;

      /*
          TODO: a) combine two HTTP calls in one, b) suppor tokens and flow ids
      */

      this.API_moveB(this.latB, this.lonB);
  })
}

onFocusAddrB()
{
  this.setState({
      addrBPosition : this.state.addrBPosition - 200,
      addrSuggestedBPosition : this.state.addrSuggestedBPosition - 200,
      addrB_has_focus : 1
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
      addrSuggestedBPosition : this.state.addrSuggestedBPosition + 200,
      addrB_has_focus : 0
  })

  // If address B gets or loses focus and the street is there then try to find
  // a house suggestion
  if (this.isAtLeastStreetInB)
    this.findHouse(this.state.addressInB);

    // If there is not full address in B then retain focus
  if (!this.isFullAddressInB)
    this.AddrBInputRef.current.focus();
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
    // Note: but still there is no full address in B because otherwise
    //  the condition above would be true
    if (newAddressInB.startsWith(this.state.addressInSuggestedB) && this.isAtLeastStreetInSuggestedB)
    {
      this.isFullAddressInB = false;
      this.isAtLeastStreetInB = true;
    }
    else
    // Same thing is above if B starts EXACTLY with the recent real string gotten from the server
    if (newAddressInB.startsWith(this.recentStreetNameInSuggestedB))
    {
      this.isFullAddressInB = false;
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

//console.log(`this.isAtLeastStreetInB=${this.isAtLeastStreetInB}, this.isAtLeastStreetInSuggestedB=${this.isAtLeastStreetInSuggestedB}`);


  // If street name is already there then search for a house
  //  Note: newAddressInB can contain not necessarily a bare sreet name it also
  //  can contain something beyond - but that's OK - we want to autocomplete it to the house
  //  Note: it will not change the B field - change only the suggestion
  if (this.isAtLeastStreetInB && this.isAtLeastStreetInSuggestedB)
    this.findHouse(newAddressInB);
  // If street name is not already there then search for a street
  else
  {
    //console.log(`before find_street: ${newAddressInB}`);

    fetch("http://185.241.194.113:1234/geo/find_street?s=" + newAddressInB)
      .then(response => response.json())
      .then(responseJson => {


    //console.log(`2responseJson`);

        // Found a street - save it the the suggestion (it's just a street, not a full address)
        if (!(responseJson.data.length === 0))
        {
         // console.log(`3responseJson`);
          this.setState({ addressInSuggestedB: responseJson.data[0][1]})
   
          this.isAtLeastStreetInSuggestedB = true; // Street name is there
          this.isFullAddressInSuggestedB = false; // Full address is not
          this.recentStreetNameInSuggestedB = responseJson.data[0][1];
        }
        else
        {
          //console.log(`2search for poi for ${newAddressInB}`);

          // If nothing is found then search for POIs
          // Note: if we search for POIs before we got empty result from streets then
          //  we will endup with endless suggestion about found poi by it's street address
          // Note: once POI found we jump directly to the full address state

          fetch("http://185.241.194.113:1234/geo/find_house?poi=1&s=" + newAddressInB)
            .then(response => response.json())
            .then(responseJson => {

//console.log(`3search for poi for ${newAddressInB}`);


//console.log(`4search for poi responseJson[0].poi_name=${responseJson[0].poi_name},responseJson[0].full_address=${responseJson[0].full_address}`);

              // If POI is found
              if (!(responseJson.length === 0))
              {

               // console.log(`5search for poi responseJson[0].poi_name=${responseJson[0].poi_name},responseJson[0].full_address=${responseJson[0].full_address}`);

                // Combine the poi full name
                var poi_full_name = responseJson[0].poi_name + " " + responseJson[0].full_address;
                
                // It's a full house now :-)
                this.isAtLeastStreetInSuggestedB = true;
                this.isFullAddressInSuggestedB = true;
                this.recentStreetNameInSuggestedB = poi_full_name;

                // Save everything we need to save for the full house state
                this.setState({
                  addressInSuggestedB : poi_full_name,
                  B_coordinate : {latitude: responseJson[0].house_latitude, longitude: responseJson[0].house_longitude}
                  })
                this.latB = responseJson[0].house_latitude;
                this.lonB = responseJson[0].house_longitude;
                this.API_moveB(this.latB, this.lonB);
              }
            })
        }
      })
    }
} // onChangeAddrB(newAddressInB)

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

// Updates coordinate of the best price point
updatBestPriceCoordinate()
{
  // The best price point is the closes point for the rectangle of corner points:
  // the best driver and B
  if (this.state.bestDriver == undefined || this.latB == undefined || this.lonB == undefined)
  {
    this.setState({bestPrice_coordinate : undefined});
  }
  else
  {
    var lat = this.latA, lon = this.lonA;

    if (this.latA < this.state.bestDriver.latitude && this.latA < this.latB)
      lat = Math.min(this.state.bestDriver.latitude, this.latB)
    if (this.latA > this.state.bestDriver.latitude && this.latA > this.latB)
      lat = Math.max(this.state.bestDriver.latitude, this.latB)

    if (this.lonA < this.state.bestDriver.longitude && this.lonA < this.lonB)
      lon = Math.min(this.state.bestDriver.longitude, this.lonB)
    if (this.lonA > this.state.bestDriver.longitude && this.lonA > this.lonB)
      lon = Math.max(this.state.bestDriver.longitude, this.lonB)

    //console.log(`updatBestPriceCoordinate: ${lat}, ${lon}`)

    this.setState({bestPrice_coordinate : {latitude: lat, longitude: lon}});
  }

 // if (this.state.bestPrice_coordinate != undefined)
   // console.log(`updatBestPriceCoordinate2: ${this.state.bestPrice_coordinate.latitude}, ${this.state.bestPrice_coordinate.longitude}`)
}


/*

  Hook: a user moves Pin of A

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
  {
    this.setState({
        rider_to_driver_route: [
          {latitude: this.latA, longitude: this.lonA},
          {latitude: this.state.bestDriver.latitude, longitude: this.state.bestDriver.longitude}
        ],
        A_to_B_route: [
          {latitude: this.latA, longitude: this.lonA},
          {latitude: this.latB, longitude: this.lonB}
        ]
    })

    //console.log(`onRegionChange: A_to_best_price_route = ${this.state.A_to_best_price_route}`)

    if (this.state.bestPrice_coordinate != undefined)
      this.setState({
        A_to_best_price_route: [
          {latitude: this.latA, longitude: this.lonA},
          {latitude: this.state.bestPrice_coordinate.latitude, longitude: this.state.bestPrice_coordinate.longitude}
        ]
      })
  }

  // Update coordinates for the best price point
  this.updatBestPriceCoordinate();

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
        this.render_polyline_A_to_driver = (
<MapView.Polyline
            
            lineDashPattern={[5,5]}

            coordinates={this.state.rider_to_driver_route}
		

		strokeColor="green" // fallback for when `strokeColors` is not supported by the map-provider
		/*strokeColors={[
			'#7F0000',
			'#00000000', // no color, creates a "long" gradient between the previous and next coordinate
			'#B24112',
			'#E5845C',
			'#238C23',
			'#7F0000'
		]}*/
		strokeWidth={1}
            
            />
        )

        // Route from A to B
        this.render_polyline_A_to_B = (
          <MapView.Polyline coordinates={this.state.A_to_B_route}
          lineDashPattern={[5,5]}
		      strokeColor="blue" 
		      strokeWidth={1}/>)

        // From A to best price
        this.render_polyline_A_to_best_price = (
          <MapView.Polyline coordinates={this.state.A_to_best_price_route}
          lineDashPattern={[5,5]}
		      strokeColor="red"
		      strokeWidth={1}/>)

      // Information about the price and the distance
      if (this.state.is_order_placed)
      {
        this.render_price_and_distance = (

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
       this.render_price_and_distance = (

        <Text style={{ position: 'absolute', top: 150, borderWidth: 0, fontSize: '15px', fontWeight: 'bold'}}>Цена {Math.round(this.state.order_price)} Руб{"\n"}Подача {Math.round(this.state.order_pickup_ETA)} минут{"\n"}{"\n"}<Text style={{fontSize: '11px'}}>Идите по зеленой стрелке для быстрой подачи{"\n"}к флажку для снижения цены</Text></Text>
      
      )


      }
      else 
        this.order_taxi = ( <Text></Text>  );



      


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

            <MapView.Marker coordinate={this.state.region2}/>
            <MapView.Marker pinColor='green' coordinate={this.state.bestDriver}/>
            <MapView.Marker pinColor='black' coordinate={this.state.B_coordinate}/>
            <MapView.Marker image={require('./assets/pink-flag.png')} coordinate={this.state.bestPrice_coordinate}/>

            {this.render_polyline_A_to_driver}
            {this.render_polyline_A_to_B}
            {this.render_polyline_A_to_best_price}

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
      <Text style={{ position: 'absolute', top: 30, fontSize: '15px'}}>Откуда</Text>

        {this.render_price_and_distance}
        {this.render_suggested_price}

      <Text style={{ position: 'absolute', top: 380, fontSize: '15px'}}>Куда</Text>
      <TextInput style={{ position: 'absolute', left : 10, right : 10, top: 50, height: 40, borderColor: 'gray', backgroundColor: 'white',
        borderWidth: 3, fontSize: '19px', fontWeight: 'bold'}} value={ this.state.addr_A}/>


      <TextInput selectTextOnFocus={true} 
      
      onChangeText={this.onChangeAddrB}
      onBlur={this.onBlurAddrB}
      onFocus={this.onFocusAddrB}

      ref={this.AddrBInputRef}

      style={{ position: 'absolute', left : 10, right : 10, top: this.state.addrBPosition, height: 40, borderColor: 'gray', backgroundColor: 'white',  borderWidth: 3, fontSize: '19px', fontWeight: 'bold'}}

        value={this.state.addressInB}
        
        />

        <Text style={[{ 
          
          
          position: 'absolute', top: this.state.addrSuggestedBPosition, backgroundColor: 'white', borderWidth: 0, color: 'black', fontWeight: 'bold', fontSize: '23px'},
          !this.state.addrB_has_focus && {display: 'none'}
          
        ]}
        
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



