import React from 'react';
import MapView from 'react-native-maps';
import { StyleSheet, Dimensions, FlatList, ActivityIndicator, Text, View, TextInput, Button } from 'react-native';

export default class FetchExample extends React.Component {
  constructor(props) {
    super(props);
    this.state = { isLoading: false, page : "phone_reg", token : "" };
    this.enteredText = "";
    this.phoneNumber = "";
    this.onChangeText = this.onChangeText.bind(this);
    this.sendSMSCode = this.sendSMSCode.bind(this);
    this.enterSMSCode = this.enterSMSCode.bind(this);
    this.onRegionChange = this.onRegionChange.bind(this);
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


onRegionChange(region) {
  console.log(region.latitude, region.longitude)

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



    if (this.state.page == "phone_reg"/*"map"*/)
    {
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
      
          
          />
      <Text style={{ position: 'absolute', top: 50}}>Откуда</Text>
      <Text style={{ position: 'absolute', bottom: 90}}>Куда</Text>
      <TextInput style={{ position: 'absolute', left : 10, right : 10, top: 100, height: 40, borderColor: 'gray', 
        borderWidth: 1}} value={ this.state.addr_A}/>
      <TextInput style={{ position: 'absolute', left : 10, right : 10, bottom: 50, height: 40, borderColor: 'gray', 
        borderWidth: 1}}/>
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

/*

    <FlatList
          data={this.state.dataSource}
          renderItem={({ item }) => (
            <Text>
            {item.full_address}
              {item.title}, {item.releaseYear}
            </Text>

          )}
          keyExtractor={({ id }, index) => id}
        />


        */
