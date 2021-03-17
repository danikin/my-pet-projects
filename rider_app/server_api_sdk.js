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
      if (api_ws.readyState == 1)
      {
        // It's already connected
        console.log(`api_reinitialize_websocket_: it's already connected - send pending messages`);

        // Try to send pending messages
        api_websocket_send_pending_();
      }
      else
      if (api_ws.readyState == 0)
        console.log(`api_reinitialize_websocket_: the websocket is connecting. Let's wait`);
      else
      if (api_ws.readyState == 2)
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
    while (api_pending_messages_.length != 0)
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
function api_websocket_send_(message)
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

var api_is_driver_;
// This is one of the following:
//  1.  Current price view or order of a rider if this is a rider app
//      If it's undefined for a rider app then a rider is yet to register
//      or authenticate
//  2.  The best order or assigned order of a driver if this is a driver
//      If it's udefined for a driver app then there is no best order at all
var api_current_order_;

// This is one of the following:
//  1.  A driver assigned to a current order of a rider if this is a rider app
//      If it's undefined for a rider app then there is either no order
//      for a rider - only a price view or even no price view (not registered or
//      not authenticated) 
//  2.  Current driver if this is a driver app
//      If it's undefined for a driver app then a driver is yet to register
//      or authenticate
var api_current_driver_;

// These are id of a rider if this is a rider app otherwise it's undefined
// and id of a driver if this is a driver app otherwise it's undefined
//
// The only reason for these ids is to make sure that the rider and the driver
//  will be registered only once with once generated id
//
// Note: api_current_* above is inizialized only when an id matched with
//  api_local_*_id has been received from the server
var api_local_rider_id_;
var api_local_driver_id_;

var api_ws_current_order_;
var api_ws_current_driver_;

// Processes a message from the server and call callbacks
function api_websocket_on_message_(e)
{
   /*console.log(`MESSAGE RECEIVED4`);

  console.log(`MESSAGE RECEIVED2: e=${JSON.stringify(e)}`);

  console.log(`MESSAGE RECEIVED2: e.data=${JSON.stringify(e.data)}`);
*/
  //console.log(`api_websocket_on_message_(${JSON.stringify(e)})`);


  //console.log(`api_websocket_on_message_: e.data=${e.data}`);

/*let message = undefined;

if (e === undefined)
return;

if (e.data === undefined)
return;*/

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

  /*
  *   For a driver app
  */

  if (api_local_driver_id_ !== undefined)
  {
    // 1. Iterate drivers & riders
    if (message.data.drivers !== undefined)
    {
      // Iterate drivers
      for (let driver of message.data.drivers)
      {
        // Save current driver and notify about it's possible change of properties
        if (driver.driver_id == api_local_driver_id_)
        {
          api_ws_current_driver_ = driver;
          break;
        }
      }

      //console.log(`api_websocket_on_message_: api_local_driver_id_=${api_local_driver_id_}, driver_to_return = (${JSON.stringify(driver_to_return)})`);
      

      // TODO! We need to change the logic below. It worked when everything was in a single
      // message. Now a rider update can be received earlier than a driver update - so we
      // need to keep all updates per driver and per rider

      // Now iterate riders to find the best or assigned order for a driver
      if (api_ws_current_driver_ !== undefined && message.data.riders !== undefined)
        for (let rider of message.data.riders)
        {
          if (api_ws_current_driver_.best_rider_id == rider.rider_id ||
              (api_ws_current_driver_.assigned_order_id !== undefined && // This is for not to have two equal undefs
                api_ws_current_driver_.assigned_order_id == rider.assigned_order_id))
          {
            api_ws_current_order_ = rider;
            break;
          }
        }

      //console.log(`api_websocket_on_message_: api_local_driver_id_=${api_local_driver_id_} rider_to_return = (${JSON.stringify(rider_to_return)})`);
      
      /*

      We don't need this check anymore because it was found that websocket does not work
      with messages more than 15800 bytes. That's why the server sends one message per
      object now and not everything at once

      // Check on data consistency
      if (api_ws_current_driver_ !== undefined)
      {
        if (api_ws_current_driver_.best_rider_id !== undefined &&
            api_ws_current_driver_.best_rider_id != -1 &&
            api_ws_current_order_ === undefined)
          {
            console.log(`api_websocket_on_message_: DATA INCONSISTENCY IN SERVER RESPONSE! driver_to_return.best_rider_id=${api_ws_current_driver_.best_rider_id}, but the rider is not found in the response`);
            // Unset the driver
            api_ws_current_driver_ = undefined;
          }
      }*/

      // If at least a driver to return then call a callback
      if (api_ws_current_driver_ !== undefined)
      {
        if (api_ws_callback_on_message_received_ === undefined)
        {
          console.log(`api_websocket_on_message_: NO CALLBACK! Can't return: driver_to_return=${JSON.stringify(api_ws_current_driver_)}, rider_to_return=${JSON.stringify(api_ws_current_order_)}`);
        }
        else
          api_ws_callback_on_message_received_();
      }

    } // if (message.data.drivers !== undefined)

    // 2. Process a personal message - coordinate change
    if (message.data.personal_message_type == "simulator_coordinate_change")
      api_ws_callback_on_coordinate_change_({latitude: message.data.lat, longitude: message.data.lon});
    else
    // 3. Personal message - statistics
    if (message.data.personal_message_type == "stat")
      api_ws_callback_on_stat_ready_(message.data.entries);

  } // if (api_local_driver_id_ !== undefined)

  /*
  *   For a rider app
  */

  if (api_local_rider_id_ !== undefined)
  {
    let notify = false;

    //console.log(`api_websocket_on_message_: api_local_rider_id=${api_local_rider_id_}`)
    // Iterate riders, find the current one and save their data
    if (message.data.riders !== undefined)
      for (let rider of message.data.riders)
        if (rider.rider_id == api_local_rider_id_)
        {
            // If there is a change the set the notification flag
            notify = notify || (api_ws_current_order_ !== rider)
            api_ws_current_order_ = rider;
        }

    //console.log(`api_websocket_on_message_: api_ws_current_order_=${JSON.stringify(api_ws_current_order_)}`)

    // Iterate drivers and find the one that will be the best
    if (message.data.drivers !== undefined)
      for (let driver of message.data.drivers)
      {
        // If we already received information about the current order then
        // use the best driver from that as the best one to show
        if (api_ws_current_order_ !== undefined &&
          api_ws_current_order_.best_driver_id == driver.driver_id)
        {
            // If there is a change the set the notification flag
            notify = notify || (api_ws_current_driver_ !== driver)
            api_ws_current_driver_ = driver;
            break;
        }
        else
        // Otherwise use ONE OF drivers who have this order on top
        if (api_local_rider_id_ == driver.best_rider_id)
        {
          // If there is a change the set the notification flag
          notify = notify || (api_ws_current_driver_ !== driver)
          api_ws_current_driver_ = driver;
          break;
        }
      }

    //console.log(`api_websocket_on_message_: api_ws_current_driver_=${JSON.stringify(api_ws_current_driver_)}`)

    // Change - notify
    if (notify)
      api_ws_callback_on_message_received_();
  } // if (api_local_rider_id_ !== undefined)

/*
  // Iterate riders
  if (message.data.riders !== undefined)
    for (let rider of message.data.riders)
    {
      // Save current order/rider if it's a rider app, the local rider is known and
      //  that id is received from the server
      if (rider.rider_id == api_local_rider_id_)
        api_current_order_ = rider;

      // Save best/assigned order if it's a driver app, 
     if (api_local_driver_id_ !== undefined && api_current_driver_ !== undefined &&
       //     api_current_driver_.best_rider_id === )
    }

  // Iterate drivers
  if (message.data.drivers !== undefined)
    for (let driver of messages.data.drivers)
    {
      // Save current driver if it's a driver app
      if (driver.driver_id == api_local_driver_id_)
        api_driver_on_driver_change(driver);
    }

  // Try to match the current order with its local id
  // and the current driver with its local id
  if ()*/

} // function api_websocket_on_message_(e)

var rider_A_;
var rider_B_;

// Sets points A and B for a rider if both are ready
function api_rider_set_AB_()
{
  // If either of points is undefined then do nothing
  if (rider_A_ === undefined || rider_B_ === undefined)
    return;

  // Create a rider id locally if we haven't done it yet
  if (api_local_rider_id_ === undefined)
    api_local_rider_id_ = Math.round(Math.random() * 1000000000);

  // Send a message to the server
  api_websocket_send_(`{"event":"test_price_view", "data":{"id":${api_local_rider_id_},"latA":${rider_A_.latitude},"lonA":${rider_A_.longitude},"latB":${rider_B_.latitude},"lonB":${rider_B_.longitude}}}`);
}

/*
*   API-SDK - external driver API
*/

// Sets driver's position
// Note: this is the only function in API that receives a callback
//  Once the callback is received by a caller it can use since then
// Note: this function MUST be called ONLY for unassigned drivers because
//  it broadcasts. It's better for performance
export function api_driver_set_position(point, callback_on_message, callback_on_coordinate_change)
{
  // Remember that this is a driver
  api_is_driver_ = 1;

  // If we never created local id for a driver then do it
  let is_first_time = 0;
  if (api_local_driver_id_ === undefined)
  {
    api_local_driver_id_ = Math.round(Math.random() * 1000000000);
    is_first_time = 1;
  }

  // This functions will be called when a message is received from the server
  api_ws_callback_on_message_received_ = callback_on_message;
  api_ws_callback_on_coordinate_change_ = callback_on_coordinate_change;

  if (!api_ws_in_authenticated_)
  {
    // For the first time call authenticate the driver
    // Authentication of a driver will allow the following:
    //
    // 1. Receive personal messages in a form of
    //  {"event":"personal_message","data":{"auth_string":"somestring" ... the rest of the message}}
    //  Note: coordinate update from a simulator is a personal message
    //  Note: notification of a rider about assigned driver's cooridinate updates will
    //    go through a separate message from the driver app whenever the driver coordinate
    //    changes. Why not from the simulator? Because that will be a production-ready
    //    solution when the coordinate is taken from the app (from GPS) and be delivered
    //    to a respective rider through a personal message
    //  Note: for unassigned drivers and riders coordinate still be delivered through
    //    the broadcast server because everybody needs to know unassigned from both sides
    //    to see all options
    //
    //  2. Send personal messages by a simulator or by a rider in the same form as above
    //  Note: auth_string for now for the driver is "driver-$driver_id" e.g. "driver-123456789"
    //  TODO! In a future is will be somethig more secure
    //
    // Authentication is just sending a string:
    //  {"event":"auth","data":{"auth_string":"somestring"}}

    api_websocket_authenticate_();
  }

  // Otherwise just update driver's coordinate
  api_websocket_send_(`{"event":"test_update_driver_position", "data":{"driver_id":${api_local_driver_id_},"lat":${point.latitude},"lon":${point.longitude}}}`);

  /* This is for test reasons - to have drivers accept orders quickly
  if (is_first_time)
  {
    api_driver_set_auto_accept_best_order_flag(1);
  }*/
} // function api_websocket_on_message_(e)

// Returns current order and driver from the recent update
export function api_get_current_order()
{
  return api_ws_current_order_;
}
export function api_get_current_driver()
{
  return api_ws_current_driver_;
}

// Set the best order auto accept flag for a driver
export function api_driver_set_auto_accept_best_order_flag(flag)
{
  // Remember that this is a driver
  api_is_driver_ = 1;

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

/*
*   API-SDK - external rider API
*/

// Sets point A for a rider
export function api_rider_set_A(pointA, callback_on_message)
{
  rider_A_ = pointA;

  api_ws_callback_on_message_received_ = callback_on_message;

  api_rider_set_AB_();
}

// Sets point B for a rider
export function api_rider_set_B(pointB, callback_on_message)
{
  rider_B_ = pointB;

  api_rider_set_AB_();
}


