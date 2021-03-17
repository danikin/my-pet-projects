/*
      SDK with the server
      All communications with server MUST be performed through this SDK

      (c) Dennis Anikin 2020
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
        console.log(`api_reinitialize_websocket_: reconnect websocket`);
        api_ws.connect();
    }
  }, 100);
}


/*
* Internal functions-hooks for the websocket
*/

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

// Places a message in a locl queue
// Note: a message is just stored in a local queue and is reliably sent to a server
//  once it's accessible
// Note: this functions can be can anytime event if the websocket is not created or
//  is not connected
function api_websocket_send_(message)
{
  console.log(`api_websocket_send_: put message to a pending queue ${api_websocket_send_}`);
  api_pending_messages_.push[message];

  // Try to send pending including the new one. It can work :-)
  api_websocket_send_pending_();
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


function api_websocket_on_message_(e)
{
  let message = JSON.parse(e.data);

  if (message.data === undefined)
  {
    // Bad message - skip it
    console.log(`api_websocket_on_message_: BAD message, without "data": ,message=${JSON.stringify(message)}, e=${JSON.stringify(e)}`);
    return;
  }

  // Try to match the current order with its local id
  // and the current driver with its local id


}


var api_is_driver_;
// This is one of the following:
//  1.  Current price view or order of a rider if this is a rider app
//      If it's undefined for a rider app then a rider is yet to register
//      or authenticate
//  2.  The best order or assigned order of a driver if this is a driver
//      If it's udefined for a driver app then there is no best order at all
var api_current_order;

// This is one of the following:
//  1.  A driver assigned to a current order of a rider if this is a rider app
//      If it's undefined for a rider app then there is either no order
//      for a rider - only a price view or even no price view (not registered or
//      not authenticated) 
//  2.  Current driver if this is a driver app
//      If it's undefined for a driver app then a driver is yet to register
//      or authenticate
var api_current_driver;

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

/*
*   Internal helper functions
*/

var rider_A_;
var rider_B_;
function api_rider_set_AB_()
{
  // If either of points is undefined then do nothing
  if (rider_A_ === undefined || rider_B_ === undefined)
    return;

  // Create a rider id locally if we haven't done it yet
  if (api_local_rider_id_ === undefined)
    api_local_driver_id_ = Math.round(Math.random() * 1000000000);

  // Send a message to the server
  api_websocket_send_(`"event":"test_price_view", "data":{"id":${api_local_rider_id_},"latA":${rider_A_.latitude},"lonA":${rider_A_.longitude},"latB":${rider_B_.latitude},"lonB":${rider_B_.longitude}}}`);
}

/*
*   Driver API
*/

export function api_driver_set_position(point)
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


/*
*   Rider API
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

