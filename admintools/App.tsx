import React, { useState } from 'react';
import price_view_icon from './icons/price_view.svg'
import assigned_driver_icon from './icons/assigned_driver.svg'
import unassigned_driver_icon from './icons/unassigned_driver.svg'
import assigned_order_icon from './icons/assigned_order.svg'
import unassigned_order_icon from './icons/unassigned_order.svg'
//import ride_begins_icon from './icons/ride_begins.svg'
import flag_checkered_solid from './icons/flag-checkered-solid.svg'
import angry_regular  from './icons/angry-regular.svg'
import './App.css';

import * as LeafeLet from 'react-leaflet'
import { LatLngTuple, LatLng, icon} from 'leaflet';
import * as server_api_sdk from './server_api_sdk.js'
import * as route from './route'



//import { ContextMenu, MenuItem } from "react-contextmenu";
//import DropdownButton from 'react-bootstrap/DropdownButton'
//import Dropdown from 'react-bootstrap/Dropdown'

const defaultLatLng: LatLngTuple = [55.5442069, 37.52135357];
const zoom:number = 15;

let rider_A_latlng : any;
let rider_B_latlng : any;

let route_map = new Map();
let route_params_map = new Map();


const LeafletMap:React.FC = () => {

  // When a new message from the websocket comes in
  const on_message_from_websocket = () => {
    let all_d = server_api_sdk.api_get_all_drivers(on_message_from_websocket);
    let all_r = server_api_sdk.api_get_all_riders(on_message_from_websocket);

    //console.log(`on_message_from_websocket: driver=${JSON.stringify(driver)}`);
    //console.log(`on_message_from_websocket: all drivers=${JSON.stringify(all_d)}, all_d.length=${all_d.length}`);

    set_all_drivers(all_d);
    set_all_riders(all_r);

    // Check if this rider is on the route map and if they're not then build the route
    // Note: this can happen on initial load of the map when riders come from the server
    for (let rider of all_r)
    {
      if (route_map.get(rider.rider_id) === undefined)
      {
        // Set the route temporatily to an empty one until it builds
        // This is to avoid building a lot of routes simulteneously for each rider
        route_map.set(rider.rider_id, [[]]);

        route.get_route(new route.LatLng(rider.latA, rider.lonA),
                        new route.LatLng(rider.latB, rider.lonB),
        (route_object : route.RouteObject) =>
        {
          route_map.set(rider.rider_id, route_object.route.map (x => [x.latitude, x.longitude]));
          route_params_map.set(rider.rider_id, {distance: route_object.distance, duration: route_object.duration});
          set_all_routes(all_routes + 1);
      });
      }
    }

  }

  const [all_drivers, set_all_drivers] = useState(server_api_sdk.api_get_all_drivers(on_message_from_websocket));
  const [all_riders, set_all_riders] = useState(server_api_sdk.api_get_all_riders(on_message_from_websocket));
  const [all_routes, set_all_routes] = useState(0);//new Map());
  const [context_menu_popup_position, set_context_menu_popup_position] = useState(defaultLatLng);
  const [popup_key, set_popup_key] = useState(1);

  const get_driver_icon = (driver : any) : any =>
  {
    if (driver.assigned_rider_id)
      return assigned_driver_icon;
    else
    if (driver.best_rider_id)
      return unassigned_driver_icon;
    else
      return angry_regular;
  }

  // Rendering markers with drivers
  const listItems = all_drivers.map((driver) => 
    <LeafeLet.Marker icon={icon({
    iconUrl: get_driver_icon(driver),
    iconSize: [38, 95]})} key={driver.driver_id} position={ function(){
      return new LatLng(driver.lat, driver.lon)
    }()} onclick={function (m: any) {

      // Left click on a driver marker accepts the order IF there is a best order
      //  and IF the driver is not assigned yet
      if (driver.best_rider_id && !driver.assigned_rider_id)
        server_api_sdk.api_websocket_send_(`{"event":"marketplace2_assign_order", "data":{"driver_id":${driver.driver_id},"rider_id":${driver.best_rider_id}}}`);
      }}>
      <LeafeLet.Tooltip>
        driver_id: {driver.driver_id}<br/>
        best_rider_id: {driver.best_rider_id}<br/>
        best_profit_margin: {(driver.best_profit_margin===undefined)?0:driver.best_profit_margin.toFixed(2)}<br/>
        best_order_price: {(server_api_sdk.get_rider_by_id(driver.best_rider_id)===undefined)?0:
          server_api_sdk.get_rider_by_id(driver.best_rider_id).price_as_placed}<br/>
        assigned_rider_id: {driver.assigned_rider_id}<br/>
      </LeafeLet.Tooltip>
    </LeafeLet.Marker>
  );

  const get_rider_icon = (rider : any) : any =>
  {
    if (rider.price_as_placed === 0)
      return price_view_icon;
    else
    if (rider.assigned_driver_id !== 0)
      return assigned_order_icon;
    else
      return unassigned_order_icon;
  }

  // Rendering markers with riders
  const listItems_riders = all_riders.map((rider) => 
   <LeafeLet.Marker icon={icon({
   iconUrl: get_rider_icon(rider),
   iconSize: [38, 95]})} key={rider.rider_id} position={ function(){
     return new LatLng(rider.latA, rider.lonA)
   }()} onclick={function (m: any) {
    // Left click on a rider marker - place the order with the market price received from
    // the server for this rider id
    server_api_sdk.api_websocket_send_(`{"event":"marketplace2_udpate_order", "data":{"rider_id":${rider.rider_id},"latA":${rider.latA},"lonA":${rider.lonA},"latB":${rider.latB},"lonB":${rider.lonB},"placed_order_price":${rider.market_price}}}`);
    console.log('rider marker onclick');}}>
     <LeafeLet.Tooltip>
       rider_id: {rider.rider_id}<br/>
       price_as_placed: {(rider.price_as_placed===undefined)?0:rider.price_as_placed.toFixed(1)}<br/>
       market_price: {(rider.market_price===undefined)?0:rider.market_price.toFixed(1)}<br/>
       best_driver_id: {rider.best_driver_id}<br/>
       real_distance_AB: {(route_params_map.get(rider.rider_id)===undefined)?0:
        (route_params_map.get(rider.rider_id).distance/1000).toFixed(1)}<br/>
       assigned_driver_id: {rider.assigned_driver_id}<br/>
     </LeafeLet.Tooltip>
   </LeafeLet.Marker>
  );

  // Rendering markers with drop off points
  const listItems_drop_off_points = all_riders.map((rider) => 
   <LeafeLet.Marker icon={icon({
   iconUrl: flag_checkered_solid,
   iconSize: [38, 95]})} key={rider.rider_id} position={ function(){
     return new LatLng(rider.latB, rider.lonB)
   }()}>
   </LeafeLet.Marker>
  );

 // Rendering rider routes
 const rider_routes = all_riders.map((rider) =>
 <LeafeLet.Polyline key={rider.rider_id} positions={
  (all_routes) ? route_map.get(rider.rider_id) === undefined ? [[]] : route_map.get(rider.rider_id) : [[]]
} color='red'/>
);

// Rendering popup for the context menu
const render_context_menu_popup = 
context_menu_popup_position === defaultLatLng ? (<div></div>) :
(
  <LeafeLet.Popup key={`popup-${popup_key}`} position={context_menu_popup_position}>
      <button onClick={function (m: any) {
        
        // Creating a driver

        let lat = context_menu_popup_position[0];
        let lng = context_menu_popup_position[1];
        
        server_api_sdk.api_websocket_send_
        (`{"event":"marketplace2_update_driver_position", "data":{"driver_id":${Math.round(Math.random() * 1000000000)},"lat":${lat},"lon":${lng}}}`);

        set_context_menu_popup_position(defaultLatLng);
        set_popup_key(popup_key + 1);
  
        //console.log(`oncontextmenu: m=${JSON.stringify(m.latlng)}`)
      }}>Create driver</button>
      <button onClick={function (m: any) {

        // Setting a pickup point

        let lat = context_menu_popup_position[0];
        let lng = context_menu_popup_position[1];

        rider_A_latlng = {lat: lat, lng: lng};

        set_context_menu_popup_position(defaultLatLng);
        set_popup_key(popup_key + 1);

      }}>Set pickup point</button>
      <button onClick={function (m: any) {

        // Setting a drop-off point and complete a price view
        if (rider_A_latlng !== undefined)
        {
          let lat = context_menu_popup_position[0];
          let lng = context_menu_popup_position[1];

          rider_B_latlng = {lat: lat, lng: lng};

          let rider_id = Math.round(Math.random() * 1000000000);
          server_api_sdk.api_websocket_send_(`{"event":"marketplace2_udpate_order", "data":{"rider_id":${rider_id},"latA":${rider_A_latlng.lat},"lonA":${rider_A_latlng.lng},"latB":${rider_B_latlng.lat},"lonB":${rider_B_latlng.lng},"placed_order_price":0}}`);

          set_context_menu_popup_position(defaultLatLng);
          set_popup_key(popup_key + 1);
        }

        rider_A_latlng = undefined;

      }}>Set drop-off point</button>
      <button>Change cost</button>
  </LeafeLet.Popup>
);

  
//<Popup>A pretty CSS3 popup.<br />Easily customizable.</Popup>
   var mymap = (
    <LeafeLet.Map id="mapId" 
    oncontextmenu={function (m: any) {

      set_context_menu_popup_position([m.latlng.lat, m.latlng.lng]);
      set_popup_key(popup_key + 1);

      // Right click - create a driver
/*
      
      server_api_sdk.api_websocket_send_
        (`{"event":"marketplace2_update_driver_position", "data":{"driver_id":${Math.round(Math.random() * 1000000000)},"lat":${m.latlng.lat},"lon":${m.latlng.lng}}}`);
        console.log(`oncontextmenu: m=${JSON.stringify(m.latlng)}`);*/
    }}
    ondblclick={function (m: any) {
      
      // Dblclick - start createing a price view (click will finish it)

      //rider_A_latlng = m.latlng;
  
      //console.log(`ondblclick: m=${JSON.stringify(m.latlng)}`);
  
    }}

    onclick={function (m: any) {

      if (rider_A_latlng !== undefined)
      {
     /*   // Complete a price view

        rider_B_latlng = m.latlng;

        let rider_id = Math.round(Math.random() * 1000000000);
        server_api_sdk.api_websocket_send_(`{"event":"marketplace2_udpate_order", "data":{"rider_id":${rider_id},"latA":${rider_A_latlng.lat},"lonA":${rider_A_latlng.lng},"latB":${rider_B_latlng.lat},"lonB":${rider_B_latlng.lng},"placed_order_price":0}}`);

        // Build the rounte only if it's not built yet - to avoid ddosing the server
        if (route_map.get(rider_id) === undefined)
        {
          // Set the route temporatily to an empty one until it builds
          // This is to avoid building a lot of routes simulteneously for each rider
          route_map.set(rider_id, [[]]);

          route.get_route(new route.LatLng(rider_A_latlng.lat, rider_A_latlng.lng),
            new route.LatLng(rider_B_latlng.lat, rider_B_latlng.lng),
            (route_object : route.RouteObject) =>
            {
              // console.log(`ROUTE BUILT: ${JSON.stringify(route_object)}`);
              route_map.set(rider_id, route_object.route.map (x => [x.latitude, x.longitude]));
              route_params_map.set(rider_id, {distance: route_object.distance, duration: route_object.duration});
              set_all_routes(all_routes + 1);
              //console.log(`route_map.get(rider.rider_id)=${JSON.stringify(route_map.get(rider.rider_id))}`);
          });
        }*/

/*        route.get_route(new route.LatLng(rider.latA, rider.lonA),
        new route.LatLng(rider.latB, rider.lonB),
        (route_object : route.RouteObject) =>
        {
          console.log(`ROUTE BUILT: ${JSON.stringify(route_object)}`);
          route_map.set(rider.rider_id, route_object.route.map (x => [x.latitude, x.longitude]));
          console.log(`route_map.get(rider.rider_id)=${JSON.stringify(route_map.get(rider.rider_id))}`);
        });*/

        /*route.get_route(new route.LatLng(rider_A_latlng.lat, rider_A_latlng.lng),
        new route.LatLng(rider_B_latlng.lat, rider_B_latlng.lng),
        (route_object : route.RouteObject) =>
        {
         // console.log(`ROUTE BUILT: ${JSON.stringify(route_object)}`);
         set_all_routes(all_routes.set(rider_id, route_object.route.map (x => [x.latitude, x.longitude])));
          //console.log(`route_map.get(rider.rider_id)=${JSON.stringify(route_map.get(rider.rider_id))}`);
        });*/
        
        /*let all_d = server_api_sdk.api_get_all_drivers(on_message_from_websocket);
        let all_r = server_api_sdk.api_get_all_riders(on_message_from_websocket);
    
        //console.log(`on_message_from_websocket: driver=${JSON.stringify(driver)}`);
        //console.log(`on_message_from_websocket: all drivers=${JSON.stringify(all_d)}, all_d.length=${all_d.length}`);
    
        set_all_drivers(all_d);
        set_all_riders(all_r);*/

        //set_all_routes(all_routes + 1);

      }

      //rider_A_latlng = undefined;

      
      console.log(`onclick: m=${JSON.stringify(m.latlng)}`);
  
    }}
    center={defaultLatLng}
    zoom={zoom}>
<LeafeLet.TileLayer
   url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
   attribution="&copy; <a href=&quot;http://osm.org/copyright&quot;>OpenStreetMap</a> contributors">


</LeafeLet.TileLayer>



    {listItems}
    {listItems_riders}
    {listItems_drop_off_points}
    {rider_routes}
    {render_context_menu_popup}


 
</LeafeLet.Map>



   )

   return mymap;
}

/*

<LeafeLet.Marker position={new LatLng(55.5442069, 37.52135357)}>
      <LeafeLet.Popup>A pretty CSS3 popup.<br />Easily customizable.</LeafeLet.Popup>
    </LeafeLet.Marker>

    <LeafeLet.Marker position={[55.5442069, 37.52135357]}>
<LeafeLet.Popup>
  Some cool popup! :-)
</LeafeLet.Popup>
</LeafeLet.Marker>



*/


/*

<div style={{position:"absolute", top: 100, left:100}}>
<Dropdown>
<Dropdown.Toggle variant="success" id="dropdown-basic">
  Dropdown Button
</Dropdown.Toggle>

<Dropdown.Menu>
  <Dropdown.Item href="#/action-1">Action</Dropdown.Item>
  <Dropdown.Item href="#/action-2">Another action</Dropdown.Item>
  <Dropdown.Item href="#/action-3">Something else</Dropdown.Item>
</Dropdown.Menu>
</Dropdown>
</div>
*/

export default LeafletMap;
