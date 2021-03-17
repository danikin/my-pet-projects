/*
*    debug.h
*
*    (C) Denis Anikin 2020
*
*    Headers for debug print for the taxi service
*
*/

#ifndef _debug_h_included_
#define _debug_h_included_

#include <ctime>
#include <iostream>
#include <iomanip>

namespace debug
{

class debug
{
public:

    friend std::ostream &operator<<(std::ostream &os, const debug &d)
    {
        std::time_t t = std::time(nullptr);
        return os << "[" << std::put_time(std::localtime(&t), "%c %Z") << "] ";
    }

};


} // namespace debug

#endif
