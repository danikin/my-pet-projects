/*
*	api_description.cpp
*
*	(C) Denis Anikin 2020
*
*	Description of API methods for the taxi service
*
*/


// TODO: push notification content and rules!!!

/*
 
        Contents:
 
    0.      Incoming asynchronous web-socket JSON messages to a user (rider <-, driver <-)
    I.      Outgoing web-socket JSON messages from a user (rider ->, driver ->)
    II.     Outgoing HTTP messages from a user (rider ->, driver ->)
    
    III.    Incoming asynchronous web-socket JSON messages to a rider (rider <-)
    VI.     Outgoing web-socket JSON messages from a rider (rider ->)
    V.      Outgoing HTTP messages from a rider (rider ->)
 
    VI.     Incoming asynchronous web-socket JSON messages to a driver (driver <-)
    VII.    Outgoing web-socket JSON messages from a driver (driver ->)
    VIII.   Outgoing HTTP messages from a driver (driver ->)
 
 */

/*

    0.      Incoming asynchronous web-socket JSON messages to a user (rider <-, driver <-)

*/

/* The general structure of incoming async web-socket messages is as the following */
{"event" : "update", "data" : {
    /* Chat between a user and support */
    "chat_with_support" : ...
    /* Rider flows (views and orders are inside). Described later in rider's section */
    "rider_flows" : ...
    /* Driver's best order. Described later in a driver section */
    "diver_best_order" : ...
    /* Driver's assigned order. Described later in a driver section */
    "diver_assigned_order" : ...
    /* Driver other orders. Described later in a driver section */
    "driver_other_orders" : ...
    }
}

/* Chat with support
    Note: in notifications only recent 24 hours chat is show. The rest can be obtained through an HTTP request
 */
{
    /* A text message from a user */
    {"ts" : 1234567190, "type" : "user_message", "message" : "Не работает"},
    /* A text message from support */
    {"ts" : 1234567290, "type" : "support_message", "message" : "Починили"},
    /* Current timestamp that this message was created */
    {"ts": 1234567995, "type" : "now"}
}

/*

       I.  Outgoing web-socket JSON messages from a user (rider ->, driver ->)

*/

/* Authentication by a token
    Note: rider_id and/or driver_id can be -1. That means that there not no rider/driver is attached to a user
 */
-> {"event" : "auth_by_token", "data" : {"token" : "blablabla"}}
<- {"event" : "auth_ok", "data" :
    {
        "token" : "blablabla",
        "user_id" : 123,
        "rider_id" : 456,
        "driver_id" : 789,
    }
}
<- {"event" : "auth_failed", "data" : {"reason" : "invalid_token"}}

/*
    A user has changed their location
 
    Note: if the user is a rider then all the flows for that rider with a is_A_floating == 1
            will be updates their A points and A addresses
    Note: if a user is not authentication in the web socket then the error messages is responsed otherwise
            there is NO RESPONSE
 
*/
-> {"event" : "pos_ch", "data" : {"user_id" : 123, "rider_id" : 456, "driver_id" : 456, "lon" : 55.1, "lat" : 111.2}}
<- {"event" : "pos_ch_error", "data" : {"reason" : "not_authenticated"}}

/*

       II.  Outgoing HTTP messages from a user (rider ->, driver ->)

*/

/* SMS send code */

->  /auth/sendsms?phone=+71234567890
<-  {"event"    :   "auth_sms_sent"}
    {"event"    :   "auth_sms_not_sent"}

/* SMS auth and token auth
    Note: if there is no rider or driver connected to a user then their id will be -1
 */

->  /auth/by_smscode?phone=+71234567890&smscode=1234
->  /auth/by_token?phone=+71234567890&token=blablabla
<-  {"event"    :   "auth_ok", "data" : {
            /* This token is used for the authentication and MUST be attached to any
                subsequential request to a server as "token" parameter in a query string */
            "token"     :   "blablabla",
            "user_id"   :   123,
            "rider_id"  :   456,
            "driver_id" :   789
        }
    }
<-  {"event"    :   "auth_error"}

/* Sends user's feedback. The response comes through websocket
    Note: each user has an endless chat with support with all history
*/
->  /misc/feedback?message=Не%20работает
<-  {"event"    :   "feedback_sent"}
    {"event"    :   "feedback_error"}

/* Returns all feedback chat for a user from the start */
->  /misc/get_feedback_chat
<-  {"event"    :   "feedback_chat_ok", "data" : {
        /* A text message from a user */
        {"ts" : 1234567190, "type" : "user_message", "message" : "Не работает"},
        /* A text message from support */
        {"ts" : 1234567290, "type" : "support_message", "message" : "Починили"},
        /* Current timestamp that this message was created */
        {"ts": 1234567995, "type" : "now"}
    }
}

/* Finds and autocomletes a street name or a POI
    Note: for the POI it returns it's address inside the POI name and its coordinates
 */

->  /geo/find_street?s=ле
<-  {"event"    :   "streets", "data" : [
    {"id" : 123, "name" : "Улица Ленина"},
    {"id" : 456, "name" : "Лесная улица"},
    {"id" : 789, "name" : "Ресторан 'Лето', Мясная 12", "lat" : 25.25, "lon" : 36.36}
    ]
}

/* Finds and autocomletes a house name
    Note: it returns coodinates of houses and coordinates of nearest road - that could be used as a pickup point
 */

->  /geo/find_house?s=Улица Ленина%202
<-  {"event"    :   "houses", "data" : [
    {"id" : 123, "name" : "Улица Ленина 21", "lat" : 25.25, "lon" : 36.36, "road_lat" : 25.25, "road_lon" : 35.36},
    {"id" : 456, "name" : "Улица Ленина 22", "lat" : 25.25, "lon" : 36.36, "road_lat" : 25.25, "road_lon" : 36.33},
    {"id" : 789, "name" : "Улица Ленина 23", "lat" : 25.25, "lon" : 36.36, "road_lat" : 25.25, "road_lon" : 37.34}
    ]
}

/*
 
        III.  Incoming asynchronous web-socket JSON messages to a rider (rider <-)
 
 */

/*
 
    Rider update.
 
    This JSON is sent from server to rider on a regular basis to
    update rider with recent changes
 
    Each item in "data" array is a flow. Normally one rider has one flow.
    "flow_stage" changes over time but "flow_id" does not change
 
    If there is a single flow then this array will always have one element that
    will change over time and its change will change from price_view to unassigned_order and then
    assigned_order
 
    The remaining fields for the flow depend on the flow_stage.
 
 */

{"event" : "update", "data" : {
    "rider_flows": [
              {"flow_id" : 12345, "flow_stage" : "price_view", ...},
              {"flow_id" : 12345, "flow_stage" : "price_view", ...},
              {"flow_id" : 12345, "flow_stage" : "unassigned_order", ...},
              {"flow_id" : 12345, "flow_stage" : "unassigned_order", ...},
              {"flow_id" : 12345, "flow_stage" : "assigned_order", ...},
              {"flow_id" : 12345, "flow_stage" : "assigned_order", ...}
        ]
    }
}

/*
 
    Rider update -> price_view flow
        Note: the following describes the structure of each flow in "flows" array
 
 */

{
    /* Globally identifies a flow */
    "flow_id"       :   1234,
    
    /* Timestamp of getting to this stage of the flow */
    "dt_added"      :   1234567
    
    /* Stage of the flow. Can be price_view, unassigned_order, assigned_order */
    "flow_stage"    :   "price_view",
    /* If 1 then A changes automatically online based on rider coordinates */
    "is_A_floating" :   1,
    /* Coords of A. Can be left out */
    "latA"          :   11.11,
    "lonA"          :   11.11,
    /* Address of A via reverse geocoding */
    "addr_A"        :   "Ленина 25",
    /* Coords of B. Can be left out */
    "latB"          :   11.11,
    "lonB"          :   11.11,
    /* Address of B via reverse geocoding */
    "addr_B"        :   "Маркса 19",
    
    /* Forecast distance and time of the trip */
    "forecast_distance_km" :    10,
    "forecast_duration_minutes" :   20,
    
    /* ETA (minutes), distance (km), price, id, pos for the driver who would have this order on top */
    /*  at the least price */
    /* Note: A left out -> ETA left out; A in place, B left out -> ETA is for A-to-A and "from" */
    /* Note: price for this ride in determinted by marketplace. It changes online */
    /* Note: if A is left out then fields below are also left out
    /* Note: if A is in place and B is left out then fields below are for A to A and */
    /*          the price shows to the end user as "from" */
    "price"              :   123.55,
    "best_pickup_ETA"   :   1.5,
    "best_driver_lat"   :   11.53,
    "best_driver_lon"   :   12.57,
    "best_pickup_distance"  :   0.75,
    "best_driver_id"    :   123,
    
    /* Options
        Note: options being turned on or off affect the price/ETA imediately
     */
    "no_smoking"        :   0,
    "need_childseat"    :   0,
    "only_cash"         :   1,
    "only_sbrebank_online"  :   0,
    
    /* Unassigned driver near A having and not having this order on top */
    "nearest_unassigned_drivers" :
    [
        {"lat" : 11.11, "lon" : 11.11, "has_this_on_top" : 1},
        {"lat" : 11.11, "lon" : 11.11, "has_this_on_top" : 0},
        {"lat" : 11.11, "lon" : 11.11, "has_this_on_top" : 0}
    ],
    
    /* Pickup points to attract the pin for A */
    "nearest_pickup_points" :
    [
        {"lat" : 11.11, "lon" : 11.11},
        {"lat" : 21.11, "lon" : 12.11},
        {"lat" : 17.11, "lon" : 15.11}
    ],
}

/*
 
    Rider update -> unassigned_order flow
        Note: the following describes the structure of each flow in "flows" array

*/

{
    /* Same meaning as with price_view */
    "flow_id"       :   1234,
    "dt_added"      :   1234567
    "flow_stage"    :   "unassigned_order",
    
    /* Coords of A. CAN'T be left out */
    "latA"          :   11.11,
    "lonA"          :   11.11,
    /* Address of A via reverse geocoding */
    "addr_A"        :   "Ленина 25",
    /* Coords of B. CAN'T be left out */
    "latB"          :   11.11,
    "lonB"          :   11.11,
    /* Address of B via reverse geocoding */
    "addr_B"        :   "Маркса 19",

    /* Same meaning as with price_view */
    "forecast_distance_km" :    10,
    "forecast_duration_minutes" :   20,
    
    /* Same meaning as with price_view */
    "no_smoking"        :   0,
    "need_childseat"    :   0,
    "only_cash"         :   1,
    "only_sbrebank_online"  :   0,
    
    /* Same meaning as with price_view */
    "price"              :   123.55,
    "best_pickup_ETA"   :   1.5,
    "best_driver_lat"   :   11.53,
    "best_driver_lon"   :   12.57,
    "best_pickup_distance"  :   0.75,
    "best_driver_id"    :   123,
    
    /* The price that marketplace suggests to stimulate the driver */
    "suggested_price"   :   145.88
}


/*
 
    Rider update -> assigned_order flow
        Note: the following describes the structure of each flow in "flows" array

 */

{
    /* Same meaning as with unassigned_order */
    "flow_id"       :   1234,
    "dt_added"      :   1234567
    "flow_stage"    :   "assigned_order",
    "latA"          :   11.11,
    "lonA"          :   11.11,
    "addr_A"        :   "Ленина 25",
    "latB"          :   11.11,
    "lonB"          :   11.11,
    "addr_B"        :   "Маркса 19",

    /* Same meaning as with price_view
        Note: it changes dynamically over time
     */
    "forecast_distance_km" :    10,
    "forecast_duration_minutes" :   20,

    /* Same meaning as with unassigned_order */
    "no_smoking"        :   0,
    "need_childseat"    :   0,
    "only_cash"         :   1,
    "only_sbrebank_online"  :   0,

    /* Same meaning as with unassigned_order */
    "price"              :   123.55,
    "best_pickup_ETA"   :   1.5,
    "best_driver_lat"   :   11.53,
    "best_driver_lon"   :   12.57,
    "best_pickup_distance"  :   0.75,
    "best_driver_id"    :   123,
    
    /* Phone to connect the driver */
    "driver_phone"      :   "+79999999999",
    /* Driver's name, lastname, car, license */
    "driver_name"       :   "Ivan",
    "driver_lastname"   :   "Ivanov",
    "driver_car_make"   :   "Kia",
    "driver_car_model"  :   "Rio",
    "driver_car_color"  :   "Yellow",
    "driver_license_plate"    :   "X000XX999",
    "driver_photo_url"    :   "https://blablabla",

    /* Forecast track of the driver */
    "forecast_driver_track" :   [
        {"lan" : 11.11, "lot" : 11.11},
        {"lan" : 11.11, "lot" : 11.11},
        {"lan" : 11.11, "lot" : 11.11},
        {"lan" : 11.11, "lot" : 11.11}
    ],
    
    /* Minutes since toll wait has started */
    "toll_wait_minutes_elapsed" :   2,
    /* Total toll minutes */
    "toll_wait_mintes_total" :   3,
    /* Total charge for toll wait */
    "toll_wait_charge" :   12,
    
    /* Driver's manual forecast of ETA to A */
    "driver_ETA_to_A_forecast" : 5,

    /*
     
            A chat between a rider and the other party - driver
     
            
     */
    "rider_chat"              :   [
        /* A text message from a rider */
        {"ts" : 1234567190, "type" : "rider_message", "message" : "Вы скоро будете?"},
        /* A text message from a driver */
        {"ts" : 1234567290, "type" : "driver_message", "message" : "На подлете"},
        /* A system message from a driver about ETA forecast */
        {"ts" : 1234567390, "type" : "driver_ETA",  "message" : 2},
        /* A system message from a driver about getting A */
        {"ts" : 1234567390, "type" : "driver_got_A"},
        /* A system message from marketplace about beginning of the toll wait along with its price */
        {"ts" : 1234567490 , "type" : "toll_wait_began", "wait_toll_per_minute" : 4},
        /* A text message from a rider sent by a shortcut */
        {"ts" : 1234567590, "type" : "rider_message", "Скоро выйду"},
        /* A system message from a driver about starting a ride */
        {"ts" : 1234567690, "type" : "driver_started_ride"},
        /* A system message from a driver about their decision to charge the wait toll */
        {"ts" : 1234567490 , "type" : "wait_toll_charged"},
        /* A system message from a driver about getting B and waiting cash payment with the breakdown of payment */
        {"ts" : 1234567790, "type" : "driver_got_B_wait_paymen_cash", "total" : 123,
                             "toll_wait_included": 8, "toll_wait_duration" : 2, "ride_duration": 23},
        /* A system message from a driver about getting B and waiting sber payment along with
            the phone to pay and with the breakdown of payment */
        {"ts" : 1234567890, "type" : "driver_got_B_wait_paymen_sbert", "total" : 123,
                             "toll_wait_included": 8, "toll_wait_duration" : 2, "ride_duration": 23,  "phone" : "+79169999999"},
        /* A system message from a driver about the payment done */
        {"ts" : 1234567990, "type" : "payment_done_ride_finished"},
        /* Current timestamp that this message was created */
        {"ts": 1234567995, "type" : "now"}
    ]
}

/*

       IV.  Outgoing web-socket JSON messages from a rider (rider ->)

*/


/*

       V.  Outgoing HTTP messages from a rider (rider ->)
 
        Note: EVERY message has a paramater "token" which is checked in the first place
        Note: if token check is failed then the response is ALWAYS {"event", "auth_error"}

*/

/*
 
    Before price view stage

 */

/* Rider registration. What it does it's just says to the server that hey this user will be a rider :-) */

->  /rider/reg
<-  {"event"    :   "rider_reg_ok"}
    {"event"    :   "rider_reg_failed"}

/*

    In price view stage

*/

/* Returns a list of personal addresses for this rider
 */

->  /geo/get_personal_addresses
<-  {"event"    :   "personal_addresses", "data" : [
    {"id" : 123, "name" : "Улица Ленина 37", "lat" : 25.25, "lon" : 36.36},
    {"id" : 456, "name" : "Лесная улица 25/1", "lat" : 25.25, "lon" : 36.36},
    {"id" : 789, "name" : "Ресторан 'Лето', Мясная 12", "lat" : 25.25, "lon" : 36.36}
    ]
}

/* Moves point A to the new position (through Pin)
    Note: the server will change the address automatically and notifies the user about the change through
        the update event
    Note: if is_A_floating == 1 for this flow then it does nothing and returns an error
 */
->  /rider/move_A?flow_id=123&lat=11.11&lon=12.12
<-  {"event"    :   "move_A_ok"}
<-  {"event"    :   "move_A_error"}

/* Moves point B to the new position (through Pin)
    Note: the server will change the address automatically and notifies the user about the change through
        the update event
 */
->  /rider/move_B?flow_id=123&lat=11.11&lon=12.12
<-  {"event"    :   "move_B_ok"}
<-  {"event"    :   "move_B_error"}

/* Floats or unfloats A point for the specified flow
 */
->  /rider/float_A?flow_id=123&is_A_floating=1
->  /rider/float_A?flow_id=123&is_A_floating=0
<-  {"event"    :   "float_A_ok"}
<-  {"event"    :   "float_A_error"}

/* Changes order options
    Note: each parameter beyond flow_id is an order option - it can be 0 or 1
*/
->  /rider/change_options?flow_id=123&no_smoking=1&need_childseat=1&only_cash=1&only_sbrebank_online=1
<-  {"event"    :   "change_options_ok"}
<-  {"event"    :   "change_options_error"}

/* Places an order for the specified flow
    Note: if order is placed then the flow will change the stage to unassigned_order and the client
        will be notified about it in the next update event
*/
->  /rider/place_order?flow_id=123
<-  {"event"    :   "place_order_ok"}
<-  {"event"    :   "place_order_error"}

/*
 
 In unassigned_order stage
 
 */

/* Increases the price of an order to stimulate the driver to accept it
 */
-> /rider/increase_order_price?flow_id=123&new_price=250
<-  {"event"    :   "increase_order_price_ok"}
<-  {"event"    :   "increase_order_price_error"}

/* Cancels an order
 */
-> /rider/cancel_order?flow_id=123
<-  {"event"    :   "cancel_order_ok"}
<-  {"event"    :   "cancel_order_error"}

/*
 
 In assigned_order stage
 
*/

/* Messages a driver
 */
-> /rider/message_driver?flow_id=123&message=Вы%20где
<-  {"event"    :   "message_driver_ok"}
<-  {"event"    :   "message_driver_error"}

/* Cancels an order (same as in unassigned_order stage)
 */
-> /rider/cancel_order?flow_id=123

// TODO: integration with Sberbank Online



/*

        VI.     Incoming asynchronous web-socket JSON messages to a driver (driver <-)

*/
  
{"event" : "update", "data" :
    {
        /* Information about the best unassigned order
            Note: this object comes even when a driver takes another order, but until they got A
         */
        "diver_best_order" : {
            /* Globally identifies an order
                Note: it's actually a flow_id from a rider :-)
                Note: if there is no best order then order_id is -1, is_assigned is 0 and other fields are omitted
             */
            "order_id"      :   123,

            /* Pickup point
                Note: district_A is for driver's convenience
             */
            "latA"          :   11.11,
            "lonA"          :   11.11,
            "addr_A"        :   "Ленина 25",
            "district_A"    :   "Центр",
            
            /* Drop-off point
                Note: district_B is for driver's convenience
             */
            "latB"          :   11.11,
            "lonB"          :   11.11,
            "addr_B"        :   "Маркса 19",
            "district_B"    :   "Трудовой славы",
            
            /* Forecast distance and duration of the trip */
            "forecast_distance_km" :    10,
            "forecast_duration_minutes" :   20,

            /* Real distance and duration of the trip
                Note: this info is set at the end of the trip
             */
            "real_distance_km" :    15,
            "real_duration_minutes" :   35,
            
            /* Minutes since toll wait has started */
            "toll_wait_minutes_elapsed" :   2,
            /* Total toll minutes */
            "toll_wait_mintes_total" :   3,
            /* Total charge for toll wait */
            "toll_wait_charge" :   12,
            
            /* Order options. Note: they are already compatible with driver's options */
            "no_smoking"        :   0,
            "need_childseat"    :   0,
            "only_cash"         :   1,
            "only_sbrebank_online"  :   0,

            /* The price
                Note: the price can be changed if a rider accepts a suggested price
             */
            "price"              :   123.55,
        },

        
        /* If a driver has an assigned order then it's in the following object otherwise this object is omitted
            Note: if a rider cancels an order then a driver_assigned_order object disapears - this will tell the app that
                    an order is cancelled
            Note: if the driver has auto_accept option "on" then this object will come out of the blue in the next update and
                the app should handle this with a message to a driver that the order has been auto-accepted
         */
  
        "driver_assigned_order" : {
            /* Same fields as for driver_best_order */
        }

        /* Information about other orders close to the best one but not as best :-) */
        "driver_other_orders" : [
            /* Here comes an array of object with the same structure and same meaning as diver_order above */
        ]
        
        /*
     
            A chat between a driver and the other party - rider
                Note: no system messages from a driver here because the driver does not need them
            
         */
        "driver_chat"  :   [
            /* A text message from a rider */
            {"ts" : 1234567190, "type" : "rider_message", "message" : "Вы скоро будете?"},
            /* A text message from a driver */
            {"ts" : 1234567290, "type" : "driver_message", "message" : "На подлете"},
            /* A text message from a rider sent by a shortcut */
            {"ts" : 1234567590, "type" : "rider_message", "Скоро выйду"},
            /* A system message from marketplace about beginning of the toll wait along with its price */
            {"ts" : 1234567490 , "type" : "toll_wait_began", "wait_toll_per_minute" : 4},
            /* Current timestamp that this message was created */
            {"ts": 1234567995, "type" : "now"}
        ]
    }
}

/*
 
        VII.    Outgoing web-socket JSON messages from a driver (driver ->)

*/

/*
 
        VIII.   Outgoing HTTP messages from a driver (driver ->)
 
                Note: EVERY message has a paramater "token" which is checked in the first place
                Note: if token check is failed then the response is ALWAYS {"event", "auth_error"}
 
 */

/*

    Once
 
*/

/* Registers a driver along with all the data
 */
-> /driver/reg?name=Иван&lastname=Иванов&car_make=Kia&car_model=Rio&car_year=2018&car_color=white&car_lisence_plate=X000XX999&driver_photo=https://blablabla
<-  {"event"    :   "driver_reg_ok"}
<-  {"event"    :   "driver_reg_error"}

/*

    Anytime
 
*/

/* Changes driver's data (everything except first and last names)
    Note: it changes only those parameters specified
*/

-> /driver/change_data?car_make=Kia&car_model=Rio&car_year=2018&car_color=white&car_lisence_plate=X000XX999&driver_photo=https://blablabla
<-  {"event"    :   "driver_change_data_ok"}
<-  {"event"    :   "driver_change_data_error"}

/* Returns driver's data
*/

-> /driver/get_data
<-  {"event"    :   "driver_get_data_ok", "data" : {
        "driver_phone"      :   "+79999999999",
        "driver_name"       :   "Ivan",
        "driver_lastname"   :   "Ivanov",
        "driver_car_make"   :   "Kia",
        "driver_car_model"  :   "Rio",
        "driver_car_color"  :   "Yellow",
        "driver_license_plate"    :   "X000XX999",
        "driver_photo_url"    :   "https://blablabla"
    }
}
<-  {"event"    :   "driver_get_data_error"}

/* Set driver's own tarif
    Note: if it's called without parameters then own tarif is cancelled
*/

-> /driver/set_own_tarif?min_price=89&price_per_km=5&price_per_minute=3
<-  {"event"    :   "change_own_tarif_ok"}
<-  {"event"    :   "change_own_tarif_error"}

/* Returns driver's tarif
    Note: if there is not own tarif then fields are just omitted
*/

-> /driver/get_own_tarif
<-  {"event"    :   "get_own_tarif_ok", "data" : {
        "min_price"      :   89,
        "price_per_km"       :   5,
        "price_per_minute"   :   3

    }
}
<-  {"event"    :   "get_own_tarif_error"}

/* Set driver's settings
    Note: it changes only those parameters specified
*/

-> /driver/set_settings?notify_new_order=1&notify_suggested_price_up=1&notify_high_demand=1&push=1&sound=1&smoke_allowed=1&childseat=1&no_push_after=21&no_push_before=9
<-  {"event"    :   "set_settings_ok"}
<-  {"event"    :   "set_settings_error"}

/* Returns driver's settings
*/

-> /driver/get_settings
<-  {"event"    :   "get_settings_ok", "data" : {
        "notify_new_order"      :   1,
        "notify_suggested_price_up"       :   1,
        "notify_high_demand"   :   1,
        "push"              :   1,
        "sound"             :   1,
        "smoke_allowed"     :   1,
        "childseat"         :   1,
        "no_push_after"     :   1,
        "no_push_before"    :   1
    }
}
<-  {"event"    :   "get_settings_error"}

/* Set driver's other settings and filters
    Note: it changes only those parameters specified
*/

-> /driver/set_other_settings_and_filters?max_distance_A=2&max_distance_B=10&no_cash=0&no_sbrebank_online=0&auto_accept_order=1&personal_address=Ленина%2015&use_personal_address=0
<-  {"event"    :   "set_other_settings_and_filters_ok"}
<-  {"event"    :   "set_other_settings_and_filters_error"}

/* Returns driver's other settings and filters
*/

-> /driver/get_other_settings_and_filters
<-  {"event"    :   "get_other_settings_and_filters_ok", "data" : {
        {"max_distance_A"        :   1},
        {"max_distance_B"        :   1},
        {"no_cash"               :   0},
        {"no_sbrebank_online"    :   0},
        {"auto_accept_order"     :   1},
        {"personal_address"      :   "Ленина 15"},
        {"use_personal_address"      :   0}
    }
}
<-  {"event"    :   "get_other_settings_and_filters_error"}

/* Returns local district list
*/
-> /driver/get_local_districts
<-  {"event"    :   "get_local_districts_ok", "data" : [
        {"district_id" : 123, "district_name" : "Трехозерка"},
        {"district_id" : 456, "district_name" : "Краснозатонский"},
        {"district_id" : 789, "district_name" : "Седкыркещ"},
        {"district_id" : 012, "district_name" : "Верхний Чов"},
        {"district_id" : 345, "district_name" : "Верхняя Максаковка"},
        {"district_id" : 678, "district_name" : "Выльгорт"}
    ]
}
<-  {"event"    :   "get_local_districts_error"}

/* Set driver's bad districts
    Note: A_ids is a list of bad districts for A and B_ids the same for B. If a list is empty then there is no bad districts
*/

-> /driver/set_bad_districts?ids_A=123,012,678&ids_B=123,345
<-  {"event"    :   "set_bad_districts_ok"}
<-  {"event"    :   "set_bad_districts_error"}

/* Returns driver's bad districts
*/
-> /driver/get_bad_districts
<-  {"event"    :   "get_bad_districts_ok", "data" : [
        {"district_id" : 123, "bad_A" : 1, "bad_B" : 1},
        {"district_id" : 456, "bad_A" : 1, "bad_B" : 1},
        {"district_id" : 789, "bad_A" : 1, "bad_B" : 1},
        {"district_id" : 012, "bad_A" : 1, "bad_B" : 1},
        {"district_id" : 345, "bad_A" : 1, "bad_B" : 0},
        {"district_id" : 678, "bad_A" : 0, "bad_B" : 0}
    ]
}
<-  {"event"    :   "get_bad_districts_error"}

/* Returns driver's statistics
*/
-> /driver/get_stat?date_from=03-08-2020&date_to=09-08-2020&aggregate=0
<-  {"event"    :   "get_stat_ok", "data" : [
        {"date" : "03-08-20", "revenue" : 2301, "rides" : 14},
        {"date" : "04-08-20", "revenue" : 2950, "rides" : 14},
        {"date" : "05-08-20", "revenue" : 2932, "rides" : 19, "toll" : 150, "is_paid" : 0},
        {"date" : "06-08-20", "revenue" : 1655, "rides" : 9},
        {"date" : "07-08-20", "revenue" : 2655, "rides" : 21, "toll" : 150, "is_paid" : 1},
        {"date" : "08-08-20", "revenue" : 2878, "rides" : 17, "toll" : 150, "is_paid" : 1}
    ]
}
-> /driver/get_stat?date_from=01-07-2020&date_to=02-08-2020&aggregate=1
<-  {"event"    :   "get_stat_ok", "data" : {
        {"date_from" : "01-07-2020"},
        {"date_to" : "02-08-2020"},
        {"revenue" : 43780},
        {"rides" : 471},
        {"toll" : 3150},
        {"debt" : 300}
    }
}
<-  {"event"    :   "get_stat_error"}

// TODO: integration with external payment systems


/*

    Only when there is no assigned order
 
*/

/* Accepts the specified order
    Note: if this order is already accepted or not an order yet then error is returned
*/
-> /driver/accept_order?order_id=123
<-  {"event"    :   "driver_accept_order_ok"}
<-  {"event"    :   "driver_accept_order_error"}

/*

    Only when there is an assigned order
 
*/

/* Cancels current assigned order
    Note: if there is no assigned order then returns error
    Note: if there is not a good moment to cancel (between got A and toll wait started) then returns error
*/
-> /driver/cancel_order
<-  {"event"    :   "driver_cancel_order_ok"},
<-  {"event"    :   "driver_cancel_order_error"}

// TODO: integration with navigators + link to navigators

/* If a driver wants to manually forecast ETA then this method is called
    Note:
*/
-> /driver/manual_ETA_forecast?ETA=3
<-  {"event"    :   "driver_manual_ETA_forecast_ok"},
<-  {"event"    :   "driver_manual_ETA_forecast_error"}

/* Driver confirms getting to the pickup point
    If ignore_too_far is 0 and the driver is too far from the pickup point then nothing will happen
    If ignore_too_far is 1 then this will overcome server's warning that a driver is too far
    If the driver is too far then anyway driver_got_A_ok_too_far will be returned
*/
-> /driver/got_A?ignore_too_far=0
<-  {"event"    :   "driver_got_A_ok"},
<-  {"event"    :   "driver_got_A_ok_too_far"}
<-  {"event"    :   "driver_got_A_error"}

/* Returns a mobile phone number to call a rider. The phone is active until the order is finished and 1 hour after that */
-> /driver/get_rider_phone
<-  {"event"    :   "driver_get_rider_phone_ok", "data" : {"+79169999999"}},
<-  {"event"    :   "driver_get_rider_phone_error"}

/* Driver confirms that a ride has started. It works only if the driver has confirmed got_A before */
-> /driver/ride_started
<-  {"event"    :   "driver_ride_started_ok"},
<-  {"event"    :   "driver_ride_started_error"}

/* Driver accepts or forgives wait toll has confirmed got_A before */
-> /driver/accept_wait_toll_charge
<-  {"event"    :   "driver_accept_wait_toll_charge_ok"},
<-  {"event"    :   "driver_accept_wait_toll_charge_error"}

-> /driver/forgive_wait_toll_charge
<-  {"event"    :   "driver_forgive_wait_toll_charge_ok"},
<-  {"event"    :   "driver_forgive_wait_toll_charge_error"}

/* Driver confirms getting to the drop-off point
    If ignore_too_far is 0 and the driver is too far from the drop-off point then nothing will happen
    If ignore_too_far is 1 then this will overcome server's warning that a driver is too far
    If the driver is too far then anyway driver_got_A_ok_too_far will be returned
*/
-> /driver/got_B?ignore_too_far=0
<-  {"event"    :   "driver_got_B_ok"},
<-  {"event"    :   "driver_got_B_ok_too_far"}
<-  {"event"    :   "driver_got_B_error"}

/* Driver confirms the the order is closed
    Note: it only works if the driver has confirmed got_B before
    Note: in the next update the driver_assigned_order ovject disapears
*/
-> /driver/close
<-  {"event"    :   "driver_close_ok"},
<-  {"event"    :   "driver_close_error"}
