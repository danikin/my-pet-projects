import React from 'react';
import MapView from 'react-native-maps';
import Marker from 'react-native-maps';
import { StyleSheet, Dimensions, FlatList, ActivityIndicator, Text, View, TextInput, Button } from 'react-native';
import flagPinkImg from './assets/flag-pink.png';

export default class FetchExample extends React.Component {
  constructor(props) {
    super(props);
    this.state = { isLoading: false, page : "map", token : "" };
    this.enteredText = "";
    this.phoneNumber = "";
    this.onChangeText = this.onChangeText.bind(this);
    this.sendSMSCode = this.sendSMSCode.bind(this);
    this.enterSMSCode = this.enterSMSCode.bind(this);
    this.onRegionChange = this.onRegionChange.bind(this);
    this.onChangeAddrB = this.onChangeAddrB.bind(this);
    this.autocompleteB = this.autocompleteB.bind(this); 
    this.AddrBInputRef = React.createRef();
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

/*
 
 Hook for changing the address of B
 
*/
onChangeAddrB(addr_B)
{
    // On each change of B return to the map state from the map_request_ride state
    this.setState(
          {
            page: "map",
    })

    // Use space as a request for auto completion
    if (this.state.autocompletedAddrB &&
        addr_B.length > this.state.autocompletedAddrB.length && addr_B.endsWith(' '))
    {
      this.autocompleteB();
    }

    // Synchronize inputed text and the state
    this.setState(
          {
            autocompletedAddrB: addr_B
          }
      )



  /*console.log(
    "'"+this.state.autocompletedAddrB + "',"+ addr_B + "','" +
    this.fullStreet + "'", this.isAutocompletedAddrBFullStreet);
*/


  // If street name is already there then search for a house
  if (this.currentStreet && addr_B.startsWith(this.currentStreet))
  {
    fetch("http://185.241.194.113:1234/geo/find_house?s=" + addr_B)
    .then(response => response.json())
  .then(responseJson => {
      this.setState(
          {
            // Now save the full address with the house
            addr_B: responseJson[0].full_address,
            //autocompletedAddrB: addr_B
          }
      )
      this.isFullAddressInB = true;
      this.latB = responseJson[0].house_latitude;
      this.lonB = responseJson[0].house_longitude;
  })
  }
  // If street name is not already there then search for a street
else
{

  fetch("http://185.241.194.113:1234/geo/find_street?s=" + addr_B)

  .then(response => response.json())
  .then(responseJson => {

      // Found a street - save it the the address of B (it's just a street, not a full address)
      this.setState(
          {
            addr_B: responseJson[0][1],
          }
      )
      this.isFullAddressInB = false;
  })
}
}

/*

  Hook: a user selects the address from suggestion

*/

autocompleteB()
{

  // If the full address is there then show the button "Request a Ride"
  if (this.isFullAddressInB)
  {
        this.setState(
          {
            page: "map_request_ride",
          })

          // Lose the focus for the input of B to hide keyboard
          this.AddrBInputRef.current.blur();
  }



  this.setState(
            {
              autocompletedAddrB: this.state.addr_B,
            })
            this.currentStreet = this.state.addr_B;
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

  this.setState({ region });
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



    if (this.state.page == "map" || this.state.page == "map_request_ride")
    {

// Add Button for ordering if it's time to order
if (this.state.page == "map_request_ride")
    this.order_taxi = (<Text color="black"
        
        title="Заказать такси" 
        style={{ position: 'absolute', top: 320, borderColor: 'black', borderWidth: 2, backgroundColor: 'yellow', color: 'black',
        fontSize: '30px'}}
        
        >Заказать такси</Text>);
  else 
    this.order_taxi = ( <Text></Text>  );

return (
      <View style={styles.container}>

        <MapView style={styles.mapStyle} initialRegion={{
            latitude: 61.666594,
            longitude: 50.827201,
            latitudeDelta: 0.01,
            longitudeDelta: 0.01
          }}

          region={this.state.region}
      onRegionChange={this.onRegionChange}
      
          
          >
          <Marker coordinate={{ latitude: 61.666594, longitude: 50.827201}}
          
            centerOffset={{ x: -18, y: -60 }}
            anchor={{ x: 0.69, y: 1 }}

            image={flagPinkImg}

          />
          </MapView>
      <Text style={{ position: 'absolute', top: 50}}>Откуда</Text>
      <Text style={{ position: 'absolute', top: 150}}>Куда</Text>
      <TextInput style={{ position: 'absolute', left : 10, right : 10, top: 100, height: 40, borderColor: 'gray', 
        borderWidth: 1}} value={ this.state.addr_A}/>


      <TextInput onChangeText={this.onChangeAddrB}
      
      ref={this.AddrBInputRef}

      style={{ position: 'absolute', left : 10, right : 10, top: 200, height: 40, borderColor: 'gray', 
        borderWidth: 1}}
        value={this.state.autocompletedAddrB}
        
        />

        <Text style={{ position: 'absolute', top: 250, backgroundColor: 'green',
        
        borderWidth: 2, color: 'black',
        fontSize: '19px'
        
        }}
        onPress={this.autocompleteB}
        >{this.state.addr_B}</Text>

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


