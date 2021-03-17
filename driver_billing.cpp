/*
*	driver_billing.cpp
*
*	(C) Denis Anikin 2020
*
*	Impl for driver billing
*
*/

#include <algorithm>

#include "driver_billing.h"

namespace marketplace
{

struct billing_params
{
    // Fee for an order to a driver in the share of an order total price
    static double fee_for_order() { return 0.03; }
    
    // For for a 24 hrs shift
    static double fee_for_shift() { return 100; }
    
    // Cashback for app usage
    static double cashback_app_usage() { return 10; }
    
    // One-time bonus for app installation
    static double bonus_app_install() { return 15000; }
    
    // Number of days without a fee by a promo
    static int days_no_fee_promo() { return 90; }
};

void driver_stat::add_driver_stat_entry(driver_stat_entry_ride &entry)
{
    time_t now = time(NULL);
    
    entry.ts_ = now;
    
    // If it's cancelled then the price and the fee are zero
    if (entry.ride_status_ != 1)
    {
        entry.price_ = 0;
        entry.fee_ = 0;
    }
    
    // Price can be zero because the ride could be cancelled
    if (entry.price_ != 0)
    {
        // Fill the fee
        entry.fee_ = billing_params::fee_for_order() * entry.price_;

        // Then check if a driver has a promo for a free free
        // Normally there are a few promos per driver so the fullscan is quite fast
        for (auto &p : driver_stat_promos_)
        {
            if (now >= p.ts_start_ and now <= p.ts_finish_ and p.promo_type_ == 1)
            {
                // Yes! Driver has a promo - no fee
                entry.fee_ = 0;
                break;
            }
        }
        
        // Check for a purchased shift - in that case the fee is also zero
        // Note: a purchase works for next 24 hours of its timestamp so we need to
        //  find an active shift
        auto i = std::lower_bound(driver_stat_entry_shift_purchases_.begin(),
                                driver_stat_entry_shift_purchases_.end(), now);
        if (i != driver_stat_entry_shift_purchases_.end() && i->ts_ - now < 24*3600)
        {
            // Found a shift purchase! No fee :-)
            entry.fee_ = 0;
        }
    }
    
    // Add the stat entry
    driver_stat_entry_rides_.push_back(entry);
}

void driver_stat::get_driver_stat_entries(time_t from, time_t to, std::vector<driver_stat_entry_ride> &result)
{
    get_objects_from_to_impl_(from, to, result, driver_stat_entry_rides_);
}

void driver_stat::get_driver_stat_aggr_entries(time_t from, time_t to, std::vector<driver_stat_entry_ride> &result)
{
    result.clear();
    std::vector<driver_stat_entry_ride> temp;
    
    // Get all entries
    get_driver_stat_entries(from, to, temp);
    
    // Agregate them on per week basis
    time_t cur_week_beginning = 0;
    driver_stat_entry_ride entry = {0,0,0,0, 0,0,0,0, 0};
    for (auto &e : temp)
    {
        // Convert a timestamp to the most recent Monday 00:00:00
        time_t week_begining = -3*86400 + (e.ts_order_accepted_ / (86400 * 7)) * 86400 * 7;
        
        // If this starts a new week
        if (week_begining > cur_week_beginning)
        {
            // Save an old week entry
            if (cur_week_beginning)
                result.push_back(entry);
            
            // Start a new week
            cur_week_beginning = week_begining;
            entry = {0,0,0,0, 0,0,0,0, 0};
        }
        
        entry.ts_order_accepted_ = week_begining;
        entry.price_ += e.price_;
        entry.real_distance_to_A_ += e.real_distance_to_A_ + e.real_distance_A_to_B_;
        entry.real_seconds_to_A_ += e.real_seconds_to_A_ + e.real_seconds_A_to_B_;
        if (e.ride_status_ == 1)
            ++entry.real_distance_A_to_B_; // Store here the number of finished trips
        else
            ++entry.real_seconds_A_to_B_; // Store here the number of cancelled trips
        entry.fee_ += e.fee_;
    }
    
    if (cur_week_beginning)
        result.push_back(entry);
}

void driver_stat::get_driver_stat_entry_shift_purchase(time_t from, time_t to,
                                            std::vector<driver_stat_entry_shift_purchase> &result)
{
    get_objects_from_to_impl_(from, to, result, driver_stat_entry_shift_purchases_);
}

void driver_stat::get_driver_stat_payments(time_t from, time_t to, std::vector<driver_stat_payment> &result)
{
    get_objects_from_to_impl_(from, to, result, driver_stat_payments_);
}

void driver_stat::get_driver_stat_withdrawals(time_t from, time_t to, std::vector<driver_stat_withdrawal> &result)
{
    get_objects_from_to_impl_(from, to, result, driver_stat_withdrawals_);
}

void driver_stat::get_driver_stat_cashbacks(time_t from, time_t to, std::vector<driver_stat_cashback> &result)
{
    get_objects_from_to_impl_(from, to, result, driver_stat_cashbacks_);
}
void driver_stat::get_driver_stat_bonuses(time_t from, time_t to, std::vector<driver_stat_bonus> &result)
{
    get_objects_from_to_impl_(from, to, result, driver_stat_bonuses_);
}

void driver_stat::get_driver_stat_promos(time_t from, time_t to, std::vector<driver_stat_promo> &result)
{
    get_objects_from_to_impl_(from, to, result, driver_stat_promos_);
}

driver_stat_balance driver_stat::restore_driver_balance(time_t ts)
{
    // Sum all payments
    driver_stat_balance dsb;
    dsb.payment_amout_ = 0;
    for (auto &i : driver_stat_payments_)
        dsb.payment_amout_ += i.amount_;
    
    // Sum all cashbacks
    dsb.cashbacks_ = 0;
    for (auto &i : driver_stat_cashbacks_)
        dsb.cashbacks_ += i.amount_;
    
    // Sum all bonuses
    dsb.bonuses_ = 0;
    for (auto &i : driver_stat_bonuses_)
        dsb.bonuses_ += i.amount_;
    
    
    // Sum all withholdings. Those are per-ride fees and shift purchases
    dsb.withheld_ = 0;
    for (auto &i : driver_stat_entry_rides_)
        dsb.withheld_ -= i.fee_;
    for (auto &i : driver_stat_entry_shift_purchases_)
        dsb.withheld_ -= i.fee_;
    
    // Sum all withdrawals
    dsb.withdrawals_ = 0;
    for (auto &i : driver_stat_withdrawals_)
        dsb.withdrawals_ -= i.amount_;
    
    return dsb;
}

void driver_stat::purchase_shift(time_t ts)
{
    driver_stat_entry_shift_purchases_.push_back({.ts_ = ts, .fee_ = billing_params::fee_for_shift()});
}

void driver_stat::make_payment(double amount, int agent)
{
    driver_stat_payments_.push_back({.ts_ = time(NULL), .amount_ = amount, .agent_ = agent});
}

double driver_stat::withdraw_money(double amount, int agent)
{
    // Get the balance
    driver_stat_balance bal = restore_driver_balance(time(NULL));
    
    // How much can we withdraw? First of all determine the debt
    // Remember that withheld and withdrawals are negative?
    double balance = bal.payment_amout_ + bal.cashbacks_ + bal.bonuses_ + bal.withheld_ + bal.withdrawals_;
    
    // If there is a debt then we can't withdraw anything
    if (balance <= 0)
        return 0;
    
    // If there is a positive balance then we can only withdraw no more than
    // min(balance, bal.cashbacks_ + bal.withdrawals_)
    // Why? Because only cashbacks are withdrawable, but if a driver owes then
    // cashbacks partially cover the debt and only then are subject to withdraw
    double max_amount_to_withdraw = std::min(balance, bal.cashbacks_ + bal.withdrawals_);
    
    if (amount > max_amount_to_withdraw)
        amount = max_amount_to_withdraw;
    
    driver_stat_withdrawals_.push_back({.ts_ = time(NULL), .amount_ = amount, .agent_ = agent});
    
    return amount;
}

void driver_stat::cashback_app_usage(time_t now)
{
    // Check the ts of the most recent cashback
    time_t recent_ts = driver_stat_cashbacks_.empty() ?
        0 : driver_stat_cashbacks_[driver_stat_cashbacks_.size()-1].ts_;
    
    // If it was more than a day ago then add a new cashback
    if (recent_ts <= now - 24*3600)
        driver_stat_cashbacks_.push_back({.ts_ = now,
            .amount_ = billing_params::cashback_app_usage(),
            .cashback_reason_ = 1 // Cashback for app usage
        });
}

bool driver_stat::bonus_app_install(time_t now)
{
    // This is a one-time bonus
    // Check if it's never been given
    for (auto &i : driver_stat_bonuses_)
        if (i.bonus_type_ == 1)
            return false;
    
    driver_stat_bonuses_.push_back({.ts_ = now,
        .amount_ = billing_params::bonus_app_install(),
        .bonus_type_ = 1 // Bonus for app install
    });
    
    return true;
}

bool driver_stat::promo_app_install(time_t now)
{
    // This is a one-time promo
    // Check if it's never been granted
    for (auto &i : driver_stat_promos_)
        if (i.promo_type_ == 1)
            return false;
    
    driver_stat_promos_.push_back({.ts_ = now,
        .ts_start_ = now,
        .ts_finish_ = now + billing_params::days_no_fee_promo() * 24 * 3600,
        .promo_type_ = 1 // No fee for N months for app install
    });
    
    return true;
}

}
