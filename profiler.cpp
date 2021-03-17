/*
*	profiler.h
*
*	(C) Denis Anikin 2020
*
*	Headers for the profiler 
*
*/

#include "profiler.h"

void profiler::print_stat()
{
    if (!is_time_to_print_stat_)
        return;
    
    std::cerr << std::endl;

    // Form a vector of all parents
    std::vector<profiling_point*> unique_parents_;
    for (int i = 0; i < hash_table_size_; ++i)
    {
        profiling_point *pp = pseudo_hash_table_ + i;
        pp->temp_first_parent_ = get_first_parent(pp);
        if (pp->temp_first_parent_)
            unique_parents_.push_back(pp->temp_first_parent_);
    }
    
    // For each unique parent store the total runtime of its children
    std::sort(unique_parents_.begin(), unique_parents_.end(), [](const profiling_point *a, const profiling_point *b){
        return a->handler_ < b->handler_;
    });
    unique_parents_.erase(std::unique(unique_parents_.begin(), unique_parents_.end()), unique_parents_.end());
    for (auto parent : unique_parents_)
    {
        parent->temp_children_runtime_ = 0;
        for (profiling_point *pp = pseudo_hash_table_; pp < pseudo_hash_table_ + hash_table_size_; ++pp)
            if (pp->temp_first_parent_ == parent)
                parent->temp_children_runtime_ += pp->runtime_;
    }
    
    // get_first_parent would not work properly inside the sorting because it
    //  leverages hash table, which is not the one during the sorting :-)
    // Also we need a copy of an array to sort to still use parents while sorting
    std::vector<profiling_point> ht_copy;
    for (int i = 0; i < hash_table_size_; ++i)
    {
        profiling_point *pp = pseudo_hash_table_ + i;
        pp->temp_first_parent_ = get_first_parent(pp);
        if (pp->handler_)
            ht_copy.push_back(*pp);
    }
    
    // Sort the vector
    std::sort(ht_copy.begin(), ht_copy.end(), [this]
                (const profiling_point &lhs, const profiling_point &rhs)
    {
        int level_l = lhs.level_;
        int level_r = rhs.level_;
        
        const profiling_point *l = &lhs, *r = &rhs;

        if (!level_l && !level_r)
            return l->first_enter_time_ < r->first_enter_time_;
        
        while (true)
        {
            profiling_point *p_l = l->temp_first_parent_;
            profiling_point *p_r = r->temp_first_parent_;

            // Going up to the same level
            if (l->level_ > r->level_)
                l = p_l;
            else
            if (r->level_ > l->level_)
                r = p_r;
            else
            {
                // On the same level and with a common parent
                if (p_l == p_r)
                {
                    // "l" and "r" can be from different arrays, that's why we compare it by handlers
                    if (l->handler_ == r->handler_)
                        return level_l < level_r;
                    else
                    if (l->first_enter_time_ != r->first_enter_time_)
                        return l->first_enter_time_ < r->first_enter_time_;
                    else
                        return l->handler_ < r->handler_;
                }
                // On the same level and with still a different parents
                else
                {
                    // Going up until find a common parent
                    l = p_l;
                    r = p_r;
                }
            }
        }
    });
    
    for (auto &pp : ht_copy)
    {
        const char *handler = (const char*)pp.handler_;
        if (handler)
        {
            for (int k = 0; k < pp.level_; ++k)
                std::cerr << " ";
            std::cerr << (pp.temp_first_parent_?100*pp.runtime_/pp.temp_first_parent_->runtime_:100) << "% ";
            std::cerr << handler << ": RPS=" << std::setprecision(5)
            << pp.counter_ * 1.0 / interval_seconds_ << std::setprecision(5) <<
            ", rt=" << pp.runtime_ / 1000000.0 << "s"  << ", cnt=" << pp.counter_;
            if (pp.temp_children_runtime_)
                std::cerr << " >" << (100-100*pp.temp_children_runtime_/pp.runtime_) << "%";
            std::cerr << ", ^" << (pp.first_parent_handler_ ? (const char*)pp.first_parent_handler_ : "NULL")
            << std::endl;
        }
    }

    for (int i = 0; i < hash_table_size_; ++i)
    {
        profiling_point *pp = pseudo_hash_table_ + i;
        
        // Reset the pseudo hash table entry
        pp->counter_ = 0;
        pp->runtime_ = 0;
        pp->level_ = 0;
        pp->first_enter_time_ = 0;
        pp->handler_ = NULL;
        pp->first_parent_handler_ = NULL;
        pp->temp_children_runtime_ = 0;
    }
    
    is_time_to_print_stat_ = false;
    time_stat_printed_ = 0;
    cur_parent_= NULL;
    cur_level_ = 0;
}

