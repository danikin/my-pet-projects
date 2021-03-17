
#ifndef _md5_h_included_
#define _md5_h_included_

#include <cstdint>
#include <string>
#include <climits>

bool md5(const char *message,
         size_t buffer_size,
         size_t size,
         unsigned char *md5);

#endif
