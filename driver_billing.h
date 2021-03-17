/*
*	driver_billing.h
*
*	(C) Denis Anikin 2020
*
*	Headers for driver billing 
*
*/

#ifndef _driver_billing_h_included_
#define _driver_billing_h_included_

#include <time.h>
#include <vector>
#include <map>

namespace marketplace
{

// Entry about an order/ride
struct driver_stat_entry_ride
{
    // When order was accepted by a driver
    time_t ts_order_accepted_;
    
    // When the order was finished
    time_t ts_;
    
    // Total price with all extras (0 if cancelled)
    double price_;
    // 1    -   finished
    // 2    -   cancelled by a driver
    // 3    -   cancelled by a rider
    char ride_status_;

    // Real distances and duration of both parts of a trip
    int real_distance_to_A_;
    int real_seconds_to_A_;
    int real_distance_A_to_B_;
    int real_seconds_A_to_B_;
    
    // Real comission to be payed (0 if not aplicable)
    double fee_;
};

template <class T>
inline int operator<(const T &a, time_t t)
{
    return a.ts_ < t;
}

// Entry about the fact of purchase of the shift
struct driver_stat_entry_shift_purchase
{
    // Which date the shift is purchased for
    // Note: a shift is purchases for 24 hours. If a driver
    //  wishes to purchase a shift for week (7 days), then it
    //  will look like 7 adjacent entries of driver_stat_entry_shift_purchase
    // Note: "ts_" is the time shift ends, so it is something like 23.08.2020 23:59:59
    time_t ts_;
    
    // The fee paid for purchasing
    double fee_;
};

// Entry for the payment
struct driver_stat_payment
{
    // When the payment was done
    time_t ts_;
    
    // Amount of payment
    double amount_;
    
    // Agent of payment
    //  1   -   wire
    //  2   -   QIWI
    //  3   -   Eleksnet
    //  4   -   Sberbank.Online
    int agent_;
};

// Entry for the withdrawal
struct driver_stat_withdrawal
{
    // When the withdrawal was done
    time_t ts_;
    
    // Amout of withdrawal
    double amount_;
    
    // Agent of withdrawal
    //  1   -   wire
    //  2   -   QIWI
    //  3   -   Eleksnet
    //  4   -   Sberbank.Online
    //  5   -   MTS
    //  6   -   Beeline
    //  7   -   Megafon
    //  8   -   Tele2
    int agent_;
};

// Entry for the cashback
struct driver_stat_cashback
{
    // When the cashback was done
    time_t ts_;
    
    // Amout of cashback
    double amount_;
    
    // Reason
    //  1   -   app usage with push and GPS on for 24 hours
    int cashback_reason_;
};

// Entry for the money bonus
struct driver_stat_bonus
{
    // When the bonus was granted
    time_t ts_;
    
    // Amount of bonus
    double amount_;
    
    //  1   -   app installation
    //  2   -   referal
    int bonus_type_;
};

// Entry for the promo (non-money bonus)
struct driver_stat_promo
{
    // When the bonus was granted
    time_t ts_;
    
    // When the promo starts
    time_t ts_start_;
    
    // When the promo finishes
    time_t ts_finish_;
    
    //  1   -   app installation -> no fee from ts_start_ till ts_finish_
    int promo_type_;
};

// Driver's balance for a date
struct driver_stat_balance
{
    // The amount which the driver has deposited up to date (positive)
    double payment_amout_;
    
    // The amount which the driver has received as cashbacks up to date (positive)
    double cashbacks_;
    
    // The amount which the driver has received as bonuses up to date (positive)
    double bonuses_;

    
    
    // The amount that has been withheld for fees and comissions up to date (negative)
    double withheld_;
    
    // The amount that the driver has withdrawn up to date (negative)
    double withdrawals_;
};

// Statistics and history on driver's orders
class driver_stat
{
public:
    
    // Adds an entry each time a ride finishes
    void add_driver_stat_entry(driver_stat_entry_ride &entry);
    
    // Returns a list of driver stat entries between two dates
    void get_driver_stat_entries(time_t from, time_t to, std::vector<driver_stat_entry_ride> &result);
    
    // Returns a list of driver stat entries between two dates aggregated by weeks
    void get_driver_stat_aggr_entries(time_t from, time_t to, std::vector<driver_stat_entry_ride> &result);

    // Returns driver's balance for a specific date
    // Note: the balance is calculated from scratch
    driver_stat_balance restore_driver_balance(time_t ts);
    
    // Make a shift purchase
    // ts - is when the purcashed shift ends. It will work 24 hours beforehand
    void purchase_shift(time_t ts);
    
    // Make a payment by a driver via a specified agent
    void make_payment(double amount, int agent);
    
    // Withdraw money via a specified agent
    // Note: only cashbacks can be withdrawn
    // Returns the amount which has actually been withdrawn
    double withdraw_money(double amount, int agent);
    
    // Returns shift purchases for the period
    void get_driver_stat_entry_shift_purchase(time_t from, time_t to, std::vector<driver_stat_entry_shift_purchase> &result);
    
    // Returns payments for the period
    void get_driver_stat_payments(time_t from, time_t to, std::vector<driver_stat_payment> &result);

    // Returns withdrawals for the period
    void get_driver_stat_withdrawals(time_t from, time_t to, std::vector<driver_stat_withdrawal> &result);

    // Returns cashbacks for the period
    void get_driver_stat_cashbacks(time_t from, time_t to, std::vector<driver_stat_cashback> &result);

    // Returns bonuses for the period
    void get_driver_stat_bonuses(time_t from, time_t to, std::vector<driver_stat_bonus> &result);

    // Returns bonuses for the period
    void get_driver_stat_promos(time_t from, time_t to, std::vector<driver_stat_promo> &result);
    
    // Grants a cashback for app usage
    // Note: cashback is given no more than once a day
    void cashback_app_usage(time_t now);
    
    // One-time bonus for app installation
    bool bonus_app_install(time_t now);
    
    // One-time promo for app installation
    bool promo_app_install(time_t now);
    
private:

    template <class T, class O>
    void get_objects_from_to_impl_(time_t from, time_t to, std::vector<T> &result, O &object)
    {
        result.clear();
        auto i = std::lower_bound(object.begin(), object.end(), from);
        while (i != object.end())
        {
            if (i->ts_ > to)
                break;
            result.push_back(*i);
            ++i;
        }
    }
    
    // Driver promos ordered by ts_start_;
    std::vector<driver_stat_promo> driver_stat_promos_;

    // Driver bonuses;
    std::vector<driver_stat_bonus> driver_stat_bonuses_;
    
    // All rides per a driver sorted by ts when the order was accepted
    std::vector<driver_stat_entry_ride> driver_stat_entry_rides_;
    
    // All purchased shifts
    std::vector<driver_stat_entry_shift_purchase> driver_stat_entry_shift_purchases_;
    
    // All payments of a driver
    // Note: a vector not a map because we fullscan it for the balance which is faster
    //  for a vector than for a map
    std::vector<driver_stat_payment> driver_stat_payments_;
    
    // All cashbacks of a drivr
    std::vector<driver_stat_cashback> driver_stat_cashbacks_;
    
    // All withdrawals of a driver
    std::vector<driver_stat_withdrawal> driver_stat_withdrawals_;
};

}

#endif

