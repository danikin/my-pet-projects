/*
      SDK with the server
      All communications with server MUST be performed through this SDK

      (c) Dennis Anikin 2020
*/

/*
*   Websocket layer - internals
*/

var api_ws;

// Flag - is the websocket authenticated
// Note: the websocket MUST authenticate as soon as it connects - that's
//  important to receive personal messages like coordinate updates
var api_ws_in_authenticated_ = false;

// Internal function that initializes websocket
function api_initialize_websocket_()
{
    if (api_ws === undefined)
    {
      console.log(`api_initialize_websocket_`);

      api_ws_in_authenticated_ = false;

      api_ws = new WebSocket('ws://185.241.194.113:80');
      api_ws.onopen = api_websocket_onopen_;
      api_ws.onclose = api_websocket_onclose_;
      api_ws.onerror = api_websocket_onerror_;
      api_ws.onmessage = api_websocket_on_message_;
      console.log(`api_initialize_websocket_: done`);
    }
}

// Internal function that schedule re-initialization of
// the websocket in 0.1s
function api_reinitialize_websocket_()
{
  console.log(`api_reinitialize_websocket_`);

  let callme = function()
  {
    if (api_ws === undefined)
    {
      console.log(`api_reinitialize_websocket_: api_ws === undefined: call api_initialize_websocket_`);
      api_initialize_websocket_();
    }
    else
    {
      if (api_ws.readyState === 1)
      {
        // It's already connected
        console.log(`api_reinitialize_websocket_: it's already connected - send pending messages`);

        // Try to send pending messages
        api_websocket_send_pending_();
      }
      else
      if (api_ws.readyState === 0)
        console.log(`api_reinitialize_websocket_: the websocket is connecting. Let's wait`);
      else
      if (api_ws.readyState === 2)
        console.log(`api_reinitialize_websocket_: the websocket is closing. Let's wait`);
      else
      {
        console.log(`api_reinitialize_websocket_: the websocket is closed, reconnect the websocket, api_ws.readyState=${api_ws.readyState}`);

        // Initilize the websocket again
        // Note: it seems like the only way to connect a websocket is to creat a new
        // instance of the class WebSocket
        api_initialize_websocket_();
      }
    }
  } // let callme = function()

  // Call the function now - what if it helps, and schedule another call in 0.1s
  callme();
  setTimeout(callme, 100);
}

var api_pending_messages_ = [];

function api_websocket_onopen_()
{
  console.log(`api_websocket_onopen_`);

  // Try to send pending messages
  api_websocket_send_pending_();

  // Try to authenticate the websocket
  api_websocket_authenticate_();
}

function api_websocket_onclose_(e)
{
  console.log('api_websocket_onclose_, reason:', e.reason);

  // Schedule reinitialization
  api_reinitialize_websocket_();
}

function api_websocket_onerror_(e)
{
  console.log('api_websocket_onerror_, reason:', e.reason);
  
  // Schedule reinitialization
  api_reinitialize_websocket_();
}

// Sends pending messages to the server
function api_websocket_send_pending_()
{
  //console.log(`api_websocket_send_all_pending_: queue size is ${api_pending_messages_.length}`);

  // If the websocket is lost the reinitizlize it
  if (api_ws === undefined)
  {
    console.log(`api_websocket_send_all_pending_: websocket is lost - reinitialize it`);
    api_reinitialize_websocket_();
  }
  else
  {
    while (api_pending_messages_.length !== 0)
    {
      // Take a message from the pending queue
      let message = api_pending_messages_.shift();

      console.log(`api_websocket_send_all_pending_: got a message from pending: ${message}`);

      // And try to send it
      try
      { 
        console.log(`api_websocket_send_all_pending_: trying to send message from a pending queue ${message}`);
        api_ws.send(message);
      }
      catch (error)
      {
        console.log(`api_websocket_send_all_pending_: error sending message=${message}, error=${error}, put it back to the pending queue`);

        // Put a message back to the pending queue
        api_pending_messages_.unshift(message);

        // Reinitialize websocket
        api_reinitialize_websocket_();

        // Do break because reinitizlization will take time so it's useless to send over
        // and over
        break;
      } // catch (error)
    } // while (api_pending_messages_.length != 0)

  } // else: if (api_ws === undefined)
} // function api_websocket_send_all_pending_()

// Authenticated the websocket
function api_websocket_authenticate_()
{
  console.log(`api_websocket_authenticate_: try to authenticate, api_local_driver_id_=${api_local_driver_id_}`);

  // If the local driver has never been created then do nothing - will authenticate
  //  as soon as it is created
  if (api_local_driver_id_ !== undefined)
  {
    api_websocket_send_(`{"event":"auth","data":{"auth_string":"driver-${api_local_driver_id_}"}}`);

    // Blindly believe that the websocket is authenticated
    // TODO! In a future handle auth errors!!!
    api_ws_in_authenticated_ = true;
  }
}

// On the broadcast message is received with driver and rider lists
var api_ws_callback_on_message_received_;
// On the coordinate via a personal message is received
var api_ws_callback_on_coordinate_change_;
// On the stat personal message is received
var api_ws_callback_on_stat_ready_;

/*
*   Websocket layer - externals
*/

// Places a message in a locl queue
// Note: a message is just stored in a local queue and is reliably sent to a server
//  once it's accessible
// Note: this functions can be can anytime event if the websocket is not created or
//  is not connected
export function api_websocket_send_(message)
{
  console.log(`api_websocket_send_: put message to the pending queue ${message}`);
  api_pending_messages_.push(message);
  //console.log(`api_websocket_send_: after put message to the pending queue api_pending_messages_.length=${ api_pending_messages_.length}, api_pending_messages_=${JSON.stringify(api_pending_messages_)}`);

  // Try to send pending including the new one. It can work :-)
  api_websocket_send_pending_();
}


/*
*   API-SDK - internal helper functions and variables
*/


// These are id of a rider if this is a rider app otherwise it's undefined
// and id of a driver if this is a driver app otherwise it's undefined
//
// The only reason for these ids is to make sure that the rider and the driver
//  will be registered only once with once generated id
//
// Note: api_current_* above is inizialized only when an id matched with
//  api_local_*_id has been received from the server
var api_local_driver_id_;

var api_ws_current_order_;
var api_ws_current_driver_;

// Map of all drivers: driver id -> driver structure
var api_ws_all_drivers_ = [];
var api_ws_all_driver_map_ = new Map();

// Map of all riders: rider id -> rider structure
var api_ws_all_riders_ = [];
var api_ws_all_rider_map_ = new Map();

// Processes a message from the server and call callbacks
function api_websocket_on_message_(e)
{
  // Receive a message and parse it
  let message = JSON.parse(e.data);
  if (message === undefined)
  {
    console.log(`api_websocket_on_message_: message === undefined`);
    return;
  }
  if (message.data === undefined)
  {
    // Bad message - skip it
    console.log(`api_websocket_on_message_: BAD message, without "data": ,message=${JSON.stringify(message)}, e=${JSON.stringify(e)}`);
    return;
  }

  // Update maps of drivers and riders
  if (message.data.drivers !== undefined)
  {
      for (let driver of message.data.drivers)
        api_ws_all_driver_map_.set(driver.driver_id, driver);

      //console.log(`api_websocket_on_message_: message.data.drivers=${JSON.stringify
        //(message.data.drivers)}, api_ws_all_driver_map_.length=${api_ws_all_driver_map_.length}`);

        //console.log(`api_ws_all_driver_map_.size=${api_ws_all_driver_map_.size}`);
  
      // Save the map as an array
      api_ws_all_drivers_ = [];
      api_ws_all_driver_map_.forEach(function(value){api_ws_all_drivers_.push(value)});

      //console.log(`api_ws_all_drivers_=${JSON.stringify(api_ws_all_drivers_)}`);

      //console.log(`api_websocket_on_message_: api_ws_all_drivers_=${JSON.stringify
        //(api_ws_all_drivers_)}`);
  }
       // console.log(`api_websocket_on_message_: api_ws_all_drivers_=${JSON.stringify(api_ws_all_drivers_)}`);
  if (message.data.riders !== undefined)
  {
      for (let rider of message.data.riders)
        api_ws_all_rider_map_.set(rider.rider_id, rider);
      api_ws_all_riders_ = [];
      api_ws_all_rider_map_.forEach(function(value){api_ws_all_riders_.push(value)});
  }



  

      
        if (api_ws_callback_on_message_received_ === undefined)
        {
          console.log(`api_websocket_on_message_: NO CALLBACK! Can't return: driver_to_return=${JSON.stringify(api_ws_current_driver_)}, rider_to_return=${JSON.stringify(api_ws_current_order_)}`);
        }
        else
          api_ws_callback_on_message_received_();
      

    // 2. Process a personal message - coordinate change
    if (message.data.personal_message_type === "simulator_coordinate_change")
      api_ws_callback_on_coordinate_change_({latitude: message.data.lat, longitude: message.data.lon});
    else
    // 3. Personal message - statistics
    if (message.data.personal_message_type === "stat")
      api_ws_callback_on_stat_ready_(message.data.entries);

} // function api_websocket_on_message_(e)


// Returns current order and driver from the recent update
export function api_get_all_riders(callback_on_message)
{
  if (callback_on_message !== undefined)
    api_ws_callback_on_message_received_ = callback_on_message;
    api_initialize_websocket_();
  return api_ws_all_riders_;
}
export function api_get_all_drivers(callback_on_message)
{
  // Hook the callback for messages
  if (callback_on_message !== undefined)
    api_ws_callback_on_message_received_ = callback_on_message;

  //console.log(`api_get_all_drivers IS CALLED AND CALLBACK SHOUD BE SET`);

  // Initializa the websocket if it's not initialized yet
  api_initialize_websocket_();

  return api_ws_all_drivers_;
}
export function get_rider_by_id(rider_id)
{
  return api_ws_all_rider_map_.get(rider_id);
}
export function get_driver_by_id(driver_id)
{
  return api_ws_all_driver_map_.get(driver_id);
}

// Set the best order auto accept flag for a driver
export function api_driver_set_auto_accept_best_order_flag(flag)
{
  api_websocket_send_(`{"event":"test_set_auto_accept_best_order_flag", "data":{"driver_id":${api_local_driver_id_},"flag":${flag ? 1 : 0}}}`);
}

// Begins a ride to B
export function api_driver_ride_begins()
{
  // Start a simulator (it does not change the server state - it just starts moving the driver to B)
  api_websocket_send_(`{"event":"test_start_ride_simulator","data":{"driver_id":${api_local_driver_id_}}}`);

  // Change a state in the server that the ride actually has started
  api_websocket_send_(`{"event":"test_driver_started_ride", "data":{"driver_id":${api_local_driver_id_}}}`);
}

// Finishes a ride
export function api_driver_ride_finishes()
{
  api_websocket_send_(`{"event":"test_finish_ride","data":{"driver_id":${api_local_driver_id_}}}`);
}

// Sets a driver at A
export function api_driver_got_A()
{
  api_websocket_send_(`{"event":"test_driver_got_A", "data":{"driver_id":${api_local_driver_id_}}}`);
}

// Driver confirms the toll wait
export function api_driver_accepted_toll_wait(seconds)
{
  api_websocket_send_(`{"event":"test_driver_accepted_toll_wait", "data":{"driver_id":${api_local_driver_id_}, "seconds": ${seconds}}}`);
}

// Driver cancels an order
export function api_driver_cancelled_order()
{
  api_websocket_send_(`{"event":"test_driver_cancelled_order", "data":{"driver_id":${api_local_driver_id_}}}`);
}

// Driver accepts the best order
export function api_driver_accept_order(order_id)
{
  api_websocket_send_(`{"event":"test_accept_order", "data":{"rider_id":${order_id},"driver_id":${api_local_driver_id_}}}`);
}

// Returns driver statistics
export function api_driver_get_stat(stat_type, ts_from, ts_to, on_stat_ready)
{
  api_websocket_send_(`{"event":"test_get_stat", "data":{"driver_id":${api_local_driver_id_},"stat_type":"${stat_type}","from":${ts_from},"to":${ts_to}}}`);

  api_ws_callback_on_stat_ready_ = on_stat_ready;
}


