/*  //Fetching the data using hhtp request

import React from 'react';
import {Text,View,StyleSheet,FlatList} from 'react-native';
//We use lifecycle components of react for fetching the data;



//Defining the class Component
export default class Sample extends React.Component{
state={
List:[]
}

//var aaa;// = "000";

//aaa = "000"

  Listings=({item})=>
  {
return(

  <Text>{item}a</Text>
)



  }


  constructor(props) { 
    
    super(props);



    this.aaa = "999";

    //const response = await fetch("https://rallycoding.herokuapp.com/api/music_albums");
  
//   fetch("https://rallycoding.herokuapp.com/api/music_albums", //{mode : 'no-cors'},

   fetch("http://185.241.194.113:1234/geo/find_house?s=%D1%83%D0%BB%D0%B8%D1%86%D0%B0%20%D0%9E%D1%81%D1%82%D1%80%D0%BE%D0%B2%D1%81%D0%BA%D0%BE%D0%B3%D0%BE%20(%D0%AD%D0%B6%D0%B2%D0%B0)%201", //{mode : 'no-cors'},




  {headers: {'Access-Control-Allow-Origin' : '*'}}
  
  
  ).then((response)=>
    {
      console.log("eee");
      this.aaa="XXX";

      this.setState({comment: response.json[0].full_address
      });

      this.aaa = this.state.comment;

    }
  )

}

  componentDidMount()
  {
fetch("https://rallycoding.herokuapp.com/api/music_albums").then((response)=>{//this.setState(
  {
//List:this.state.List.push(response);
console.log("ddd");
//this.state.List.push(response);
this.aaa="123";
//this.render();

  }
//)
});

  }
render()
{
return(
<View>
<Text>hiee !!!!!!!!!!!!!</Text>
<Text>{this.Listings}xxx{this.aaa} - {this.state.comment}</Text>
<FlatList data={this.state.List} renderItem={this.Listings}></FlatList>
</View>
)

}

}  */




import React from 'react';
import { FlatList, ActivityIndicator, Text, View, TextInput, Button } from 'react-native';

export default class FetchExample extends React.Component {
  constructor(props) {
    super(props);
    this.state = { isLoading: true };
    this.phoneNumber = "zzz";
    this.onChangeText = this.onChangeText.bind(this);
    this.sendSMSCode = this.sendSMSCode.bind(this);
  }

 phoneNumber = "zzz"

  componentDidMount() {
    return fetch('http://185.241.194.113:1234/geo/get_houses_by_location?lat=61.8034404064179&lon=50.7255792091316&resolution=11&ring_size=1')
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
      });
  }

onPressTitle() {


  console.log("123");
}

onChangeText(s)
{
  this.phoneNumber = s;
 //this.setState(({phoneNumber}) => { return s})
}

sendSMSCode()
{
//console.log(this.phoneNumber);

fetch ("http://185.241.194.113:1234/auth/sendsms?phone=" + this.phoneNumber)
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

        //    type="text"
           // value={this.state.value}
            //onChange={this.handleChange}
         ///>
/*

  id="first-name"
  label="Name"
  value={this.state.name}
  onChange={this.handleChange('name')}
  margin="normal"
*/
   //onChangeText={text => onChangeText(text)}
   // style={{ height: 40, borderColor: 'gray', borderWidth: 1 }}
    //onPress={this.onPressTitle}
    return (

      <View style={{ flex: 1, paddingTop: 20 }}>
        <Text >Введите номер телефона</Text>
        <TextInput onChangeText={this.onChangeText}
         style={{ height: 40, borderColor: 'gray', borderWidth: 1 }}/>
        <Button
  onPress={this.sendSMSCode}
  title="Отправить код на телефон"
  color="#841584"
/>


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
      </View>
    );
  }
}

