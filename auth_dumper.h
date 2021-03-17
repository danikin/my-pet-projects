/*
*    auth_dumper.h
*
*    (C) Denis Anikin 2020
*
*    Dumper for all auth data
*
*/

#ifndef _auth_dumper_h_included_
#define _auth_dumper_h_included_

#include "nlohmann/json.hpp"

#include "auth.h"

namespace dumper
{

using json = nlohmann::json;

}

#endif
