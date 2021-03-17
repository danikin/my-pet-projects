/*
*    geo.h
*
*    (C) Denis Anikin 2020
*
*    Headers for geo services
*
*/

#ifndef _geo_h_included_
#define _geo_h_included_

#include "geo_core.h"

namespace geo
{

// These functions should be called at start once to fill the database in RAM
void upload_streets_from_json(addresses &addr, const std::string &filename);
void upload_poi_from_json(addresses &addr, const std::string &filename);
void upload_houses_from_json(addresses &addr, const std::string &filename);

// street_id, house_id - ids to start from
void upload_everything(addresses &addr, const std::string &filename,
	long long street_id, long long house_id);

// A new way to upload and store things compactly
void upload_everything2(compact_addresses &addr);

}

#endif

