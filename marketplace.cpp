#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>

#include "marketplace.h"

/*
*	marketplace.cpp
*
*	(C) Denis Anikin 2020
*
*	Implementation for marketplace
*
*/

namespace marketplace
{

// Returns ETA in minutes and the route length in km between two points
void ETA_route_minutes_km(point A, point B, long double &ETA_min, long double &route_length_km)
{
    // Calculate distance between points in km
    // TODO: in a future there will be a better way
    route_length_km = manhattan_distance_km(A, B);
    
    // Calculate time in minutes between points - as the speed in 30km/hr (params::average_speed_in_town())
    // TODO: in a future there will be a better way
    ETA_min = (route_length_km / params::average_speed_in_town()) * 60;
}

// Return price for a ride in RUB by time and route length
float tarif_price(long double ETA_min, long double route_length_km)
{
    float price = route_length_km * 8 + ETA_min * 7;
    if (price < 49)
        return 49;
    else
        return price;
}

// Returns price for a ride in RUB by two points
float tarif_price(point A, point B)
{
    long double ETA_min;
    long double route_length_km;
    ETA_route_minutes_km(A, B, ETA_min, route_length_km);

    return tarif_price(ETA_min, route_length_km);
}

// Check rider filters against a driver
bool passed_rider_filters(const rider_side_filters &rsf, driver_side_filters &dsf,
                                point current, point A, point B)
{
    if (!rsf.pass_everything_)
    {
        // Order options
        if (rsf.no_smoking_ && dsf.smoking_allowed_)
        {
            dsf.fr_.by_smoking_ = true;
            return false;
        }
        if (rsf.need_childseat_ && !dsf.chidseat_)
        {
            dsf.fr_.by_childseat_ = true;
            return false;
        }
    }
    
    return true;
}
inline bool passed_rider_filters(const order &o, driver &d)
{
    return passed_rider_filters(o.rsf_, d.dsf_, d.position_where_, o.A_, o.B_);
}


// Check driver filters against a rider/order
bool passed_driver_filters(const rider_side_filters &rsf, driver_side_filters &dsf,
                                point current, point A, point B)
{
    if (!dsf.pass_everything_)
    {
        // Distance
        if (dsf.max_distance_to_A_km_ != 0 && manhattan_distance_km(current, A) > dsf.max_distance_to_A_km_)
        {
            dsf.fr_.by_max_distance_to_A_ = true;
            return false;
        }
        if (dsf.max_distance_to_B_km_ != 0 && manhattan_distance_km(A, B) > dsf.max_distance_to_B_km_)
        {
            dsf.fr_.by_max_distance_to_B_ = true;
            return false;
        }
    
        // Payment
        if ((rsf.only_cash_ && dsf.no_cash_) || (rsf.only_sbrebank_online_ && dsf.no_sbrebank_online_))
        {
            dsf.fr_.by_payment_method_ = true;
            return false;
        }
    
        // Bad districts
        for (auto &bd : dsf.bad_districts_A_)
        {
            if (manhattan_distance_km(bd, A) <= dsf.bad_district_radius_km_)
            {
                dsf.fr_.by_bad_district_A_ = true;
                return false;
            }
        }
        for (auto &bd : dsf.bad_districts_B_)
        {
            if (manhattan_distance_km(bd, B) <= dsf.bad_district_radius_km_)
            {
                dsf.fr_.by_bad_district_B_ = true;
                return false;
            }
        }
        
        // Home orders
        // TODO: check if the order is ON THE WAY home but not just AROUND home area
        if (dsf.B_only_around_home_ && manhattan_distance_km(B, dsf.home_) > dsf.home_area_radius_km_)
        {
            dsf.fr_.by_home_area_ = true;
            return false;
        }
    } // if (!dsf.no_filters_)
    
    return true;
}
inline bool passed_driver_filters(const order &o, driver &d)
{
    return passed_driver_filters(o.rsf_, d.dsf_, d.position_where_, o.A_, o.B_);
}

void marketplace::rebalance(time_t now)
{
    if (!is_rebalance_on_)
        return;
    
    // Note: Don't clear notifications. Becuse the caller might want to send it less regularily than relalance() is called

	// Check if there is time to rebalance
	if (rebalance_time_ + params::rebalance_timeout() > now)
		return;
    rebalance_time_ = now;

    // Clear everything deleted and expired
    clear_expired_and_deleted_objects(now);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long sec1 = tv.tv_sec * 1000000 + tv.tv_usec;
    
    // Force rebalance right now
    force_rebalance(now);

    gettimeofday(&tv, NULL);
    long long sec2 = tv.tv_sec * 1000000 + tv.tv_usec;
    
    std::cerr << "marketplace::rebalance: force_rebalance has taken " << (sec2 - sec1)/1000 << " miliseconds" << std::endl;
}

void marketplace::clear_expired_and_deleted_objects(time_t now)
{
    //std::cout << "debug: marketplace::clear_expired_and_deleted_objects(" << now << ")" << std::endl;
    
    // Erase all the deleted objects from lists
    for (auto pv = price_views_.begin(); pv != price_views_.end(); )
    {
        // Additionally erase all old price views
        // Note: pv_freshness_matters_ can be false for dev/test reasons
        if (pv->second.erased_ ||
            (pv_freshness_matters_ && pv->second.dt_added_ + params::price_view_expiration_time() < now))
        {
            auto save = pv;
            ++save;
            //printf("marketplace::clear_expired_and_deleted_objects debug before erase pv %d\n", (int)pv->id_); fflush(stdout);
            price_views_.erase(pv);
            pv = save;
        }
        else
            ++pv;
    }
    for (auto d = unassigned_drivers_.begin(); d != unassigned_drivers_.end(); )
    {
        // Additionally erase all drivers with old coordinates
        // Note: position_freshness_matters_ can be false for dev/test reasons
        if (d->second.erased_ ||
            (position_freshness_matters_ && d->second.position_when_ + params::position_freshness_time() < now))
        {
            auto save = d;
            ++save;
            //printf("marketplace::clear_expired_and_deleted_objects debug before erase driver %d\n", (int)d->id_); fflush(stdout);
            unassigned_drivers_.erase(d);
            d = save;
        }
        else
            ++d;

    }
    for (auto ord = unassigned_orders_.begin(); ord != unassigned_orders_.end(); )
    {
        // Additionally erase all old unassigned orders
        // Note: ord_freshness_matters_ can be false for dev/test reasons
        if (ord->second.erased_ ||
            (ord_freshness_matters_ && ord->second.dt_added_ + params::unassigned_order_expiration_time() < now)   )
        {
            auto save = ord;
            ++save;

            // This is for avoiding cubic algorithms in rebalace
            update_order_sum_metrics_from_B_on_order_will_disappear(ord->second); // O(N)
            
            //printf("marketplace::clear_expired_and_deleted_objects debug before erase order %d\n", (int)ord->id_); fflush(stdout);
            unassigned_orders_.erase(ord);
            ord = save;
        }
        else
            ++ord;
    }
}

void marketplace::recalc_order_sum_metrics_from_B()
{
    // Calculate the average metric for all orders after the_order is done
    // Note: it is needed to consider the fact that if a driver goes to a low
    //  demand place then it will decrease her/his metric
    // Note: it's done once for all drivers and does not take driver/rider filters
    //  into account. Why? Because this is just for rough approzimate of the demand
    //  in each order's B to take from there
    for (auto &ord_i : unassigned_orders_)
    {
        auto &ord = ord_i.second;
        ord.sum_metrics_from_B_ = 0;
        for (auto &another_order_i : unassigned_orders_)
        {
            //++iterations0;
            auto &another_order = another_order_i.second;
            // Don't forget not to count an order twice
            if (another_order.id_ != ord.id_)
            {
                // Drop off a rider at the drop off point of the_order and take the next
                // order from there
                ord.sum_metrics_from_B_ += the_metric(ord.B_, another_order.A_, another_order.B_, another_order.price_);
            }
        }
    }
}

void marketplace::update_order_sum_metrics_from_B_on_order_appears(order &the_order)
{
    // It's the same thing as recalc_order_sum_metrics_from_B but
    // in O(N) instead of O(N*N) and it's done on demand when
    //  1.  A new unssinged order appears

    // Calc the metric for a new order as usual
    the_order.sum_metrics_from_B_ = 0;
    for (auto &another_order_i : unassigned_orders_)
    {
        auto &another_order = another_order_i.second;
        if (another_order.id_ != the_order.id_)
            the_order.sum_metrics_from_B_ += the_metric(the_order.B_, another_order.A_, another_order.B_, another_order.price_);
    }

    // Add a metric for every existing order as the new one appears
    for (auto &another_order_i : unassigned_orders_)
    {
        auto &another_order = another_order_i.second;
        if (another_order.id_ != the_order.id_)
            another_order.sum_metrics_from_B_ += the_metric(another_order.B_, the_order.A_, the_order.B_, the_order.price_);
    }
}

void marketplace::update_order_sum_metrics_from_B_on_order_will_disappear(order &the_order)
{
    // It's the same thing as recalc_order_sum_metrics_from_B but
    // in O(N) instead of O(N*N) and it's done on demand when
    //  2.  An unassigned order disapear
    
    // Substract a metric for every existing order as the order disapears
    for (auto &another_order_i : unassigned_orders_)
    {
        auto &another_order = another_order_i.second;
        if (another_order.id_ != the_order.id_)
            another_order.sum_metrics_from_B_ -= the_metric(another_order.B_, the_order.A_, the_order.B_, the_order.price_);
    }
}

void marketplace::update_order_sum_metrics_from_B_on_order_change_price(order &the_order, double old_price)
{
    // It's the same thing as recalc_order_sum_metrics_from_B but
    // in O(N) instead of O(N*N) and it's done on demand when
    //  3.  A price for an order is changed

    // Change a metric for every existing order as the price of an order changes
    for (auto &another_order_i : unassigned_orders_)
    {
        auto &another_order = another_order_i.second;
        if (another_order.id_ != the_order.id_)
        {
            another_order.sum_metrics_from_B_ -= the_metric(another_order.B_, the_order.A_, the_order.B_,
                                                            old_price);
            another_order.sum_metrics_from_B_ += the_metric(another_order.B_, the_order.A_, the_order.B_,
                                                            the_order.price_);
        }
    }
}
void marketplace::force_rebalance(time_t now)
{
    std::cerr << "marketplace::force_rebalance: price_views_.size()=" << price_views_.size() << ", unassigned_orders_.size()=" << unassigned_orders_.size() << ", unassigned_drivers_.size()=" << unassigned_drivers_.size() << ", assigned_orders_.size()=" << assigned_orders_.size() << ", assigned_drivers_.size()=" << assigned_drivers_.size() << std::endl;
    
    int iterations0 = 0, iterations0pv = 0, iterations1 = 0, iterations2 = 0, iterations3 = 0, iterations4 = 0;
    
    // Mark each order not desirable + prepare ETA and distance
    for (auto &ord_i : unassigned_orders_)
    {
        auto &ord = ord_i.second;
        ord.desirability_ = 0;
        
        // No best driver for the order (along with best ETA and km to A)
        // This will be determine during the Algo #1
        ord.best_driver_id_ = -1;
        ord.best_ETA_min_ = -1;
        ord.best_pickup_route_length_km_ = -1;

        //float distance_km = (float)manhattan_distance_km(ord.A_, ord.B_);
        
        //float price = tarif_price(ord.A_, ord.B_);
        
        //printf ("Order %d distance %.2f, price %.2f\n", (int)ord->id_, distance_m / 1000, price);
        
        //ord.distance_km_ = distance_km;
        
        /*
         
        We actually don't need this, because an order inherits the price from its price view
         
        // Note: is a rider suggested (or accepted) increased price of a placed order then
        // rebalance sticks with that price and never uses the tarif price
        // Otherwise the use tarif price as a baseline
        //
        // Note: drivers can have their own tariffs. The baseline is some average tariff to make prices real from the start
        if (!ord.rider_suggested_price_)
            ord.price_ = price;*/
        
        /*
        
        We do this only when:
         
        1.  A new unassingned order appears
        2.  Removes
        3.  Changes its price
         
        // Calculate the average metric for all orders after the_order is done
        // Note: it is needed to consider the fact that if a driver goes to a low
        //  demand place then it will decrease her/his metric
        // Note: it's done once for all drivers and does not take driver/rider filters
        //  into account. Why? Because this is just for rough approzimate of the demand
        //  in each order's B to take from there
        ord.sum_metrics_from_B_ = 0;
        for (auto &another_order_i : unassigned_orders_)
        {
            ++iterations0;
            auto &another_order = another_order_i.second;
            // Don't forget not to count orders twice
            if (another_order.id_ != ord.id_)
            {
                // Drop off a rider at the drop off point of the_order and take the next
                // order from there
                ord.sum_metrics_from_B_ += the_metric(ord.B_, another_order.A_, another_order.B_, another_order.price_);
            }
        }*/
    }

    // Fill sum_metrics_from_B_ the same way as we did above for unassigned orderd
    // O(N*N)
    for (auto &pv : price_views_)
    {
        pv.second.sum_metrics_from_B_ = 0;
        for (auto &ord_i : unassigned_orders_)
        {
            ++iterations0pv;
            auto &ord = ord_i.second;
            pv.second.sum_metrics_from_B_ += the_metric(pv.second.B_, ord.A_, ord.B_, ord.price_);
        }
    }
    
    // TODO: smart limit loops based on H3
    
    /*
     *
     *          Steps of marketplace rebalancing
     *
     *      1.  Figure out top orders in terms of metric for each driver for the order to complete and for the
     *          next average order to complete. Action: notify drivers about possibly new top orders
     *      2.  Figure out all non desirable orders. Action: ask riders to increase the price to make each
     *          non desirable order desirabe for at least one driver
     *      3.  Figure out desirability of price views as if they were orders
     *      3.1.    For price views non desirable to any driver do action: increase price to be desirable for at least
     *              one driver and notify the rider (otherwise this order will be be seen and therefore accepted)
     *              and notify the rider
     *      3.2.    For price views desirable for at least one driver do action: decrease price to be still
     *              desirable to the same
     *              amount of drivers (it will not change visibility of the price view if it becomes an order by will
     *              stimulate its moving forward to the placed order)
     *              and notify the rider
     *      TODO: do something with nevative metric value
     *      TODO: do something with offline drivers
     *
     */
    
    // Crazy cubic algorithm :-)
    // TODO: limit it to quadratic if the number or iterations is too big
    // 1. Iterate all drivers
    // 2. For each driver search for the best order to optimize the metric with respect to the next orders
    // 3. The best means: (metric(the_order) + average(metric(other_order1), ..., metric(other_orderN)))/2 -> max
    // Why? Because a driver can take the best order but then will have to take very bad orders because they will
    // end up in suburbs or in the middle of nowhere :-)
    // Why average but not just the maximum among all the pair of orders? Well it's because orders will be taken and the
    // average will mitigate this deviation
    //
    //  Target: let every driver know about his or her top order. It works for online and offline drivers - everybody has
    //  to have top order on time because everybody even being offline would want to open the app
    //
    //  How it will benefit drivers: they will benefit from a better order in terms of profit
    //  How it will benefit riders: their orders will be more likely to be accepted
    //  Downside: not known
    //
    for (auto &drv_hash_entry : unassigned_drivers_)
    {
        driver &drv = drv_hash_entry.second;
        
        // Best metric & order if NEITHER RIDER OR DRIVER FILTERS TURNED ON
        double best_metric = -1000000000;
        order *best_order = NULL;

        // Best metric & order if BOTH RIDER & DRIVER FILTERS TURNED ON
        double best_metric_rsf_dsf = -1000000000;
        order *best_order_rsf_dsf = NULL;

        // Best metric & order if ONLY RIDER FILTERS TURNED ON
        double best_metric_rsf = -1000000000;
        order *best_order_rsf = NULL;

        // Best metric & order if ONLY DRIVER FILTERS TURNED ON
        double best_metric_dsf = -1000000000;
        order *best_order_dsf = NULL;

        // In case if there are no orders at all :-)
        drv.top_order_.id_ = -1;
        drv.top_order_.rider_id_ = -1;
        drv.top_order_desirability_ = 0;
        drv.top_order_pointer_ = NULL;
        drv.top_order_metric_value_ = -1000;

        // Same thing for imaginery world - clear all the best just in case for no orders
        // even without some filters
        drv.imaginary_top_order_id_no_filters_ = 0;
        drv.imaginary_top_metric_no_filters_ = 0;
        drv.imaginary_top_order_id_rsf_ = 0;
        drv.imaginary_top_metric_rsf_ = 0;
        drv.imaginary_top_order_id_dsf_ = 0;
        drv.imaginary_top_metric_no_dsf_ = 0;
        
        //printf("rebalace1\n");
        
        // Searching for "the order"
        for (auto &the_order_i : unassigned_orders_)
        {
            ++iterations1;
            
            auto &the_order = the_order_i.second;
            bool b_passed_rider_filters = true;
            bool b_passed_driver_filters = true;

            // Do we need to filter the order out?
            if (!passed_rider_filters(the_order, drv))
            {
                std::cerr << "rebalance debug algo #1: rider filtered the order out, order " << the_order.id_
                    << ", driver " << drv.id_ << ", reason " << drv.dsf_.fr_ << std::endl;
                b_passed_rider_filters = false;
            }
            if (!passed_driver_filters(the_order, drv))
            {
                std::cerr << "rebalance debug algo #1: driver filtered the order out, order " << the_order.id_
                    << ", driver " << drv.id_ << ", reason " << drv.dsf_.fr_ << std::endl;
                b_passed_driver_filters = false;
            }

            // How profitable is to take the_order from the current position?
            double the_order_metric = the_metric(drv.position_where_, the_order.A_, the_order.B_, the_order.price_);
            double avg = the_order.sum_metrics_from_B_;
            //printf("rebalace2\n");
                  
            //printf("the_order_metric=%.2f\n", the_order_metric); fflush(stdout);
            
            // Calculate the average metric for all orders after the_order is done
            /*int n = 0;
            for (auto &another_order_i : unassigned_orders_)
            {
                ++iterations1;
                auto &another_order = another_order_i.second;
                // Don't forget not to count orders twice
                if (another_order.id_ != the_order.id_)
                {
                    // Drop off a rider at the drop off point of the_order and take the next
                    // order from there
                    avg += drv.the_metric(the_order.B_, another_order.A_, another_order.B_, another_order.price_);
                    ++n;
                }
            }*/
            
            // For the case if there is a single unassigned order in the marketplace :-)
            // This hack will remain the order metric the same like there were not the inner order loop
            if (unassigned_orders_.size() > 1)
                avg /= unassigned_orders_.size() - 1;
            else
                avg = the_order_metric;

            //printf("avg metric=%.2f, the_order_metric=%.2f\n", avg, the_order_metric);fflush(stdout);
            
            if ((the_order_metric + avg)/2 > best_metric)
            {
                /*
                 *      We want to have top orders for all 4 cases of rider & driver filters:
                 *     both applied, rider applied, driver applied, nothing applied. Why?
                 *
                 *      For the following reason:
                 *
                 *      1.  If there is no difference for a rider if their filters applied or
                 *          did not in terms of top order then do nothing
                 *      2.  If there is no difference for a driver if their filters applied or
                 *          did not in terms of top order then do nothing
                 *      3.  If there IS A DIFFERENCE for a rider then we advise a rider to
                 *          TURN FILTERS OFF to have a better price and/or pickup time
                 *      4.  If there IS A DIFFERENCE for a driver then we advice a driver to
                 *          TURN FILTERS OFF to have a better order in terms metric
                 */
                
                double bm = (the_order_metric + avg)/2;
                order *bo = &the_order;
                
                // Now, what if both filters turned off?
                // It's just
                best_metric = bm;
                best_order = bo;
                
                // Ok, then what if only a rider turns on filters? It's
                // like driver filters don't exist. So count only rider filters
                if (b_passed_rider_filters)
                {
                    best_metric_rsf = bm;
                    best_order_rsf = bo;
                }

                // Ok, then what if only a driver turns on filters? It's
                // like rider filters don't exist. Count only driver filters
                if (b_passed_driver_filters)
                {
                    best_metric_dsf = bm;
                    best_order_dsf = bo;
                }
                
                // What if both rider and driver remain filters? Count both filters
                if (b_passed_rider_filters && b_passed_driver_filters)
                {
                    best_metric_rsf_dsf = bm;
                    best_order_rsf_dsf = bo;
                }
            } // if ((the_order_metric + avg)/2 > best_metric)

        }

        // Now best_order is the best order for the driver
        // Note: count all the filters because IT'S THE REAL BEST ORDER IN THE REAL WORLD WITH FILTERS :-)
        if (best_order_rsf_dsf)
        {
            drv.set_top_order(*best_order_rsf_dsf);
            drv.top_order_pointer_ = best_order_rsf_dsf;
            drv.top_order_metric_value_ = best_metric_rsf_dsf;
            
            // Now this order is a bit more desirable
            ++(best_order_rsf_dsf->desirability_);

            // Update order's best driver, ETA and km to A
            // If we never reach this code then the order will be without a driver at all
            // That maybe because of absence of drivers or presence of filters
            long double best_ETA_min = -1, best_pickup_route_length_km = -1;
            ETA_route_minutes_km(drv.position_where_,
                                 best_order_rsf_dsf->A_, best_ETA_min, best_pickup_route_length_km);

            if (best_order_rsf_dsf->best_driver_id_ == -1 ||
                best_ETA_min < best_order_rsf_dsf->best_ETA_min_)
            {
                best_order_rsf_dsf->best_driver_id_ = drv.id_;
                best_order_rsf_dsf->best_ETA_min_ = best_ETA_min;
                best_order_rsf_dsf->best_pickup_route_length_km_ = best_pickup_route_length_km;
            }

            // Notify about a new top order
            // TODO: is the new order has not changed then we don't need to notify
            if (notifications_on_)
                driver_notifications_.push_back(driver_notification(&drv, now));
        }
        
        // Now save for the driver order ids & metrics for IMAGINERY WORLD WITHOUT SOME FILTERS
        drv.imaginary_top_order_id_no_filters_ = best_order ? best_order->id_ : 0;
        drv.imaginary_top_metric_no_filters_ = best_metric;
        drv.imaginary_top_order_id_rsf_ = best_order_rsf ? best_order_rsf->id_ : 0;
        drv.imaginary_top_metric_rsf_ = best_metric_rsf;
        drv.imaginary_top_order_id_dsf_ = best_order_dsf ? best_order_dsf->id_ : 0;
        drv.imaginary_top_metric_no_dsf_ = best_metric_dsf;
        
        /*
         *      TODO: Now how can we use tis data?
         */
        
        // TODO: notiy rider about new best driver for an order

    } // for (auto &drv_hash_entry : unassigned_drivers_)
  
    // Note! Use drv.top_order_pointer_ only between the loop that above and the next one! Because it could be invalidated
    // It's temporary
    for (auto &drv : unassigned_drivers_)
    {
        if (drv.second.top_order_pointer_)
            drv.second.top_order_desirability_ = drv.second.top_order_pointer_->desirability_;
    }
    for (auto &dn : driver_notifications_)
    {
        dn.top_order_desirability_ = dn.driver_pointer_for_temp_use_->top_order_desirability_;
        dn.top_order_metric_value_ = dn.driver_pointer_for_temp_use_->top_order_metric_value_;
    }
    
    //printf("rebalance debug: before 2 step\n");fflush(stdout);

    // Second crazy algorithm
    //
    // We need to find non desirable orders and ask a rider to increase its price to have every non desirable
    // order at least seen by one driver
    //
    // Note: we don't consider positiveness of orders. Say, if an order makes life of a driver better being on its
    // top then it is already better than before. Moreover, making it positive can make customer not wanting to
    // pay because, for example, of oversupply situation
    //
    // 1. Iterate all orders and find not desirale ones
    // 2. Reprice them to be desirable for at least one driver
    // 3. Form a notification for repricing request
    //
    //  Target: let riders know how much actually they have to pay extra to have their order seen at least with one
    //  driver. TODO: take into account only online drivers. For example: if it's nighttime and only one driver left then
    //  you want to place the order to that driver, but not others who are sleeping
    //
    //  How it will benefit drivers: they may receive better orders in terms of profit (if riders agree to increase price)
    //  How it will benefit riders: their orders will be more likely to be accepted
    //  Downside: rider can cancel the order after asked to pay more
    //
	for (auto &not_desirable_order_i : unassigned_orders_)
	{
        auto &not_desirable_order = not_desirable_order_i.second;
        // Found a bad order
        if (!not_desirable_order.desirability_)
        {
            float min_price = 1000000000;
            float min_price_increase = 1000000000;
            double old_metric_value = -100, new_metric_value = -100;
            long long new_driver_id = 0;
            driver *new_driver = NULL;
   
            /*
             
            DEPRECATED. This logic increases price and slighly changes top metric which is not what we need
             
             
            double least_metric_diff = 1000000000;
            */
            
            // How much this non-desirable order MUST cost to attract this driver?
            // It MUST cost the exact amount for the driver to have the metric and her/his top order
            // And the main thing here is that the top order metric MUST be taken from drv because
            // that's not just the metric from the current position, that's rather the metric calculated
            // having in mind taking the next average order
            for (auto &drv_hash_entry : unassigned_drivers_)
            {
                ++iterations2;
                driver &drv = drv_hash_entry.second;
                bool b_passed_rider_filters = true;
                bool b_passed_driver_filters = true;

                // Do we need to filter the driver out?
                if (!passed_rider_filters(not_desirable_order, drv))
                {
                    std::cerr << "rebalance debug algo #2: rider filtered the order out, order " << not_desirable_order.id_
                        << ", driver " << drv.id_ << ", reason " << drv.dsf_.fr_ << std::endl;
                    b_passed_rider_filters = false;
                }
                if (!passed_driver_filters(not_desirable_order, drv))
                {
                    std::cerr << "rebalance debug algo #2: driver filtered the order out, order " << not_desirable_order.id_
                        << ", driver " << drv.id_ << ", reason " << drv.dsf_.fr_ << std::endl;
                    b_passed_driver_filters = false;
                }
                
                // TODO: in a future we need to find the best price for an order if filters turn off
                // In that case we can suggest a rider with two prices. Something like this:
                // Order price: 250RUB, to make it visible it'd be 270RUB, but to make it visible
                // without filters it could be 260RUB (or even as low as 250 if desirability increased to
                // at least 1)
                if (!b_passed_rider_filters || !b_passed_driver_filters)
                    continue;
                

                // Calculate the metric of non desirable order with respect to this driver
                // It's absolutely the same idea as for the pevious cubic algorithm :-)
                // Start with the metric for the first order for this driver
                double not_desirable_order_metric = the_metric(drv.position_where_, not_desirable_order.A_, not_desirable_order.B_, not_desirable_order.price_);

                // Then calculate average metric for next orders (excluding the current non desirable one)
                // TODO: optimization: we can divide metric calc into two steps: a)not_desirable_order.B_ -> another_order - inside this 3rd loop and b) another_order.A_ -> another_order.B_ in the outer 2nd loop which will hopefuly reduce the time taken by the 3rd loop by half
                
                double avg = not_desirable_order.sum_metrics_from_B_;
                
                /*int n = 0;
                for (const auto &another_order_i : unassigned_orders_)
                {
                    ++iterations2;
                    const auto &another_order = another_order_i.second;
                     if (another_order.id_ != not_desirable_order.id_)
                     {
                         avg += drv.the_metric(not_desirable_order.B_, another_order.A_, another_order.B_, another_order.price_);
                         ++n;
                     }
                }*/
                
                // Hack. See 1 algorithm's comment
                if (unassigned_orders_.size() > 1)
                    avg /= unassigned_orders_.size() - 1;
                else
                    avg = not_desirable_order_metric;
                
                // And finally take the metric as average for the non desirable order and average for next orders
                not_desirable_order_metric = (not_desirable_order_metric + avg)/2;
        
                if (not_desirable_order_metric > drv.top_order_metric_value_)
                {
                    // This MUST never happen
                    printf("ASSERT!!! not_desirable_order_metric=%.2f, drv.top_order_metric_value_=%.2f\n", not_desirable_order_metric, drv.top_order_metric_value_);
                    exit(0);
                }
/*
                DEPRECATED. This logic increases price and slighly changes top metric which is not what we need
 

                
                if ((drv.top_order_metric_value_ - not_desirable_order_metric) <= least_metric_diff)
                {
                    least_metric_diff = drv.top_order_metric_value_ - not_desirable_order_metric;
                    old_metric_value = not_desirable_order_metric;
                    new_metric_value = drv.top_order_metric_value_;
                    new_driver_id = drv.id_;
                    new_driver = &drv;
                }*/

                // Get the price for the order to beat the top order
                float good_price = reprice_as_good_as_this_metric_value(drv.position_where_,
                                    drv.top_order_.A_, drv.top_order_.B_, not_desirable_order.price_, not_desirable_order_metric, drv.top_order_metric_value_);
                
                float price_increase = good_price - not_desirable_order.price_;
                
                if (good_price < min_price)
                {
                    min_price = good_price;
                    old_metric_value = not_desirable_order_metric;
                    new_metric_value = drv.top_order_metric_value_;
                    new_driver_id = drv.id_;
                }
                
                if (price_increase < min_price_increase)
                {
                    min_price_increase = price_increase;
                    old_metric_value = not_desirable_order_metric;
                    new_metric_value = drv.top_order_metric_value_;
                    new_driver_id = drv.id_;
                }
            }

            /*

            DEPRECATED. This logic increases price and slighly changes top metric which is not what we need
 
 
            // Get the price for the order to beat the top order in terms of its metric
            float good_price = new_driver->reprice_as_good_as_this_metric_value(new_driver->position_where_,
                                        new_driver->top_order_.A_, new_driver->top_order_.B_, not_desirable_order.price_,
                                            old_metric_value, new_metric_value);
      */
			// Suggest the rider a bigger price
            if (notifications_on_)
                rider_notifications_.push_back(rider_notification(
                                                                  rider_notification::rider_notification_order_price_change_desire,
                                                                  now,
                                                                  not_desirable_order.rider_id_, not_desirable_order.id_, not_desirable_order.price_,
                                                              //not_desirable_order.price_ + min_price_increase,
                                                              min_price,
                                                              old_metric_value, new_metric_value, new_driver_id
				));

            // Save suggested price in the order for dev/test reasons - that's when we dump all the marketplace
            // and want to see what's under the hood without messing with notifications :-)
            not_desirable_order.suggested_price_ = min_price;
            
		} // if (!not_desirable_order.desirability_)
        
	} // for (auto &not_desirable_order : unassigned_orders_)

    //printf("rebalance debug: before 3 step\n");fflush(stdout);

    // Third crazy algorithm
    // We need to find bad price views and NOTIFY a rider about the price increase to have every bad price view at least with one driver
    // And we need to find good price views desirable for at least one driver and decrease the price for them to the
    // least amount that will allow us to keep the same desirability for the price view becoming a placed order
    //
    // Another important thing here is that don't we only want to be every price view a desirable one but also to be
    // metric-positive. So the price of the price view MUST be as much as its metric with the driver is max(0, top_order_metric_value)
    //
    // 1. Iterate all price views
    // 2. If a price view not desirable by anybody then reprice it up to be desirable for at least one driver and reprice
    //      it even more up to have at least 0 metric with the driver
    // 3. If a price view is desirable by somebody and positive then reprice it down to be desirable for the same amount of
    //      drivers keeping the metric positive
    // 3. Form a notification for repricing
    //
    //  Target: let know riders about a price change to have their possible order seen and be positive to at
    //  least one driver. TODO: take into account only online drivers. For example: if it's nighttime and only one driver left then
    //  you don't want to place the order to that driver, but not others who are sleeping. The same thing for a driver who is
    //  on the way with a rider
    //
    //  How it will benefit drivers: they may receive better orders in terms of profit (if riders agree to order after a price change)
    //  How it will benefit riders: their orders will be more likely to be accepted
    //  Downside: rider can cancel the order after being asked about price change
    //
    for (auto &pv_i : price_views_)
    {
        auto &pv = pv_i.second;
            // We need to figure out three things:
            //
            // 1. The minimum aligned price among only those drivers who will benefit from this price view becoming a placed order
            // 2. The minimum aligned price among only those drivers who will not benefit from this price view becoming an order
            // 3. Does anybody benefit from price view becoming an order? (aka is its desirability > 0?)
            //
            // The first thing will allow us to decrease the pv price down to the mininum value being still be suitable for
            // the same amount of drivers. The price decrease stimulate the order placement but will not make driver lifes worse
            //
            // If the third thing is NOT TRUE then the second thing will allow us to increase the pv price towards the minimum
            // to be suitable for at least on driver. It should be no big deal for a rider because it looks like that
            // it is shortage of free drivers so somebody has to pay more to have a ride :-)
        
            float min_aligned_price_among_drivers_ready_to_take_it = 1000000000;
            float min_aligned_price_among_drivers_not_ready_to_take_it = 1000000000;
            bool does_anybody_take_it = false;
        
            // The best driver among those benetif to take the order that'be placed from this price view
            driver *best_driver_benefit = NULL;
            // The best driver among those don't benetif to take the order that'be placed from this price view
            driver *best_driver_no_benefit = NULL;
        
            // For the debug purpose
            double old_price = pv.price_;

            for (auto &drv_hash_entry : unassigned_drivers_)
            {
                ++iterations3;
                driver &drv = drv_hash_entry.second;
                bool b_passed_rider_filters = true;
                bool b_passed_driver_filters = true;

                // Do we need to filter the driver out?
                if (!passed_rider_filters(pv, drv))
                {
                    std::cerr << "rebalance debug algo #3: rider filtered the order out, order " << pv.id_
                         << ", driver " << drv.id_ << ", reason " << drv.dsf_.fr_ << std::endl;
                    b_passed_rider_filters = false;
                }
                if (!passed_driver_filters(pv, drv))
                {
                    std::cerr << "rebalance debug algo #3: driver filtered the order out, order " << pv.id_
                         << ", driver " << drv.id_ << ", reason " << drv.dsf_.fr_ << std::endl;
                    b_passed_driver_filters = false;
                }
                 
                // TODO: in a future we need to find the best price for a view and the best ETA
                // if filters turn off
                // Probably this would be easier to do by calling price view twice: with and without filters
                if (!b_passed_rider_filters || !b_passed_driver_filters)
                    continue;
                 

                double pv_metric = the_metric(drv.position_where_, pv.A_, pv.B_, pv.price_);
             
                // Then calculate average metric for next orders (excluding the current non desirable one)
                // Orders - I MEAN IT. Not views. Because we have to be sure that IF pv becomes an order a driver
                // will have the next REAL order which totals in good average metric
                double avg = pv.sum_metrics_from_B_;
                /*int n = 0;
                for (auto &ord_i : unassigned_orders_)
                {
                    ++iterations3;
                    auto &ord = ord_i.second;
                    avg += drv.the_metric(pv.B_, ord.A_, ord.B_, ord.price_);
                    ++n;
                }*/
                
                // Hack. See 1 algorithm's comment
                // Note: divide by unassigned_orders_.size() not to unassigned_orders_.size() - 1
                //  because for a price view we consider all unassigned orders and for an
                //  unassigned order all but itself :-)
                if (unassigned_orders_.size())
                    avg /= unassigned_orders_.size();
                else
                    avg = pv_metric;

                pv_metric = (pv_metric + avg)/2;

                // Now we have pv_metric - the metric for pv
                // Take the price the pv need to have to have the SAME metric for the driver as the top order
                //  if it's positive and ZERO otherwise. Reason: we want each new order make drivers happier
                // Note: it can be bigger or lesser than the current price for pv - depends on if
                // pv_metric is bigger or lesser than the top order metric for this driver
                // Note: is there is no top order for a driver (e.g. no orders at all or
                // because of filters) then just reprice as pv is the top order - so its price
                // will be aligned with itself and will just zero driver's metric
                float pv_price_alligned_with_top_order;
                if (drv.top_order_.id_ == -1)
                    pv_price_alligned_with_top_order = reprice_as_good_as_this_metric_value(
                                drv.position_where_,pv.A_, pv.B_, pv.price_, pv_metric, 0.0);
                else
                    pv_price_alligned_with_top_order = reprice_as_good_as_this_metric_value(drv.position_where_,
                                drv.top_order_.A_, drv.top_order_.B_, pv.price_, pv_metric,
                                fmax(drv.top_order_metric_value_, 0.0));
                
                // Does the driver benefit from this price view having been placed?
                if (pv_metric > fmax(drv.top_order_metric_value_, 0.0))
                {
                    does_anybody_take_it = true;

                    if (pv_price_alligned_with_top_order < min_aligned_price_among_drivers_ready_to_take_it)
                    {
                        best_driver_benefit = &drv;
                        min_aligned_price_among_drivers_ready_to_take_it = pv_price_alligned_with_top_order;
                    }
                    
                    // TODO: save this price view as a top price view for the driver if it's better
                    // then previous price views for the same driver during this outer price view loop
                    // As a result each drivers will have the best price view id and that
                    // driver could be shown as a "bright" one on the map for the rider with this price view
                    // Right now it's only one best for a price view which is pv.best_driver_id_
                }
                else
                {
                    if (pv_price_alligned_with_top_order < min_aligned_price_among_drivers_not_ready_to_take_it)
                    {
                        best_driver_no_benefit = &drv;
                        min_aligned_price_among_drivers_not_ready_to_take_it = pv_price_alligned_with_top_order;
                    }
                }
                
            } // for (auto &drv_hash_entry : unassigned_drivers_)

            // Get the possible ETA and distance from the best driver to take this view once it'd be an order
            // Note: this info is useful for the rider and has to be shown up in her/his app
            // Note: if there is no best driver then this
            long double best_ETA_min = -1, best_pickup_route_length_km = -1;
            if (best_driver_benefit)
            {
                ETA_route_minutes_km(best_driver_benefit->position_where_, pv.A_, best_ETA_min, best_pickup_route_length_km);
                pv.best_driver_id_ = best_driver_benefit->id_;
            }
            else
            if (best_driver_no_benefit)
            {
                ETA_route_minutes_km(best_driver_no_benefit->position_where_, pv.A_, best_ETA_min, best_pickup_route_length_km);
                pv.best_driver_id_ = best_driver_no_benefit->id_;
            }
            else
                pv.best_driver_id_ = -1;

            float new_price = (does_anybody_take_it) ?
                min_aligned_price_among_drivers_ready_to_take_it:
                min_aligned_price_among_drivers_not_ready_to_take_it;

            // Notify the rider about the new price of the price view (it can be bigger or lesser)
            if (notifications_on_)
                rider_notifications_.push_back(rider_notification(
                                                                  rider_notification::rider_notification_view_price_change,now,
                                                              pv.rider_id_, pv.id_, pv.price_,new_price,0, 0, 0,
                                                              (double)best_ETA_min, (double)best_pickup_route_length_km
                ));
        
            // Change actual price in a price view.
            // It's needed at least for
            // 1. Having a real price in the order that will be inherited from this price view which
            //      is used to deeply calc driver's metric (inner unassigned order loop)
            // 2. dev/test purposes - to dump the marketplace
            // after each change
            pv.price_ = new_price;
        
            // Change the best ETA and distance in a price view
            pv.best_ETA_min_ = (double)best_ETA_min;
            pv.best_pickup_route_length_km_ = (double)best_pickup_route_length_km;
        
            // If the price is bad then log it to debug later
            if (pv.price_ <= 0)
            {
                std::cerr << "marketplace::force_rebalance: ERROR! the price for price view " << pv.id_ << " is not positive: " << pv.price_ << ", pv.best_ETA_min_=" << pv.best_ETA_min_ << ", pv.best_pickup_route_length_km_=" << pv.best_pickup_route_length_km_ << ", does_anybody_take_it=" << does_anybody_take_it << ", min_aligned_price_among_drivers_ready_to_take_it=" << min_aligned_price_among_drivers_ready_to_take_it << ", min_aligned_price_among_drivers_not_ready_to_take_it=" << min_aligned_price_among_drivers_not_ready_to_take_it << ", old_price=" << old_price<< std::endl;
            }

    } // for (auto &pv : price_views_)
    
    //printf("rebalance debug: before 4 step\n");fflush(stdout);
    
    // Forth crazy algorithm
    //
    // Now we need to do something with drivers with negative top orders
    // This means that they'd better not to work with us at all
    //
    // Why this can happen? This could be because a driver is in some place with too far away placed orders
    //
    // /*
    // Can we just ask a rider to increase the price? It depends. On the one hand we only can do it if the specific order is
    // negative to ALL DRIVERS, even for those who don't have it as a top one, because if it is positive to somebody
    // then it could be the second best option for that driver who will able to take it without bothering a rider
    // to increate the price. On the other hand the other driver could take its top order and nobody will take this
    // negative order which is bad for a rider and for drivers also.
    //
    // So it would be wise to ask a rider to increase the price for an order that is negative on the top of EACH driver
    // Otherwise drivers could never take it and will go offline
    //
    // */
    //
    // Note: we consider all the drivers, not online only. The reason for this is that one order may have more than
    // one notification about price increase. That's because some drivers are in a better position to this order than others. The
    // ones who offline could be in a worst position but anyway they can be lucky even offline to take some good order
    // that noone takes even for increased price
    //
    // Note: if there are many price-change notifications per order then the app selectes the smallest one. Why don't
    // send the smallest one only? Because if the order is still not taken and the rider is still waiting then
    // it's wise to send the next price increase to make this order visible to one more driver
    //
    // A simple way to hack negative metric problem is this:
    // 1.   Take a driver with the negative top order. Obviously all other orders also negative for the driver
    // 2.   Find among ALL orders the one that can be positive for this driver for the least price increase
    // 3.   Ask a rider about price increase. This is the minimum possible increase that will stimulate the driver to accept the order
    //
    //  How it will benefit drivers: they may receive better (more positive) orders in terms of profit (if riders
    //  agree to order after a price change)
    //  How it will benefit riders: their orders will be more likely to be accepted
    //  Downside: rider can cancel the order after being asked about price change
    //
    for (auto &drv_hash_entry : unassigned_drivers_)
    {
        driver &drv = drv_hash_entry.second;
        if (drv.top_order_metric_value_ < 0)
        {
            // Here is the driver with all negative orders
            // Now apply that crazy-two-inner-loop logic to find an order with the least price increase that
            // will make the driver metric as good as zero :-)
            double best_metric = -100;
            double min_price_increase = 1000000000;
            order *best_order = NULL;
            for (auto &ord_i : unassigned_orders_)
            {
                ++iterations4;
                auto &ord = ord_i.second;
                bool b_passed_rider_filters = true;
                bool b_passed_driver_filters = true;

                // Do we need to filter the order out?
                if (!passed_rider_filters(ord, drv))
                {
                    std::cerr << "rebalance debug algo #4: rider filtered the order out, order " << ord.id_
                         << ", driver " << drv.id_ << ", reason " << drv.dsf_.fr_ << std::endl;
                     b_passed_rider_filters = false;
                }
                if (!passed_driver_filters(ord, drv))
                {
                    std::cerr << "rebalance debug algo #4: driver filtered the order out, order " << ord.id_
                         << ", driver " << drv.id_ << ", reason " << drv.dsf_.fr_ << std::endl;
                    b_passed_driver_filters = false;
                }
                 
                // TODO: in a future we need to find the best price for an order if filters turn off
                // this will help drivers to have at least something desirable if they turn filters off
                if (!b_passed_rider_filters || !b_passed_driver_filters)
                    continue;

                double ord_metric = the_metric(drv.position_where_, ord.A_, ord.B_, ord.price_);
                
                //printf("rebalance debug: before inner loop in 4 step: ord_metric=%.2f, ord.price_=%.2f\n", ord_metric, ord.price_);fflush(stdout);
                
                double avg = ord.sum_metrics_from_B_;
                /*int n = 0;
                for (auto &another_order_i : unassigned_orders_)
                {
                    ++iterations4;
                    auto &another_order = another_order_i.second;
                    if (another_order.id_ != ord.id_)
                    {
                        avg += drv.the_metric(ord.B_, another_order.A_, another_order.B_, another_order.price_);
                        ++n;
                    }
                }*/
                
                // Hack. See 1 algorithm's comment
                if (unassigned_orders_.size() > 1)
                    avg /= unassigned_orders_.size() - 1;
                else
                    avg = ord_metric;
                
                ord_metric = (ord_metric + avg)/2;
        
                //printf("rebalance debug: before assert in 4 step: ord_metric=%.2f\n", ord_metric);fflush(stdout);
                
                if (ord_metric > 0)
                {
                    // This MUST not happen because the top order metric for "drv" is already negative and
                    // therefor "ord"'s metric also MUST be negative
                    printf("ASSERT!!! Fourh step of rebalance, ord_metric=%.2f, drv.id=%d, ord.id=%d\n", ord_metric,
                           (int)drv.id_, (int)ord.id_);fflush(stdout);
                    exit(0);
                }
   
                //printf("rebalance debug: after assert in 4 step");fflush(stdout);

                // Now ord_metric contains "ord"'s metric for "drv"

                // Reprice "ord" to make "drv"'s metric zero which obsiously will increase the order price
                float good_price = reprice_as_good_as_this_metric_value(drv.position_where_,
                                    ord.A_, ord.B_, ord.price_, ord_metric, 0);
                
                // Optimize for the price increase
                float price_increase = good_price - ord.price_;
                if (price_increase < min_price_increase)
                {
                    min_price_increase = price_increase;
                    best_order = &ord;
                    best_metric = ord_metric;
                }
            } // for (auto &the_order : unassigned_orders_)
            
            //printf("rebalance debug: before rider_notifications_.push_back in 4 step");fflush(stdout);

            // There can be no best order if there are no orders at all of if the driver or riders
            // filtered them out
            if (best_order)
            {
                // Now ask a rider about price increase
                // Note! There could be more than one notification on the order to change the price
                // That's ok. If the upper code receives them at once then it will take the maximum for each order to
                // sasitfy all the conditions upon the price increase has been asked
                if (notifications_on_)
                    rider_notifications_.push_back(rider_notification(
                                                                      rider_notification::rider_notification_order_price_change_positive,
                                                                      now,
                                                                      best_order->rider_id_, best_order->id_, best_order->price_,
                                                              best_order->price_ + min_price_increase,
                                                              best_metric, 0, drv.id_
                                                                      ));
            }
            
            // Now here could be one problem. If this driver is in the middle of nowhere then she/he will have
            // a lot of orders with bad metrics which will bring us to the situation when orders will be overpriced
            // TODO: hack this!

            // TODO: save suggested price in the order for dev/test reasons. The problem is here is that there
            // are a variety of those suggested prices per order - that's from each negative-metric-on-top driver
            // so we need to save all of them
        }
    }

    // Iterate through all orders and auto accept it by its best drivers IF a best driver
    // has this order on top - this is a win-win situation
    for (auto ord = unassigned_orders_.begin(); ord != unassigned_orders_.end(); )
    {
        // Note: this is because accept_order later if called will invalidate the iterator
        auto save = ord;
        ++save;
        
        if (ord->second.best_driver_id_ != -1)
        {
            auto best_drv_i = unassigned_drivers_.find(ord->second.best_driver_id_);
            if (best_drv_i == unassigned_drivers_.end())
            {
                std::cerr << "force_rebalance: ERROR! corrupted data: order " << ord->first << " has the best driver " << ord->second.best_driver_id_ << " who does not exist in unassigned_drivers_! Maybe this driver is already assigned another order?" << std::endl;
                best_drv_i = assigned_drivers_.find(ord->second.best_driver_id_);
                if (best_drv_i != assigned_drivers_.end())
                    std::cerr << "  force_rebalance: STILL ERROR! That driver is assigned to another order: " << best_drv_i->second.assigned_order_id_ << std::endl;
                else
                    std::cerr << "  force_rebalance: STILL ERROR! That driver is not in assigned list" << best_drv_i->second.assigned_order_id_ << std::endl;
            }
            else
            {
                // If the best driver for the order autoaccepts orders then check if it's win-win
                if (best_drv_i->second.dsf_.auto_accept_best_order_)
                {
                    if (best_drv_i->second.top_order_.id_ == ord->first)
                    {
                        // Win-win! Auto accept the order
                        // Note: the accept_order functions removes an order and a driver form
                        // unassigned, that's why all iterator to those are invalidated!
                        // That's why a) we save order/driver ids before dump it, b) save the iterator (earlier)
                        long long order_id = ord->first;
                        long long driver_id = best_drv_i->first;
                        
                        if (accept_order(ord->first, best_drv_i->first, now))
                            std::cerr << "force_rebalance: Win-win! The order " << order_id <<
                                " has been auto accepted by the driver " << driver_id << std::endl;
                        else
                            std::cerr << "force_rebalance: Win-win, but ERROR. The order " << order_id << " HAS NOT BEEN auto accepted by the driver " << driver_id << std::endl;
                        

                    }
                }
            }
        } // if (ord.second.best_driver_id_ != -1)

        ord = save;
        
    } // for (auto ord = unassigned_orders_.begin(); ord != unassigned_orders_.end(); )
    
    /*
    // Iterate through all drivers and auto accept best orders
    // If an order is desirable by many drivers then accept it by then one who is closer to it
    // TODO: implement only when I will get how to do it in a linear way. For now it's OK that there is a win-win case handled - better than nothing :-)
    for (auto &drv : unassigned_drivers_)
    {
        // If there is a top order for a driver
        if (drv.top_order_id_ != -1)
        {
            
        }
    }*/

    std::cerr << "force_rebalance: iterations0=" << iterations0 << ", iterations0pv="  << iterations0pv
        << ", iterations1=" << iterations1 << ", iterations2=" << iterations2 << ", iterations3="
        << iterations3 << ", iterations4=" << iterations4 << std::endl;
}

bool marketplace::get_order_points(long long order_id, geo::point *A, geo::point *B)
{
    auto ord_u = unassigned_orders_.find(order_id); // O(1)
    auto ord_a = assigned_orders_.find(order_id); // O(1)
    
    if (ord_u != unassigned_orders_.end())
    {
        *A = ord_u->second.A_;
        *B = ord_u->second.B_;
        return true;
    }
    else
    if (ord_a != assigned_orders_.end())
    {
        *A = ord_a->second.A_;
        *B = ord_a->second.B_;
        return true;
    }
    else
        return false;
}

geo::point marketplace::get_driver_position(long long driver_id)
{
    auto d_unassigned = unassigned_drivers_.find(driver_id); // O(1)
    auto d_assigned = assigned_drivers_.find(driver_id); // O(1)

    if (d_unassigned != unassigned_drivers_.end())
        return d_unassigned->second.position_where_;
    else
    if (d_assigned != assigned_drivers_.end())
        return d_assigned->second.position_where_;
    else
        return {.longitude = 0, .latitude = 0};
}

void marketplace::update_driver_position(long long driver_id, point position, time_t now)
{
    auto d_unassignd = unassigned_drivers_.find(driver_id); // O(1)
    auto d_assigned = assigned_drivers_.find(driver_id); // O(1)

    // If we have not found a driver then just add her/him
    if (d_unassignd == unassigned_drivers_.end() && d_assigned == assigned_drivers_.end())
        unassigned_drivers_.insert({driver_id, driver(driver_id, position, now)});
    // Update driver's position
    else
    {
        // Update driver's possition - either she/he is assigned or not
        if (d_unassignd != unassigned_drivers_.end())
        {
            d_unassignd->second.update_position(position, now);
            
            // Note: the driver CAN'T be both assigned and unassigned
            if (d_assigned != assigned_drivers_.end())
                std::cerr << "marketplace::update_driver_position: DATA CORRUPTED! driver " << driver_id << " is both assigned and unassigned" << std::endl;
        }
        else
        if (d_assigned != assigned_drivers_.end())
            d_assigned->second.update_position(position, now);
    }
    
    // Ajustment of any driver position can influence potential value of potential orders regardless
    // if the top order of the driver has been changed or not
    //
    // So we need to change price of potential orders accordingly
    // Howeever if the position change is a little bit and no changes have been happened since the
    // previous operation to the marketplace then rebalance does nothing but could be costy. Which is OK
    // because it internally it works fully fleged only once in a while :-)
    rebalance(now);
}

bool marketplace::update_price_view_A(long long rider_id, long long pv_id, point A, time_t now)
{
    auto p = price_views_.find(pv_id); // O(1)
    if (p == price_views_.end())
        return false;
    
    if (p->second.rider_id_ != rider_id || p->second.id_ != pv_id)
        return false;
    
    p->second.A_ = A;
    
    rebalance(now);
    
    return true;
}

bool marketplace::update_price_view_B(long long rider_id, long long pv_id, point B, time_t now)
{
    auto p = price_views_.find(pv_id); // O(1)
    if (p == price_views_.end())
        return false;
    
    if (p->second.rider_id_ != rider_id || p->second.id_ != pv_id)
         return false;
    
    p->second.B_ = B;
    
    rebalance(now);
    
    return true;
}

/*
void marketplace::driver_online(driver &drv)
{
	// Iterate all unassigned drivers. Can be 10K+ iterations for big towns
	for (auto d = unassigned_drivers_.begin(); d != unassigned_drivers_.end(); ++d)
		if (d->get_id() == drv.get_id())
		{
			// The driver is already here
			return;
		}

	rebalance(time(NULL));

	drv.erased_ = false;
	unassigned_drivers_.push_back(drv);
}*/

bool marketplace::add_price_view(order &&pv, time_t now)
{
    //printf("marketplace::add_price_view\n");
    //fflush(stdout);
    
    auto p = price_views_.find(pv.id_); // O(1)
    if (p == price_views_.end())
    {
        pv.erased_ = false;
        pv.dt_added_ = now;
        
        // The price of a brand new price view will be the one that comes
        // from "pv". Which is normally "0"
        // But that's OK because the end user will not be notified about the price of a price
        // view until rebalance
        
        price_views_.insert({pv.id_, pv});

        rebalance(now);
    
        return true;
    }
    else
        return false;
}

bool marketplace::add_unassigned_order(order &ord, time_t now)
{
    auto o = unassigned_orders_.find(ord.id_); // O(1)
    if (o == unassigned_orders_.end())
    {
        ord.erased_ = false;
        ord.dt_added_ = now;
        
        // Save order diastance
        ord.distance_km_ = manhattan_distance_km(ord.A_, ord.B_);
        
        // Save order price by tariff (because this order is not inherited from a price view and
        // therefor was created for test reasons)
        ord.price_ = tarif_price(ord.A_, ord.B_);
        
        unassigned_orders_.insert({ord.id_, ord});
        
        // This is for avoiding cubic algorithms in rebalace
        update_order_sum_metrics_from_B_on_order_appears(ord); // O(N)

        rebalance(now);
        
        return true;
    }
    else
        return false;

    //printf("marketplace::add_unassigned_order\n");
}

bool marketplace::convert_price_view_to_order(long long rider_id, long long pv_id, time_t now)
{
    auto p = price_views_.find(pv_id); // O(1)
    if (p != price_views_.end() && p->second.id_ == pv_id)
    {
        // Wrong rider
        if (p->second.rider_id_ != rider_id)
            return false;
            
        // Check if the order with the same id is already there
        if (unassigned_orders_.find(pv_id) != unassigned_orders_.end())
        {
            // Order with the same id is already there
            return false;
        }
        
        // If the order is already assigned then this means that something is corrupted
        if (assigned_orders_.find(pv_id) != assigned_orders_.end())
        {
            std::cerr << "marketplace::convert_price_view_to_order(rider_id=" << rider_id << ", pv_id=" << pv_id << "), price view is found in assigned_orders_!" << std::endl;
            return false;
        }

        // Insert a price view as an order to the order
        unassigned_orders_.insert({pv_id, p->second});

        // This is for avoiding cubic algorithms in rebalace
        update_order_sum_metrics_from_B_on_order_appears(p->second); // O(N)

        // Remove a price view from price views
        price_views_.erase(p);
            
        rebalance(now);
            
        return true;
    }
    
    return false;
}

bool marketplace::accept_order(long long order_id, long long driver_id, time_t now)
{
    auto ord_u = unassigned_orders_.find(order_id); // O(1)
    auto drv_u = unassigned_drivers_.find(driver_id); // O(1)

    // Wrong order_id or driver_id
    if (ord_u == unassigned_orders_.end() || drv_u == unassigned_drivers_.end())
        return false;

    auto ord_a = assigned_orders_.find(order_id); // O(1)
    auto drv_a = assigned_drivers_.find(driver_id); // O(1)

    // The order or the driver is already assigned
    // Note: be careful when check with end()!!! unassigned_orders_ and assigned_orders_ have
    //  the same type of iterators
    if (ord_a != assigned_orders_.end() || drv_a != assigned_drivers_.end())
        return false;

    // Exchange ids
    ord_u->second.assigned_driver_id_ = driver_id;
    drv_u->second.assigned_order_id_ = order_id;
    
    // Remember the time that a driver accepts the order
    ord_u->second.ts_accepted_ = now;
    
    // Assign the order and the driver
    assigned_orders_.insert({order_id, ord_u->second});
    assigned_drivers_.insert({driver_id, drv_u->second});

    // This is for avoiding cubic algorithms in rebalace
    update_order_sum_metrics_from_B_on_order_will_disappear(ord_u->second); // O(N)
    
    // Remove the order and the driver from unassigned
    unassigned_orders_.erase(ord_u);
    unassigned_drivers_.erase(drv_u);
            
    rebalance(now);
    
    return true ;
}

bool marketplace::finish_ride(long long driver_id, time_t now)
{
    // Search for a driver
    auto drv_a = assigned_drivers_.find(driver_id); // O(1)
    if (drv_a == assigned_drivers_.end())
        return false;
    
    // Search for an assigned order
    long long order_id = drv_a->second.assigned_order_id_;
    if (order_id == -1)
        return false;
    auto ord_a = assigned_orders_.find(order_id); // O(1)
    if (ord_a == assigned_orders_.end())
    {
        auto ord_u = unassigned_orders_.find(order_id);
        std::cerr << "marketplace::finish_ride: ERROR corrupted data! Driver " << driver_id << " has an assigned order " << order_id << " which is not found in assigned list. And it is "
        << ((ord_u == unassigned_orders_.end()) ? "NOT FOUND" : "FOUND") << " in unassigned list" << std::endl;
        return false;
    }
    
    // Remove this order from the driver
    drv_a->second.assigned_order_id_ = -1;

    // Unset everything regarding the best order just in case if
    // marketplace rebalance does not go for any reason
    drv_a->second.top_order_.id_ = -1;
    drv_a->second.top_order_.rider_id_ = -1;
    drv_a->second.top_order_desirability_ = 0;
    drv_a->second.top_order_pointer_ = NULL;
    drv_a->second.top_order_metric_value_ = -1000;
    
    // Add the driver to the unassigned list
    unassigned_drivers_.insert({driver_id, drv_a->second});
    // Remove the driver from the assigned list
    assigned_drivers_.erase(drv_a);
    
    // Save the rider in billing/stat
    driver_stat *ds = get_driver_stat_impl_(driver_id);
    if (ds)
    {
        // Add the entry to the stat
        driver_stat_entry_ride dser;
        dser.ts_order_accepted_ = ord_a->second.ts_accepted_;
        dser.price_ = ord_a->second.price_ + ord_a->second.price_for_toll_wait_;
        dser.ride_status_ = 1; // finished
        dser.real_distance_to_A_ = 0; // TODO!
        dser.real_seconds_to_A_ = ord_a->second.ts_driver_at_A_ - ord_a->second.ts_accepted_;
        dser.real_distance_A_to_B_ = 0; // TODO!
        dser.real_seconds_A_to_B_ = now - ord_a->second.ts_driver_at_A_;
        
        // Note don't fill fee_ here, as it will be filled in add_driver_stat_entry
        
        ds->add_driver_stat_entry(dser);
    }
    
    // Remove the order (and don't add it anywhere)
    assigned_orders_.erase(ord_a);
    
    // Now the driver is unassigned and ready for new orders and the order
    // is completely finished and removed from the marketplace

    rebalance(now);
    
    return true;
}

bool marketplace::change_order_price(long long order_id, float new_price, time_t now)
{
    auto o = unassigned_orders_.find(order_id); // O(1)
    if (o != unassigned_orders_.end() && o->second.id_ == order_id)
    {
        double old_price = o->second.price_;
        
        // Change the price of the order and marks the order
        // as the one with changed price
        o->second.price_ = new_price;
        o->second.rider_suggested_price_ = true;

        // This is for avoiding cubic algorithms in rebalace
        update_order_sum_metrics_from_B_on_order_change_price(o->second, old_price); // O(N)
        
        // Rebalance. Because this order is more likely to accept now, so we need to make it top for some drivers :-)
        rebalance(now);
        
        return true;
    }
    else
        return false;
}


bool marketplace::remove_driver(long long driver_id, time_t now)
{
    auto d = unassigned_drivers_.find(driver_id); // O(1)
    if (d != unassigned_drivers_.end() && d->second.get_id() == driver_id)
    {
        d->second.erased_ = true;
        rebalance(now);
        return true;
    }
    else
        return false;
}

bool marketplace::set_auto_accept_best_order_flag(long long driver_id, bool flag, time_t now)
{
    auto d = unassigned_drivers_.find(driver_id); // O(1)
    if (d != unassigned_drivers_.end() && d->second.get_id() == driver_id)
    {
        d->second.dsf_.auto_accept_best_order_ = flag;

        // If autoaccept is set the do the rebalance NOW to take the order now if
        // there is a win-win situation
        if (flag)
            force_rebalance(now);
        else
            rebalance(now);
        
        return true;
    }
    else
        return false;
}

bool marketplace::cancel_price_view(long long id, time_t now)
{
	// Mark the price view deleted
	// It still will be used unless it will have been deleted but not for so long until the next rebalance

    auto p = price_views_.find(id); // O(1)
    if (p != price_views_.end() && p->second.id_ == id)
    {
        p->second.erased_ = true;
        rebalance(now);
        return true;
    }
    else
        return false;
}

bool marketplace::remove_unassigned_order(long long order_id, time_t now, order *ord)
{
	// Mark the unassigned order deleted
	// It still will be used unless it will have been deleted but not for so long until the next rebalance

    auto o = unassigned_orders_.find(order_id); // O(1)
    if (o != unassigned_orders_.end() && o->second.id_ == order_id)
    {
        // Save erased order
        if (ord)
            *ord = o->second;
            
        o->second.erased_ = true;
        rebalance(now);
        return true;
    }
    else
        return false;
}

order marketplace::get_top_order(long long driver_id)
{
    auto d = unassigned_drivers_.find(driver_id); // O(1)
    if (d != unassigned_drivers_.end() && d->second.get_id() == driver_id)
        return d->second.get_top_order();
	else
        return order();
}

order marketplace::get_assigned_order(long long driver_id)
{
    order *o = get_assigned_order_impl_(driver_id);
    return o ? *o : order();
}

order *marketplace::get_assigned_order_impl_(long long driver_id)
{
    auto d = assigned_drivers_.find(driver_id); // O(1)
    if (d != assigned_drivers_.end() && d->second.get_id() == driver_id)
    {
        long long order_id = d->second.assigned_order_id_;
        if (order_id == -1)
        {
            std::cerr << "marketplace::get_assigned_order_impl_: ERROR! corrupted data: driver " << driver_id << " is assigned an order with id -1" << std::endl;
            return NULL;
        }
        
        auto o = assigned_orders_.find(order_id); // O(1)
        if (o == assigned_orders_.end())
        {
            std::cerr << "marketplace::get_assigned_order_impl_: ERROR! corrupted data: driver " << driver_id << " is assigned an order with id " << order_id << " which is not found in the assigned list" << std::endl;
            return NULL;
        }
        
        return &o->second;
    }
    else
        // No data corruption, but the driver just is not in the assigned list
        return NULL;
}

driver_stat *marketplace::get_driver_stat_impl_(long long driver_id)
{
    auto ds = driver_stat_.find(driver_id); // O(1)
    if (ds != driver_stat_.end())
        return &ds->second;
    else
    {
        // Create an entry in the stat
        return &driver_stat_[driver_id];
    }
}

bool marketplace::driver_got_A(long long driver_id, time_t now)
{
    order *o = get_assigned_order_impl_(driver_id);
    if (!o)
        return false;
    
    o->ts_driver_at_A_ = now;
    rebalance(now);
    
    return true;
}

bool marketplace::driver_started_ride(long long driver_id, time_t now)
{
    order *o = get_assigned_order_impl_(driver_id);
    if (!o)
        return false;
    
    // If a driver has never been at A then she/he can't start a ride :-)
    if (o->ts_driver_at_A_ == (time_t)-1)
        return false;
    
    o->ts_ride_started_ = now;
    rebalance(now);
    
    return true;
}

int marketplace::driver_accepted_toll_wait(long long driver_id, int seconds, time_t now)
{
    order *o = get_assigned_order_impl_(driver_id);
    if (!o)
        return -1;
    
    // If a driver has never been at A then there is no wait and the toll wait all the more so :-)
     if (o->ts_driver_at_A_ == (time_t)-1)
         return -1;
    
    // That's the total wait (not only toll!)
    int wait_seconds = ((o->ts_ride_started_ == (time_t)-1 || o->ts_ride_started_ == 0) ? now : std::min(now, o->ts_ride_started_)) - o->ts_driver_at_A_;

    // Didn't make it to the toll
    if (wait_seconds < params::free_wait_duration())
    {
        std::cerr << "marketplace::driver_accepted_toll_wait(" << driver_id << ", " << seconds << "): didn't make it to the toll: wait_seconds = " << wait_seconds << ", params::free_wait_duration()=" << params::free_wait_duration() << ", o->ts_ride_started_" << o->ts_ride_started_ << ", now=" << now << ", o->ts_ride_started_=" << o->ts_ride_started_ << std::endl;
        return 0;
    }
    
    // The real number of toll seconds
    int toll_seconds = std::min(wait_seconds - params::free_wait_duration(), seconds);
    
    // Adjust the toll price
    // Note: it can be adjusted more than once if the driver calls driver_accepted_toll_wait
    o->price_for_toll_wait_ = ((toll_seconds + 59) / 60) * params::toll_wait_minute_price();
    
    return toll_seconds;
}

bool marketplace::driver_cancelled_order(long long driver_id, time_t now)
{
    /*
     *      Order cancellation by a driver is very similar to finish the ride
     *      The only difference is that if the order is cancelled BEFORE the ride
     *      is started - the order is moved to unassigned (to find a new driver)
     *      Otherwise the order is removed - because a rider could be in another place
     *      and have different plans
     */
    
    // Search for a driver
     auto drv_a = assigned_drivers_.find(driver_id); // O(1)
     if (drv_a == assigned_drivers_.end())
         return false;
     
     // Search for an assigned order
     long long order_id = drv_a->second.assigned_order_id_;
     if (order_id == -1)
         return false;
     auto ord_a = assigned_orders_.find(order_id); // O(1)
     if (ord_a == assigned_orders_.end())
     {
         auto ord_u = unassigned_orders_.find(order_id);
         std::cerr << "marketplace::driver_cancelled_order: ERROR corrupted data! Driver " << driver_id << " has an assigned order " << order_id << " which is not found in assigned list. And it is "
         << ((ord_u == unassigned_orders_.end()) ? "NOT FOUND" : "FOUND") << " in unassigned list" << std::endl;
         return false;
     }
     
    // Cancellation after got A and before ride started is possible ONLY after toll wait has started
    if (ord_a->second.ts_driver_at_A_ != (time_t)-1 &&
            (ord_a->second.ts_ride_started_ == (time_t)-1 || ord_a->second.ts_ride_started_ == 0))
    {
        int wait_seconds = now - ord_a->second.ts_driver_at_A_;
        
        // To early to cancel
        if (wait_seconds <= params::free_wait_duration())
            return false;
    }
    
     // Remove this order from the driver
     drv_a->second.assigned_order_id_ = -1;

     // Unset everything regarding the best order just in case if
     // marketplace rebalance does not go for any reason
     drv_a->second.top_order_.id_ = -1;
     drv_a->second.top_order_.rider_id_ = -1;
     drv_a->second.top_order_desirability_ = 0;
     drv_a->second.top_order_pointer_ = NULL;
     drv_a->second.top_order_metric_value_ = -1000;

    // Save the ride in billing/stat
      driver_stat *ds = get_driver_stat_impl_(driver_id);
      if (ds)
      {
          // Add the entry to the stat
          driver_stat_entry_ride dser;
          dser.ts_order_accepted_ = ord_a->second.ts_accepted_;
          dser.price_ = ord_a->second.price_ + ord_a->second.price_for_toll_wait_;
          dser.ride_status_ = 2; // cancelled by a driver
          dser.real_distance_to_A_ = 0; // TODO!
          dser.real_seconds_to_A_ = ord_a->second.ts_driver_at_A_ - ord_a->second.ts_accepted_;
          dser.real_distance_A_to_B_ = 0; // TODO!
          dser.real_seconds_A_to_B_ = now - ord_a->second.ts_driver_at_A_;

          ds->add_driver_stat_entry(dser);
      }
    
     // Add the driver to the unassigned list
     unassigned_drivers_.insert({driver_id, drv_a->second});
     // Remove the driver from the assigned list
     assigned_drivers_.erase(drv_a);
     
    // If the ride is already started then just remove the order
    if (ord_a->second.ts_ride_started_ != (time_t)-1 && ord_a->second.ts_ride_started_ != 0)
    {
        // Remove the order (and don't add it anywhere)
        assigned_orders_.erase(ord_a);
    }
    else
    {
        // Turn the order into unassigned
        ord_a->second.best_driver_id_ = -1;
        ord_a->second.best_ETA_min_ = -1;
        ord_a->second.best_pickup_route_length_km_ = -1;
        ord_a->second.ETA_ = -1;
        ord_a->second.distance_km_ = -1;
        ord_a->second.assigned_driver_id_ = -1;
        ord_a->second.ts_driver_at_A_ = -1;
        ord_a->second.ts_ride_started_ = -1;
        ord_a->second.price_for_toll_wait_ = -1;
        
        unassigned_orders_.insert({order_id, ord_a->second});

        // This is for avoiding cubic algorithms in rebalace
        update_order_sum_metrics_from_B_on_order_appears(ord_a->second); // O(N)

        // Erase the order from the assigned
        assigned_orders_.erase(ord_a);
    }
    
     // Now the driver is unassigned and ready for new orders
     rebalance(now);

    return true;
}


bool marketplace::rider_cancelled_order(long long order_id, time_t now)
{
    /*
     *      Order cancellation by a rider works like this
     *      1.  If an order is unassigned then it is just converted back to a price view
     *      2.  If an order is assigned and a ride has not started yet then it is
     *          converted back to a price view
     *      3.  Otherwise an order can be cancelled. If a rider wants to quit the car then they
     *          will do it anyway, but the order will be closed by a driver
     */
    
    auto ord_u = unassigned_orders_.find(order_id); // O(1)
    auto ord_a = assigned_orders_.find(order_id); // O(1)
    
    // The order does not exist
    if (ord_u == unassigned_orders_.end() && ord_a == assigned_orders_.end())
    {
        std::cerr << "marketplace::rider_cancelled_order: can't cancel, order " << order_id << " does not exist" <<
            std::endl;
        return false;
    }
    
    // The ride is started - can't cancel
    if (ord_a != assigned_orders_.end() &&
        ord_a->second.ts_ride_started_ != 0 && ord_a->second.ts_ride_started_ != -1)
    {
        std::cerr << "marketplace::rider_cancelled_order: can't cancel, order " << order_id << " because a ride has already started" << std::endl;
        
        if (ord_u != unassigned_orders_.end())
            std::cerr << "marketplace::rider_cancelled_order: corrupted data(1)! The order " << order_id <<
            " is both unassigned and assigned" << std::endl;
        
        return false;
    }

    // Next steps will convert an order to a price view anyway
    // That's why check if it's already in price views
    if (price_views_.find(order_id) != price_views_.end())
    {
        std::cerr << "marketplace::rider_cancelled_order: corrupted data(2)! Can't cancel an unassigned/assigned order, order " << order_id << " because it's already in price views" << std::endl;
        return false;
    }
    
    // The order is unassigned - convert it back to the price view
    if (ord_u != unassigned_orders_.end())
    {
        if (ord_a != assigned_orders_.end())
            std::cerr << "marketplace::rider_cancelled_order: corrupted data(3)! The order " << order_id <<
            " is both unassigned and assigned" << std::endl;
 
        // This is for avoiding cubic algorithms in rebalace
        update_order_sum_metrics_from_B_on_order_will_disappear(ord_u->second); // O(N)

        // Convert the unassigned order to the price view
        ord_u->second.rider_suggested_price_ = false;
        ord_u->second.suggested_price_ = 0;
        ord_u->second.sum_metrics_from_B_ = 0;
        
        // Insert it to price views
        price_views_.insert({order_id, ord_u->second});

        // Remove the order from unassigned
        unassigned_orders_.erase(ord_u);
    }
    // The order is assigned and the ride has not started yet - convert it back to price view
    // plus unassign a driver from the order
    else
    {
        // Find a driver for this assigned order
        long long driver_id = ord_a->second.assigned_driver_id_;
        
        auto drv_a = assigned_drivers_.find(driver_id); // O(1)
        if (drv_a == assigned_drivers_.end())
        {
            // No driver  corrupted data
            std::cerr << "marketplace::rider_cancelled_order: corrupted data(4)! The order " << order_id <<
                " is both assigned and without an assigned driver. driver_id=" << driver_id << std::endl;
            
            return false;
        }
        
        // Convert the assigned order to the price view
        ord_a->second.rider_suggested_price_ = false;
        ord_a->second.suggested_price_ = 0;
        
        ord_a->second.best_driver_id_ = -1;
        ord_a->second.best_ETA_min_ = -1;
        ord_a->second.best_pickup_route_length_km_ = -1;
        ord_a->second.ETA_ = -1;
        ord_a->second.distance_km_ = -1;
        ord_a->second.assigned_driver_id_ = -1;
        ord_a->second.ts_driver_at_A_ = -1;
        ord_a->second.ts_ride_started_ = -1;
        ord_a->second.price_for_toll_wait_ = -1;
        ord_a->second.sum_metrics_from_B_ = 0;
        
        // Remove this order from the driver
        drv_a->second.assigned_order_id_ = -1;

        // Unset everything regarding the best order just in case if
        // marketplace rebalance does not go for any reason
        drv_a->second.top_order_.id_ = -1;
        drv_a->second.top_order_.rider_id_ = -1;
        drv_a->second.top_order_desirability_ = 0;
        drv_a->second.top_order_pointer_ = NULL;
        drv_a->second.top_order_metric_value_ = -1000;

        // Save the ride in billing/stat
        driver_stat *ds = get_driver_stat_impl_(driver_id);
        if (ds)
        {
            // Add the entry to the stat
            driver_stat_entry_ride dser;
            dser.ts_order_accepted_ = ord_a->second.ts_accepted_;
            dser.price_ = ord_a->second.price_ + ord_a->second.price_for_toll_wait_;
            dser.ride_status_ = 3; // cancelled by a rider
            dser.real_distance_to_A_ = 0; // TODO!
            dser.real_seconds_to_A_ = ord_a->second.ts_driver_at_A_ - ord_a->second.ts_accepted_;
            dser.real_distance_A_to_B_ = 0; // TODO!
            dser.real_seconds_A_to_B_ = now - ord_a->second.ts_driver_at_A_;

            ds->add_driver_stat_entry(dser);
        }
        
        // Add the driver to the unassigned list
        unassigned_drivers_.insert({driver_id, drv_a->second});
        // Remove the driver from the assigned list
        assigned_drivers_.erase(drv_a);
         
        // Add the order to the price views
        price_views_.insert({order_id, ord_a->second});

        // Note: this is the fact that an assigned order disaper and a price view appears
        // We don't need to call update_order_sum_metrics_from_B_on_order_* here because those
        //  methods for unassigned orders only
        
        // Remove the order from the assigned list
        assigned_orders_.erase(ord_a);
    }

    // Now the driver is unassigned and ready for new orders
    // And the rider has a new price view and is ready for a new order
    rebalance(now);

    return true;
}

bool marketplace::get_good_price_and_metric(long long driver_id, long long order_id, double &price, double &metric)
{
    auto d = unassigned_drivers_.find(driver_id); // O(1)
    if (d != unassigned_drivers_.end() && d->second.get_id() == driver_id)
    {
        auto o = unassigned_orders_.find(order_id); // O(1)
        if (o != unassigned_orders_.end() && o->second.id_ == order_id)
        {
            metric = the_metric(d->second.position_where_, o->second.A_, o->second.B_, o->second.price_);
            price = reprice_as_good_as_this_metric_value(d->second.position_where_, o->second.A_, o->second.B_, 0, o->second.price_, 0);
            return true;
        }
    }
    
    return false;
}



long double marketplace::the_metric(point current, point A, point B, long double price)
{
    // Pretend moving from current to A
    long double ETA_to_A_min;
    long double route_to_A_length_km;
    ETA_route_minutes_km(current, A, ETA_to_A_min, route_to_A_length_km);
    
    // Pretend waiting a rider at A 3 minutes
    ETA_to_A_min += 3;
    
    // Pretend moving from A to B
    long double ETA_A_to_B_min;
    long double route_A_to_B_length_km;
    ETA_route_minutes_km(A, B, ETA_A_to_B_min, route_A_to_B_length_km);

    // Pretend waiting a rider to get out at B 1 minute
    ETA_A_to_B_min += 1;
    
    long double metric = 0;
    
    /*switch (metric_type_)
    {
        case driver_metric_revenue_per_second:
            metric = price / (ETA_to_A_min + ETA_A_to_B_min);
            break;
        case driver_metric_revenue_per_m:
            metric = price / (route_to_A_length_km + route_A_to_B_length_km);
            break;
        case driver_metric_profit_per_second:*/
            metric = (price - params::cost_per_km() * (route_to_A_length_km + route_A_to_B_length_km)) / (ETA_to_A_min + ETA_A_to_B_min);
            /*break;
    }*/

    return metric;
}

/*
bool driver::is_better_than_top(const order &ord)
{
    // If there is not top order yet
    if (!top_order_.id_)
        return true;
    
	// Note: we need to calculate the metric of the top order on the fly because it can
	// be changes unexpectedly due to driver position changes
    return the_metric(position_where_, ord.A_, ord.B_, ord.price_) >
            the_metric(position_where_, top_order_.A_, top_order_.B_, top_order_.price_);
}*/

float marketplace::reprice_as_good_as_this_metric_value(point current, point A, point B,
                                                   float bad_price, double bad_metric_value, double good_metric_value)
{
    // Pretend moving from current to A
    long double ETA_to_A_min;
    long double route_to_A_length_km;
    ETA_route_minutes_km(current, A, ETA_to_A_min, route_to_A_length_km);
    
    // Pretend waiting a rider at A 3 minutes
    ETA_to_A_min += 3;

    // Pretend moving from A to B
    long double ETA_A_to_B_min;
    long double route_A_to_B_length_km;
    ETA_route_minutes_km(A, B, ETA_A_to_B_min, route_A_to_B_length_km);

    // Pretend waiting a rider to get out at B 1 minute
    ETA_A_to_B_min += 1;

    // So what the price should be for (current -> A -> B) to have the metric equal to good_metric_value
	float good_price = 0;

	/*switch (metric_type_)
	{
		case driver_metric_revenue_per_second:
			good_price = bad_price + (good_metric_value-bad_metric_value) * (ETA_to_A_min + ETA_A_to_B_min);
			break;
		case driver_metric_revenue_per_m:
			good_price = bad_price + (good_metric_value-bad_metric_value) * (route_to_A_length_km + route_A_to_B_length_km);
			break;
		case driver_metric_profit_per_second:*/
			good_price = bad_price + (good_metric_value-bad_metric_value) * (ETA_to_A_min + ETA_A_to_B_min)
                + params::cost_per_km()/*cost_per_km_*/ * (route_to_A_length_km + route_A_to_B_length_km);
			/*break;
	}*/

	return good_price;
}

bool marketplace::get_driver_stat_entries(long long driver_id, time_t from, time_t to, std::vector<driver_stat_entry_ride> &result)
{
    auto *stat = get_driver_stat_impl_(driver_id);
    if (!stat)
        return false;
    stat->get_driver_stat_entries(from, to, result);
    return true;
}

bool marketplace::get_driver_stat_aggr_entries(long long driver_id, time_t from, time_t to, std::vector<driver_stat_entry_ride> &result)
{
    auto *stat = get_driver_stat_impl_(driver_id);
    if (!stat)
        return false;
    stat->get_driver_stat_aggr_entries(from, to, result);
    return true;
}

bool marketplace::get_driver_stat_entry_shift_purchase(long long driver_id, time_t from, time_t to, std::vector<driver_stat_entry_shift_purchase> &result)
{
    auto *stat = get_driver_stat_impl_(driver_id);
    if (!stat)
        return false;
    stat->get_driver_stat_entry_shift_purchase(from, to, result);
    return true;
}

bool marketplace::get_driver_stat_payments(long long driver_id,time_t from, time_t to, std::vector<driver_stat_payment> &result)
{
    auto *stat = get_driver_stat_impl_(driver_id);
    if (!stat)
        return false;
    stat->get_driver_stat_payments(from, to, result);
    return true;
}

bool marketplace::get_driver_stat_withdrawals(long long driver_id,time_t from, time_t to, std::vector<driver_stat_withdrawal> &result)
{
    auto *stat = get_driver_stat_impl_(driver_id);
    if (!stat)
        return false;
    stat->get_driver_stat_withdrawals(from, to, result);
    return true;
}

bool marketplace::get_driver_stat_cashbacks(long long driver_id,time_t from, time_t to, std::vector<driver_stat_cashback> &result)
{
    auto *stat = get_driver_stat_impl_(driver_id);
    if (!stat)
        return false;
    stat->get_driver_stat_cashbacks(from, to, result);
    return true;
}

bool marketplace::get_driver_stat_bonuses(long long driver_id,time_t from, time_t to, std::vector<driver_stat_bonus> &result)
{
    auto *stat = get_driver_stat_impl_(driver_id);
    if (!stat)
        return false;
    stat->get_driver_stat_bonuses(from, to, result);
    return true;
}

bool marketplace::get_driver_stat_promos(long long driver_id,time_t from, time_t to, std::vector<driver_stat_promo> &result)
{
    auto *stat = get_driver_stat_impl_(driver_id);
    if (!stat)
        return false;
    stat->get_driver_stat_promos(from, to, result);
    return true;
}

driver_stat_balance marketplace::restore_driver_balance(long long driver_id, time_t ts)
{
    auto *stat = get_driver_stat_impl_(driver_id);
    if (!stat)
        return driver_stat_balance{-1, -1, -1, -1, -1};
    return stat->restore_driver_balance(ts);
}

} // namespace marketplace
