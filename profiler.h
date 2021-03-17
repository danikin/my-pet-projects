/*
*	profiler.h
*
*	(C) Denis Anikin 2020
*
*	Headers for the profiler 
*
*/

#ifndef _profiler_h_included_
#define _profiler_h_included_

#include <time.h>
#include <sys/time.h>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <vector>

class profiler
{
private:
    
    struct profiling_point;
    
public:
    
    profiler(int interval_seconds) : interval_seconds_(interval_seconds) {}
    
    struct sentry
    {
    public:
        
        sentry(unsigned long long time_sentry_created,
               profiler::profiling_point *current_profiling_point,
               profiler *p) :
                time_sentry_created_(time_sentry_created),
                current_profiling_point_(current_profiling_point),
                profiler_(p)
        {
        }
        sentry(sentry &&rhs) :
            time_sentry_created_(rhs.time_sentry_created_),
            current_profiling_point_(rhs.current_profiling_point_),
            profiler_(rhs.profiler_)
        {
            rhs.profiler_ = NULL;
        }
        ~sentry()
        {
            if (profiler_)
            {
                --profiler_->cur_level_;
                
                // Goint one parent up
                if (profiler_->cur_parent_)
                {
                    const void *parent_handler = profiler_->cur_parent_->first_parent_handler_;
                    if (!parent_handler)
                        profiler_->cur_parent_ = NULL;
                    else
                        profiler_->cur_parent_ = profiler_->get_profiling_point(parent_handler);

                }
                struct timeval tv;
                gettimeofday(&tv, NULL);
                current_profiling_point_->runtime_ += tv.tv_sec * 1000000 + tv.tv_usec - time_sentry_created_;
            }
        }
        
    private:

        unsigned long long time_sentry_created_;
        
        profiler::profiling_point *current_profiling_point_;
        
        profiler *profiler_;
    };
    friend struct sentry;

    // Prints staistics
    // Usage:
    //  1.  Declare rps_counter in a class as a member
    //  2.  Whenever you need to calc stat do this:
    //          auto _dont_use_me_ = rc_.print_counter(id, "myfunction", 3);
    //      It will print RPS and function runtime from here and to the end of the scope
    //          every interval_seconds
    //      Note: id is from 0 to 99 - to differentiate counters - it's better than by name from
    //          the performance standpoint
    sentry profile(const void *handler)
    {
        profiling_point *pp = get_profiling_point(handler);

        struct timeval tv;
        gettimeofday(&tv, NULL);
        unsigned long long current_time = tv.tv_sec * 1000000 + tv.tv_usec;
        
        if (!time_stat_printed_)
            time_stat_printed_ = current_time;
        else
        {
            if (current_time - time_stat_printed_ >= interval_seconds_ * 1000000)
            {
               // std::cerr << "profiler::profile: current_time=" << current_time
               // << ", time_stat_printed_=" << time_stat_printed_ << std::endl;
                is_time_to_print_stat_ = true;
            }
        }

        pp->level_ = cur_level_;
        if (!pp->first_enter_time_)
            pp->first_enter_time_ = current_time;
        if (!pp->first_parent_handler_)
        {
            pp->first_parent_handler_ = cur_parent_ ? cur_parent_->handler_ : NULL;
            //std::cerr << "pp=" << (const char*)pp->handler_ <<
              //  ", pp->first_parent_=" << (pp->first_parent_?(const char*)pp->first_parent_->handler_:"NULL")
                //<< std::endl;
        }
        ++pp->counter_;
        
        // This variable will be decremented when the sentry will leave its scope
        ++cur_level_;
        cur_parent_ = pp;
        
        return sentry(current_time, pp, this);
    }
    
    void print_stat();

private:
    
    int interval_seconds_;

    struct profiling_point
    {
        unsigned long long counter_ = 0;
        unsigned long long runtime_ = 0;
        int level_ = 0;
        unsigned long long first_enter_time_ = 0;
        
        // We need a handler here, not a pointer to profiling_point because of the sorting
        //   in print_stat
        const void *first_parent_handler_ = NULL;
        const void *handler_ = NULL;
        
        // That's only for print_stat at the sorting time!
        profiling_point *temp_first_parent_ = NULL;
        
        // Sum runtimes of all children
        unsigned long long temp_children_runtime_ = 0;
    };
    
    const int hash_table_size_ = sizeof(pseudo_hash_table_)/sizeof(profiling_point);
    
    // const void * -> profiling_point
    profiling_point pseudo_hash_table_[1003] = {};
    
    profiling_point *get_profiling_point(const void *handler)
    {
        // "Hash" the handler :-)
        int i = ((unsigned long long)handler) % hash_table_size_;
        
        // Check for a conflict
        // Note: the best way to avoid conflicts is to make sure that hash_table_size_ is much
        //  bigger than the maximum amount of profiling points
        if (pseudo_hash_table_[i].handler_ && pseudo_hash_table_[i].handler_ != handler)
        {
            std::cerr << "profiler::get_profiling_point: CONFLICT!!! handler=" << (unsigned long long)handler
            << ", i=" << i << ", pseudo_hash_table_[i].handler_=" << (unsigned long long)pseudo_hash_table_[i].handler_
            << ", handler(str)=" << (const char*)handler << ", pseudo_hash_table_[i].handler_(str)="
            << (const char*)pseudo_hash_table_[i].handler_
            << std::endl;
        }
        else
            pseudo_hash_table_[i].handler_ = handler;

        return pseudo_hash_table_ + i;
    }
    
    profiling_point *get_first_parent(const profiling_point *pp)
    {
        const void *parent_handler = pp->first_parent_handler_;
        if (!parent_handler)
            return NULL;
        else
            return get_profiling_point(parent_handler);
    }
    
    unsigned long long time_stat_printed_ = 0;
    int cur_level_ = 0;
    profiling_point *cur_parent_ = NULL;
    bool is_time_to_print_stat_ = false;
};

#endif
