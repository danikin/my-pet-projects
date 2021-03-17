import React from 'react';
import MapView, { PROVIDER_GOOGLE } from 'react-native-maps';
import Marker from 'react-native-maps';
import { StyleSheet, Dimensions, FlatList, ActivityIndicator, Text, View, TextInput, Button } from 'react-native';
import flagPinkImg from './assets/flag-pink.png';




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
        addrSuggestedBPosition: 460
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
}


  componentDidMount() {
/*    return fetch('http://185.241.194.113:1234/geo/get_houses_by_location?lat=61.8034404064179&lon=50.7255792091316&resolution=11&ring_size=1')
      //'https://reactnative.dev/movies.json')
      .then(response => response.json())
      .then(responseJson => {
        this.setState(
          {
            isLoading: false,
            //phoneNumber: "",
            dataSource: responseJson.data.houses,//movies,
          },
          function() {}
        );
      })
      .catch(error => {
        console.error(error);
      });*/
  }


onChangeText(s)
{
  this.enteredText = s;
}

findHouse()
{
  if (this.isAtLeastStreetInB)
  {
    fetch("http://185.241.194.113:1234/geo/find_house?s=" + this.state.addressInB)
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
  })
  }
}

onFocusAddrB()
{
  this.setState({
      addrBPosition : this.state.addrBPosition - 200,
      addrSuggestedBPosition : this.state.addrSuggestedBPosition - 200
  })

  // If address B gets or loses focus and the street is there then try to find
  // a house suggestion
  this.findHouse();
}

onBlurAddrB()
{
  this.setState({
      addrBPosition : this.state.addrBPosition + 200,
      addrSuggestedBPosition : this.state.addrSuggestedBPosition + 200
  })

  // If address B gets or loses focus and the street is there then try to find
  // a house suggestion
  this.findHouse();
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



  /*console.log(
    "'"+this.state.autocompletedAddrB + "',"+ addr_B + "','" +
    this.fullStreet + "'", this.isAutocompletedAddrBFullStreet);
*/


  // If street name is already there then search for a house
  //  Note: newAddressInB can contain not necessarily a bare sreet name it also
  //  can contain something beyond - but that's OK - we want to autocomplete it to the house
  //  Note: it will not change the B field - change only the suggestion
  if (this.isAtLeastStreetInB && this.isAtLeastStreetInSuggestedB)
  {
    fetch("http://185.241.194.113:1234/geo/find_house?s=" + newAddressInB)
    .then(response => response.json())
  .then(responseJson => {
      this.setState(
          {
            // Now save the full address with the house
            addressInSuggestedB: responseJson[0].full_address
          }
      )
      this.isFullAddressInSuggestedB = true;
      this.isAtLeastStreetInSuggestedB = true;
      this.latB = responseJson[0].house_latitude;
      this.lonB = responseJson[0].house_longitude;
  })
  }
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

onRegionChange(region) {
  //console.log(region.latitude, region.longitude)

  this.latA = region.latitude;
  this.lonA = region.longitude;
  this.setState({ region2 : {latitude: region.latitude, longitude: region.longitude} });

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
/*
  Commented this out because it's slow with Google Maps for some reason

  this.setState({ region });

  */
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
        this.order_taxi = (<Text color="black"
        
        title="Заказать такси" 
        style={{ position: 'absolute', top: 500, borderColor: 'black', borderWidth: 2, backgroundColor: 'yellow', color: 'black',
        fontSize: '30px'}}
        
        >Заказать такси</Text>);
      else 
        this.order_taxi = ( <Text></Text>  );


          
//{{ latitude: 61.666594, longitude: 50.827201}}
/*
    {{ position: 'absolute', left : 10, right : 10, top: 410, height: 40, borderColor: 'gray',  borderWidth: 1, fontSize: '19px', fontWeight: 'bold'}}
*/

/*

const Bstyle = { position: 'absolute', left : 10, right : 10, top: this.state.addrBPosition, height: 40, borderColor: 'gray',  borderWidth: 1, fontSize: '19px', fontWeight: 'bold'}

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
      
          
          >

            <MapView.Marker ref={this.PinRef} coordinate={this.state.region2}
            
            
            />

          </MapView>
      <Text style={{ position: 'absolute', top: 30}}>Откуда</Text>
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



