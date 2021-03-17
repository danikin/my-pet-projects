/*
      SDK with the server
      All communications with server MUST be performed through this SDK

      (c) Dennis Anikin 2020
*/

/*
*   Websocket layer - internals
*/

var api_ws;

// Internal function that initializes websocket
function api_initialize_websocket_()
{
    if (api_ws === undefined)
    {
      console.log(`api_initialize_websocket_`);

      api_ws = new WebSocket('ws://185.241.194.113:80');;
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
  console.log(`api_reinitialize_websocket_`)
  setTimeout(function()
  {
    if (api_ws === undefined)
    {
          console.log(`api_reinitialize_websocket_: reinitialize websocket`);
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
        {
          console.log(`api_reinitialize_websocket_: reconnect websocket`);

          // Close the connections
          api_ws.close();

          // Initilize the websocket again
          // Note: it seems like the only way to connect a websocket is to creat a new
          // instance of the class WebSocket
          api_initialize_websocket_();
        }
    }
  }, 100);
}

var api_pending_messages_ = [];

function api_websocket_onopen_()
{
  console.log(`api_websocket_onopen_`);

  // Try to send pending messages
  api_websocket_send_pending_();
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
  console.log(`api_websocket_send_all_pending_: queue size is ${api_pending_messages_.length}`);

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

var api_callback_on_message_received;

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

// Processes a message from the server and call callbacks
function api_websocket_on_message_(e)
{
  //console.log(`api_websocket_on_message_(${JSON.stringify(e)})`);

  // Receive a message and parse it
  let message = JSON.parse(e.data);
  if (message.data === undefined)
  {
    // Bad message - skip it
    console.log(`api_websocket_on_message_: BAD message, without "data": ,message=${JSON.stringify(message)}, e=${JSON.stringify(e)}`);
    return;
  }

  // For a driver app
  if (api_local_driver_id_ !== undefined)
  {
    if (message.data.drivers !== undefined)
    {
      // Iterate drivers
      let driver_to_return;
      for (let driver of message.data.drivers)
      {
        // Save current driver and notify about it's possible change of properties
        if (driver.driver_id == api_local_driver_id_)
        {
          driver_to_return = driver;
          break;
        }
      }

      //console.log(`api_websocket_on_message_: api_local_driver_id_=${api_local_driver_id_}, driver_to_return = (${JSON.stringify(driver_to_return)})`);
      
      // Now iterate riders to find the best or assigned order for a driver
      let rider_to_return;
      if (driver_to_return !== undefined && message.data.riders !== undefined)
        for (let rider of message.data.riders)
        {
          if (driver_to_return.best_rider_id == rider.rider_id ||
              (driver_to_return.assigned_order_id !== undefined && // This is for not to have two equal undefs
                driver_to_return.assigned_order_id == rider.assigned_order_id))
          {
            rider_to_return = rider;
            break;
          }
        }

      //console.log(`api_websocket_on_message_: api_local_driver_id_=${api_local_driver_id_} rider_to_return = (${JSON.stringify(rider_to_return)})`);
      
      // Check on data consistency
      if (driver_to_return !== undefined)
      {
        if (driver_to_return.best_rider_id !== undefined && driver_to_return.best_rider_id != -1 &&
          rider_to_return === undefined)
          {
            console.log(`api_websocket_on_message_: DATA INCONSISTENCY IN SERVER RESPONSE! driver_to_return.best_rider_id=${driver_to_return.best_rider_id}, but the rider is not found in the response`);
            // Unset the driver
            driver_to_return = undefined;
          }

        if (driver_to_return.assigned_order_id !== undefined && driver_to_return.assigned_order_id != -1 &&
          rider_to_return === undefined)
          {
            console.log(`api_websocket_on_message_: DATA INCONSISTENCY IN SERVER RESPONSE! driver_to_return.assigned_order_id=${driver_to_return.assigned_order_id}, but the rider is not found in the response`);
            
            // Unset the driver
            driver_to_return = undefined;
          }
      }

      // If at least a driver to return then call a callback
      if (driver_to_return !== undefined)
      {
        if (api_callback_on_message_received === undefined)
        {
          console.log(`api_websocket_on_message_: NO CALLBACK! Can't return: driver_to_return=${JSON.stringify(driver_to_return)}, rider_to_return=${JSON.stringify(rider_to_return)}`);
        }
        else
          api_callback_on_message_received(driver_to_return, rider_to_return);
      }

    } // if (message.data.drivers !== undefined)

  } // if (api_local_driver_id_ !== undefined)

  /*// For a rider app
  if (api_local_rider_id_ !== undefined)
  {
    // Iterate riders
    if (message.data.riders !== undefined)
      for (let rider of message.data.riders)
      {
        if (rider.rider_id == api_local_rider_id_)
        {
          api_rider_on_rider(rider);
        }
      }

    // Drivers - ?
  }

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

}

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
  api_websocket_send_(`"event":"test_price_view", "data":{"id":${api_local_rider_id_},"latA":${rider_A_.latitude},"lonA":${rider_A_.longitude},"latB":${rider_B_.latitude},"lonB":${rider_B_.longitude}}}`);
}

/*
*   API-SDK - external driver API
*/

// Sets driver's position
// Note: this is the only function in API that receives a callback
//  Once the callback is received by a caller it can use since then
export function api_driver_set_position(point, callback)
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
  api_callback_on_message_received = callback;

  // Otherwise just update driver's coordinate
  api_websocket_send_(`{"event":"test_update_driver_position", "data":{"driver_id":${api_local_driver_id_},"lat":${point.latitude},"lon":${point.longitude}}}`);

  /* This is for test reasons - to have drivers accept orders quickly
  if (is_first_time)
  {
    api_driver_set_auto_accept_best_order_flag(1);
  }*/
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

/*
*   API-SDK - external rider API
*/

// Sets point A for a rider
export function api_rider_set_A(pointA)
{
  rider_A_ = pointA;

  api_rider_set_AB_();
}

// Sets point B for a rider
export function api_rider_set_B(pointB)
{
  rider_B_ = pointB;

  api_rider_set_AB_();
}


