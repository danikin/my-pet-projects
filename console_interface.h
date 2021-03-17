/*
*    console_interace.h
*
*    (C) Denis Anikin 2020
*
*    Headers for the console interface of the taxi service
*
*/

#ifndef _console_interface_h_included_
#define _console_interface_h_included_

#include "marketplace.h"

namespace console_interface
{

// Takes marketplaces from standard input in a form of json, rebalances them and outputs some debug info
void debug_marketplaces();

}

#endif

