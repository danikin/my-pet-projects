/*
*       marketplace_api_db.cpp
*
*       (C) Denis Anikin 2020
*
*       Persistance layer for marketplace API
*
*/

void upload_data_from_db()
{


/*
Session sess(mysql_url);

    // Get driver info
    RowResult res = sess.sql("\
\
select drivers.driver_id, top_order_id, metric, latitude, longitude, dt_position_updated, gas_price_per_meter\
\n\
from anikin_taxi_marketplace.drivers drivers,\n\
anikin_taxi_marketplace.driver_settings driver_settings,\n\
anikin_taxi_general.users users,\n\
anikin_taxi_marketplace.cars cars\n\
\n\
\n\
where\n\
\n\
 drivers.user_id=users.user_id\n\
 and drivers.driver_id=driver_settings.driver_id\n\
 and drivers.current_settings_id=driver_settings.driver_settings_id\n\
 and drivers.driver_id=cars.driver_id\n\
 and drivers.is_online=1\n\
 and cars.is_car_current=1\n\
").execute();

    // Upload drivers
    while (true)
    {
        Row row = res.fetchOne();
        if (!row)
            break;

        driver d;
        d.id_ = row["drivers.driver_id"];
        d.erased_ = false;
        d.position_where_.latitude = row["latitude"];
        d.position_where_.longitude = row["longitude"];
        d.cost_per_km_ = row["gas_price_per_meter"];

        std::string metric = row[2].operator std::string();

        if (metric == "revenue_per_second")
            d.metric_type_ = driver::driver_metric_revenue_per_second;
        if (metric == "revenue_per_meter")
            d.metric_type_ = driver::driver_metric_revenue_per_m;
        if (metric == "profit_per_second")
            d.metric_type_ = driver::driver_metric_profit_per_second;

        impl_.driver_online(d);
    }

    // Get all the orders for last 24 hours in status before placement. Basically that's price view
    res = sess.sql(
"\n\
SELECT order_id, rider_id,\n\
    point_A_latitude, point_A_longitude, point_B_latitude, point_B_longitude,\n\
        ETA_A_to_B,\n\
        distance,\n\
        price_as_initially_shown price\n\
\n\
FROM\n\
    anikin_taxi_marketplace.orders orders\n\
WHERE dt_price_requested >= DATE_ADD(NOW(), INTERVAL -1 DAY) and order_status < 'order_placed'\n\
"

).execute();

    // Upload price views
    while (true)
    {
        Row row = res.fetchOne();
        if (!row)
            break;

        order o;
        o.id_ = row["order_id"];
        o.rider_id_ = row["rider_id"];
        o.A_.latitude = row["point_A_latitude"];
        o.A_.longitude = row["point_A_longitude"];
        o.B_.latitude = row["point_B_latitude"];
        o.B_.longitude = row["point_B_longitude"];
        o.ETA_ = row["ETA_A_to_B"];
        o.distance_km_ = row["distance"];
        o.price_ = row["price"];
        o.erased_ = false;
        impl_.add_price_view(o);
    }

    // Get all placed but unassigned orders for last 15 minutes
    // Why 15 minutes? Because a rider is unlikely to wait more than 15 minutes for order acceptance
    res = sess.sql(
"\n\
SELECT\n\
        order_id, rider_id,\n\
        point_A_latitude, point_A_longitude, point_B_latitude, point_B_longitude,\n\
        ETA_A_to_B,\n\
        distance,\n\
        coalesce(price_as_changed_after_placed, price_as_placed) price\n\
FROM\n\
    anikin_taxi_marketplace.orders orders\n\
WHERE dt_order_placed >= DATE_ADD(NOW(), INTERVAL -15 MINUTE) and\n\
    (order_status >= 'order_placed' and order_status < 'order_accepted')\n\
"
).execute();

    // Upload unassigned orders
    while (true)
    {
        Row row = res.fetchOne();
                if (!row)
                        break;

        order o;
        o.id_ = row["order_id"];
        o.rider_id_ = row["rider_id"];
        o.A_.latitude = row["point_A_latitude"];
        o.A_.longitude = row["point_A_longitude"];
        o.B_.latitude = row["point_B_latitude"];
        o.B_.longitude = row["point_B_longitude"];
        o.ETA_ = row["ETA_A_to_B"];
        o.distance_km_ = row["distance"];
        o.price_ = row["price"];
        o.erased_ = false;
    
        impl_.add_unassigned_order(o);
    }

    printf("marketplace_api::marketplace_api: uploading marketplace data ... done\n");
    fflush(stdout);*/
}

