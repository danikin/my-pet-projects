/*
*	marketplace.h
*
*	(C) Denis Anikin 2020
*
*	Headers for marketplace
*
*/

#ifndef _marketplace_h_included_
#define _marketplace_h_included_

#include <list>
#include <time.h>
#include <vector>
#include <unordered_map>

#include "geolocation.h"
#include "driver_billing.h"

namespace marketplace
{

using namespace geo;

// Marketplace params
// TODO: make it configurable
struct params
{
	// Timeout after which the price view is expired
	// Should be long enough because a rider can forget about a potential ride for a while :-)
	static int price_view_expiration_time() { return 24 * 3600; }

	// Timeout after which the unassigned order is expired
	// Should be short enough because if a rider forgets about this then the rider will go nowhere
	static int unassigned_order_expiration_time() { return 300; }

	// How often to rebalance the marketplace. The operation could be costy for big towns
	static int rebalance_timeout() { return 1; }

	// How much do we need to increase the price to beat the top order?
	// It should be some little amount like 10 RUB
	static int price_adjust_step() { return 10; }

	// Average speed in km/hr
    static int average_speed_in_town() { return 30; }

    // Returns interval of freshness of coordinates before a driver is erased from marketplace
    static int position_freshness_time() { return 120; }

    // Wait before the toll starts
    static int free_wait_duration() { return 180; }
    
    // Price per toll minute
    static double toll_wait_minute_price() { return 4; }
    
    // Cost per km for a driver including gas, rent, her/his salare
    static double cost_per_km() { return 14; }
};

// Filters on a driver side
class driver_side_filters
{
public:
    
    driver_side_filters() : pass_everything_(true), auto_accept_best_order_(false) {}
    
    // Turns all filters off
    bool pass_everything_;
    
    bool auto_accept_best_order_;
    
    // Max pickup distance
    double max_distance_to_A_km_;
    
    // Max travel distance
    double max_distance_to_B_km_;
    
    // The driver does not accept cash
    bool no_cash_;
    // The driver does not accept sberbank online
    bool no_sbrebank_online_;
    
    // Only take orders with B from around home area
    bool B_only_around_home_;
    point home_;
    double home_area_radius_km_;
    
    // Lists of bad districts for pickup and dropoff points
    bool bad_districts_on_;
    // TODO: consider changing std::vector to std:array for performance reasons
    std::vector<point> bad_districts_A_;
    std::vector<point> bad_districts_B_;
    double bad_district_radius_km_;
    
    /*bool own_tarif_on_;
    double own_tarif_minimum_price_;
    double price_per_km_;
    double price_per_min_;*/
    
    // The car is with smoking allowed
    bool smoking_allowed_;
    // The driver has a child seat
    bool chidseat_;
    // The driver does not accept pets in the car
    bool no_pets_;
    // The driver does not accept big parties (4+ people)
    bool no_big_parties_;
    
    // Details on how the order ws filters
    struct filtering_result
    {
        bool by_max_distance_to_A_;
        bool by_max_distance_to_B_;
        bool by_payment_method_;
        bool by_home_area_;
        bool by_bad_district_A_;
        bool by_bad_district_B_;
        bool by_smoking_;
        bool by_childseat_;
        
        friend std::ostream& operator<<(std::ostream& out, const filtering_result &fr)
        {
            return out
            << "by_max_distance_to_A: " << fr.by_max_distance_to_A_ << " "
            << "by_max_distance_to_B: " << fr.by_max_distance_to_B_ << " "
            << "by_payment_method: " << fr.by_payment_method_ << " "
            << "by_home_area: " << fr.by_home_area_ << " "
            << "by_bad_district_A: " << fr.by_bad_district_A_ << " "
            << "by_bad_district_B: " << fr.by_bad_district_B_ << " "
            << "by_smoking: " << fr.by_smoking_ << " "
            << "by_childseat: " << fr.by_childseat_ << " "
            ;
        }
    } fr_;
};

// Filters on a rider side
class rider_side_filters
{
public:
    
    rider_side_filters() : pass_everything_(true) {}
    
    // Turns all filters off
    bool pass_everything_;
    
    bool no_smoking_;
    bool need_childseat_;
    bool only_cash_;
    bool only_sbrebank_online_;
    bool with_pet_;
};

// Check if filters are passed
bool passed_rider_filters(const rider_side_filters &rsf, driver_side_filters &dsf,
                          point current, point A, point B);
bool passed_driver_filters(const rider_side_filters &rsf, driver_side_filters &dsf,
                           point current, point A, point B);

// Order or potential order asked by a rider
struct order
{
    order() : id_(-1), rider_id_(-1), desirability_(-1), rider_suggested_price_(false),
        best_ETA_min_(-1), best_pickup_route_length_km_(-1), best_driver_id_(-1),
        assigned_driver_id_(-1), ts_accepted_(-1),
        ts_driver_at_A_(-1), ts_ride_started_(-1), price_for_toll_wait_(0),
        sum_metrics_from_B_(0) {}
    
    order(long long order_id,
          long long rider_id,
          point A,
          point B) : id_(order_id), rider_id_(rider_id), A_(A), B_(B), ETA_(0), distance_km_(0),
    dt_added_(0), price_(0), suggested_price_(0), rider_suggested_price_(false), erased_(0), desirability_(false),
    best_ETA_min_(-1), best_pickup_route_length_km_(-1), best_driver_id_(-1),
    assigned_driver_id_(-1), ts_accepted_(-1),
    ts_driver_at_A_(-1), ts_ride_started_(-1), price_for_toll_wait_(0),
    sum_metrics_from_B_(0) {}
    
	// Uniqie order id
	long long id_;

	// Originator id (rider)
	long long rider_id_;

	// Pick up and drop off points
	point A_, B_;

	// Estimated time of a ride in seconds
	int ETA_;

	// Estimated distance of a ride in km
	int distance_km_;
    
    // Datetime when the order/view was added to the marketplace
    time_t dt_added_;

	// Price
	float price_;
    
    // Suggested prices after rebalance
    float suggested_price_;
    
    // If this field is true then rebalance takes this price rather than tariff price as a base
    // Note: this is for situations when a rider has accepted the price change and then rebalance happens
    bool rider_suggested_price_;

	bool erased_;
    
    // How many drivers sees it as a top order
    int desirability_;
    
    // This info is needed for the price view to show the customer how much to wait and
    double best_ETA_min_;
    double best_pickup_route_length_km_;
    int best_driver_id_;
    
    // Filters on a rider side
    rider_side_filters rsf_;
    
    /*
     *  Temporary part - for rebalance optimization
     */
    
    // Sum of all the metrics for all the orders for a driver from this order's/price_view's B
    double sum_metrics_from_B_;
    
    
    /*
     *  Assigned part
     */
    
    // If this order is assigned then here is the driver it is assigned for
    long long assigned_driver_id_;
    
    // When the driver accepted this order
    // Note: this ts is needed for driver stat to understand time spent on order
    time_t ts_accepted_;
    
    // Time that the driver got A
    time_t ts_driver_at_A_;

    // Time that the ride started
    time_t ts_ride_started_;
    
    // The actual price for the toll wait (depends on how many toll minutes
    // the driver has applied, but no more than ts_ride_started_ - ts_driver_at_A_ - free_wait_max_duration_)
    double price_for_toll_wait_;
}; // struct order


// Driver
class driver
{
public:

    driver(long long driver_id, point position, time_t now) : erased_(false), id_(driver_id),position_where_(position), position_when_(now),
        /*metric_type_(driver_metric_profit_per_second), cost_per_km_(14), */top_order_metric_value_(-1000), top_order_desirability_(0), top_order_pointer_(NULL),
        assigned_order_id_(-1)
    {
        imaginary_top_order_id_no_filters_ = 0;
        imaginary_top_metric_no_filters_ = 0;
        imaginary_top_order_id_rsf_ = 0;
        imaginary_top_metric_rsf_ = 0;
        imaginary_top_order_id_dsf_ = 0;
        imaginary_top_metric_no_dsf_ = 0;
    }
    
    
	long long get_id() { return id_; }

	// Returns just an id of the top order
	long long get_top_order_id() { return top_order_.id_; }

	// Returns the top order
	order get_top_order() { return top_order_; }

	// Sets the top order for the driver
	void set_top_order(const order &ord) { top_order_ = ord; }

	// Update driver's position
	void update_position(point position, time_t now)
    {
        position_where_ = position;
        position_when_ = now;
    }

	// Checks if this order is better than the top one based on its parameters and price
	// Note! This functions uses the current most recent position of a driver to determine ETA and distance
	// to the pickup point
	//bool is_better_than_top(const order &ord);

	// The top order for the driver
	// We need to store the copy of the order to determine driver's metric fast!
	// This field is calculated during each rebalance so it does not need to be uploaded
	order top_order_;



	bool erased_;

	// Driver id
	long long id_;

	// Current position of the driver
	point position_where_;
    
    // When the position was updated
    time_t position_when_;

    /*
	// The metric that the driver wants to maximize. She/he might not know this but that's
	// exactly what she/he does :-)
	enum
	{
		driver_metric_revenue_per_second,
		driver_metric_revenue_per_m,
		driver_metric_profit_per_second
	} metric_type_;*/

	// For this metric to optimize driver_metric_profit_per_second
	// Costs:
    // 1. Gas: 4 RUB/km
    // 2. Rent: 1000 RUB/day == 1000 RUB / 480 minute == 2 RUB / min == 4 RUB/km
    // 3. Driver salary elsewhere: 30000 RUB/month == 1500 RUB/day == 1500 RUB/480 minute == 3 RUB/min == 6 RUB/km
    // Total: 14 RUB/km
	//double cost_per_km_;
    
    // Driver's metric for the top order
    // This is NOT just a top order metric calculated with respect to current driver position
    // It's rather the one that has been calculated through two loops on unassigned_orders
    double top_order_metric_value_;
    
    int top_order_desirability_;

    // For temporary use!!!
    order *top_order_pointer_;
    
    // Filters on a driver side
    driver_side_filters dsf_;
    
    /*
     *  For imaginery world without some filters
     */
    long long imaginary_top_order_id_no_filters_;
    double imaginary_top_metric_no_filters_;
    long long imaginary_top_order_id_rsf_;
    double imaginary_top_metric_rsf_;
    long long imaginary_top_order_id_dsf_;
    double imaginary_top_metric_no_dsf_;
    
    // If this driver is assigned then here is the order she/he is assigned to
    long long assigned_order_id_;

}; // class driver

// Notification to a rider
struct rider_notification
{
	// Type of an object the notification is about
	enum object_type {
		rider_notification_view_price_change,
		rider_notification_order_price_change_desire,
        rider_notification_order_price_change_positive,
		rider_notification_driver_position_change
	} object_type_;

	rider_notification(object_type ot, time_t when, long long rider_id, long long id, float old_price, float new_price,
                       double old_metric_value, double new_metric_value, long long new_driver_id,
                       double best_ETA_min = 0, double best_pickup_route_length_km = 0) :
		object_type_(ot), when_(when), rider_id_(rider_id), id_(id)
	{
        old_price_ = old_price;
		new_price_ = new_price;
        old_metric_value_ = old_metric_value;
        new_metric_value_ = new_metric_value;
        new_driver_id_ = new_driver_id;
        best_ETA_min_ = best_ETA_min;
        best_pickup_route_length_km_ = best_pickup_route_length_km;
	}

    time_t when_;
    
	// Who to notify
	long long rider_id_;

	// Id of an object the notification is about
	long long id_;


	/*
	*	Notification data
	*/

	union {
		// If the price is changed then it's here
        struct {
            float old_price_;
            float new_price_;
            double old_metric_value_;
            double new_metric_value_;
            long long new_driver_id_;
            double best_ETA_min_;
            double best_pickup_route_length_km_;
        };

		// If driver position is changed then it's here
		point new_driver_position_;
	};
};

// Notification to a driver
struct driver_notification
{
	driver_notification(driver *driver, time_t now)
    {
        when_ = now;
        
        driver_pointer_for_temp_use_ = driver;
        
        id_ = driver->top_order_.id_;
        driver_id_ = driver->id_;
        top_order_desirability_ = driver->top_order_desirability_;

        object_type_ = driver_notification_top_order;
    }

    time_t when_;

	// Who to notify
	long long driver_id_;
    
    // For temporary use!!!
    driver *driver_pointer_for_temp_use_;

	// Id of an object the notification is about
	long long id_;
    
    int top_order_desirability_;
    double top_order_metric_value_;

	// Type of an object the notification is about
	enum {
		// Notifies the driver about top order change. It can be because there is a new better one or because
		// the current one has been canceled or because as the driver moves they can receive a new better order
		// New id of the top order will be in the id_
		driver_notification_top_order
	} object_type_;
};

// Marketplace
// It's normally one town or a town and suburbs
class marketplace
{
public:

	marketplace(bool is_rebalance_on,
                bool position_freshness_matters,
                bool pv_freshness_matters,
                bool ord_freshness_matters,
                bool notifications_on) : rebalance_time_(0), is_rebalance_on_(is_rebalance_on), position_freshness_matters_(position_freshness_matters),
    pv_freshness_matters_(pv_freshness_matters), ord_freshness_matters_(ord_freshness_matters),
    notifications_on_(notifications_on){}

	/*
	*
	*	Operations to the marketplace
	*
	*	Each operation can result in notifications of riders and drivers
	*	The caller MUST check notification status after each operation and propertly notify riders and/or drivers
	*
	*	Before calling an operation the caller MUST be sure that all the previous notification have been already sent
	*	Otherwise they will be removed and never sent
	*
	*/

	// Removes a driver for the marketplace and order dispatch
    // Note: to add a driver to the marketplace the caller needs just to update her/his position. That's
    // actually the one and only way to add a driver to the marketplace
	bool remove_driver(long long driver_id, time_t now);

	// Adds a new price view by a rider
	// One rider can add many price views
	bool add_price_view(order &&pv, time_t now);

	// Adds a new order for a rider that is not assigned a driver yet
	bool add_unassigned_order(order &ord, time_t now);

    // Converts already existing price view to the order - meaning a rider placed the order
    // Returns true if conversion is succesful
    // Note: we only need rider_id to check if the price view actually belongs to this rider
    bool convert_price_view_to_order(long long rider_id, long long pv_id, time_t now);

    // Accepts the specified order by specified driver
    // Returns true if acceptance is succesful
    // Note: the order MUST be unassigned and the rider MUST be unassigned
    // Note: after acceptance the order stops being visible for dispatch to
    // Note: after acceptance the driver stops being visible for dispatch for
    bool accept_order(long long order_id, long long driver_id, time_t now);
    
    // Finished a ride by a specified driver
    // Note: if a driver does not have an accepted order then nothing happens and false is returned
    bool finish_ride(long long driver_id, time_t now);
    
    bool set_auto_accept_best_order_flag(long long driver_id, bool flag, time_t now);

    // Changes the order price accepted by the rider
    // It's assumed that the caller has already checked that the price is not too much and that
    // the rider have not done it by mistake
    bool change_order_price(long long order_id, float new_price, time_t now);

	// Updates the position of a driver
    // Note: when driver position expire then she/he is removed from the marketplace
    // Note: when driver shows up (by calling this method after being removed) the she/he is returned
    // to the marketplace
	void update_driver_position(long long driver_id, point position, time_t now);
    
    // Update price-view's A and B
    // Why? If an end user moves/changes A and/or B then it's best ETA and price
    // should be updated so it's better to change it in the markeplace rather then
    // to create a new object
    bool update_price_view_A(long long rider_id, long long pv_id, point A, time_t now);
    bool update_price_view_B(long long rider_id, long long pv_id, point B, time_t now);

	// Cancels a price view by a rider
	// For the optimization purpuse cancelation will happen during the next rebalancing
	bool cancel_price_view(long long id, time_t now);

	// Cancels an unassigned order by a rider or the same method could be called if
	// a driver has taken the order so it's not need to be dispatched anymore
	// For the optimization purpuse cancelation will happen during the next rebalancing
    // Returns true if the order actually has benn removed
    // The copy of a removed order is placed to ord - this will be needed later
    // to save it as an assigned order if it has been removed due to being assigned
	bool remove_unassigned_order(long long order_id, time_t now, order *ord = NULL);

    // Driver confirms her/his arrival at A
    // Note: from this point free wait has been started
    bool driver_got_A(long long driver_id, time_t now);
    
    // Driver confirms that a rider in in the car and the ride has been started
    // Note: toll wait timer stops at this point
    bool driver_started_ride(long long driver_id, time_t now);
    
    // Driver accepts the toll for the specified amount of seconds
    // Note: seconds is capped with time(driver_started_ride) - time(driver_got_A) - "usually 3 minutes of free wait"
    // Note: seconds can be zero - means a driver forgives the toll for the wait
    // Returns actual amount of accepted toll seconds
    int driver_accepted_toll_wait(long long driver_id, int seconds, time_t now);

    // Cancels an assigned order by a driver
    // Note: an order can be cancelled by a driver ONLY in the following moments
    //  1.  Before a driver got A
    //  2.  After toll wait is started
    bool driver_cancelled_order(long long driver_id, time_t now);

    // Cancels an unassigned or assigned order by a rider
    // Note: an order can be cancelled by a rider ONLY in the following moments
    //  1.  Unassigned
    //  2.  Assigned before ride starts
    //  In both cases an order turns to a price view
    bool rider_cancelled_order(long long order_id, time_t now);
    
    /*
     *  This is for reducing cubic algorithms in the marketplace to quadratic
     *  Note: the first function MUST be called once in a while if we switch
     *      pricing from manhattan distance to some other distance that changes over time
     *      TODO!!!
     *  Note: the other functions are just called in on an event basis
     */
    void recalc_order_sum_metrics_from_B(); // O(N*N)
    void update_order_sum_metrics_from_B_on_order_appears(order &the_order); // O(N)
    void update_order_sum_metrics_from_B_on_order_will_disappear(order &the_order); // O(N)
    void update_order_sum_metrics_from_B_on_order_change_price(order &the_order, double old_price); // O(N)
    
	/*
	*	Queries to the marketplace
	*/

    // Calculates driver's metric if she/he goes from current, then picks up at A and drops off at B
    long double the_metric(point current, point A, point B, long double price);

    // Here is the potential ride with a bad price
    // And here is the metric of this ride - bad_metric_value
    // bad_metric_value calculated externaly with the respect to the next average ride
    // And here is the good metric of this ride
    // We need the price to keep up with the good metric
    float reprice_as_good_as_this_metric_value(point current, point A, point B,
                                               float bad_price, double bad_metric_value, double good_metric_value);

    
    // Returns driver's position
    // Works for either unassigned drivers or assigned drivers
    geo::point get_driver_position(long long driver_id);

    // Returns order's positions
    // Works for either unassigned drivers or assigned drivers
    bool get_order_points(long long order_id, geo::point *A, geo::point *B);
    
	// Returns notifications about recent changed
	// It's OK to copy data because a) it's once in a while b) it's like 10-30K for even big towns
	void get_notification_lists(std::list<rider_notification> &rn, std::list<driver_notification> &dn)
	{
		rn = rider_notifications_;
		dn = driver_notifications_;
	}
    
    bool is_any_notification()
    {
        return !rider_notifications_.empty() && !driver_notifications_.empty();
    }

	// Returns the best order's id for the driver
	// This function iterate all the drivers and can be costy therefore it should not be called
	// unless a driver does not have information about the top order which is normally not the case
	// because a driver gets notified every time the top order changes
	order get_top_order(long long driver_id);
    
    // If the driver is assigned an order then returns her/his assigned order
    // Otherwise returns an order object with id_ == -1
    order get_assigned_order(long long driver_id);
    
    // Returns the price that the order must have to zero driver's metric
    // Returns the metric value for the driver against the specified order
    bool get_good_price_and_metric(long long driver_id, long long order_id, double &price, double &metric);
    
	// Clears all the notifications
	// The caller MUST be sure that everything was sent before calling this method
	void clear_notifications()
	{
		rider_notifications_.clear();
		driver_notifications_.clear();
	}
    
    // It's useful to turn rebalance on sometimes. For example for an initial data upload: we don't
    // need rebalance until it's all uploaded
    void turn_rebalance_on() { is_rebalance_on_ = true; }
    void turn_rebalance_off() { is_rebalance_on_ = false; }

    // Returns internals of marketplace
    void get_price_views(std::unordered_map<long long, order> &r) { r = price_views_; }
    void get_unassigned_orders(std::unordered_map<long long, order> &r) { r = unassigned_orders_; }
    void get_unassigned_drivers(std::unordered_map<long long, driver> &r) { r = unassigned_drivers_; }
    void get_assigned_orders(std::unordered_map<long long, order> &r) { r = assigned_orders_; }
    void get_assigned_drivers(std::unordered_map<long long, driver> &r) { r = assigned_drivers_; }

    // Rebalances the marketplace now
    void force_rebalance(time_t now);
    
    // Clears everything expired and deleted
    // Note: this function is called automatically in course of automatic rebalance
    // Note: force_rebalance DOES NOT call this function
    void clear_expired_and_deleted_objects(time_t now);

    /*
     *  Driver stat & billing
     */

    bool get_driver_stat_entries(long long driver_id, time_t from, time_t to, std::vector<driver_stat_entry_ride> &result);
    bool get_driver_stat_aggr_entries(long long driver_id, time_t from, time_t to, std::vector<driver_stat_entry_ride> &result);
    bool get_driver_stat_entry_shift_purchase(long long driver_id, time_t from, time_t to, std::vector<driver_stat_entry_shift_purchase> &result);
    bool get_driver_stat_payments(long long driver_id,time_t from, time_t to, std::vector<driver_stat_payment> &result);
    bool get_driver_stat_withdrawals(long long driver_id,time_t from, time_t to, std::vector<driver_stat_withdrawal> &result);
    bool get_driver_stat_cashbacks(long long driver_id,time_t from, time_t to, std::vector<driver_stat_cashback> &result);
    bool get_driver_stat_bonuses(long long driver_id,time_t from, time_t to, std::vector<driver_stat_bonus> &result);
    bool get_driver_stat_promos(long long driver_id,time_t from, time_t to, std::vector<driver_stat_promo> &result);
    driver_stat_balance restore_driver_balance(long long driver_id, time_t ts);
    
private:
    
    marketplace(const marketplace&);
    marketplace& operator=(const marketplace&);

    // Rebalances the marketplace. It's only done its costy work once in params::rebalance_timeout() seconds
    // All erased and expired objects are cleared inside
    void rebalance(time_t now);
    
    // If the driver is assigned an order then returns the pointer to her/his assigned order
    // Otherwise returns NULL
    // Note: the pointer to the order that can be changed but quickly - until a new
    //  assigned order appears or erases (otherwise the internal container will invalid
    //  this pointer)
    order *get_assigned_order_impl_(long long driver_id);
    
    driver_stat *get_driver_stat_impl_(long long driver_id);
    
	// Active price views
    std::unordered_map<long long, order> price_views_;

	// Orders ready to be assigned. The key is order_id
	std::unordered_map<long long, order> unassigned_orders_;

	// Drivers ready to take orders. The key is driver_id
	std::unordered_map<long long, driver> unassigned_drivers_;

    // Assigned orders. The key is order_id
    std::unordered_map<long long, order> assigned_orders_;

    // Assigned drivers
    std::unordered_map<long long, driver> assigned_drivers_;
    
    // Statistics & billing for every driver
    std::unordered_map<long long, driver_stat> driver_stat_;
    
	// Notifications
	std::list<rider_notification> rider_notifications_;
	std::list<driver_notification> driver_notifications_;

	time_t rebalance_time_;
    bool is_rebalance_on_;
    
    // Do we need to clean up expired things?
    bool position_freshness_matters_;
    bool pv_freshness_matters_;
    bool ord_freshness_matters_;
    
    // Do we need to use notifications (we don't need them if we rather send all the marketplace over
    // every now and then)
    bool notifications_on_;
};

}

#endif


