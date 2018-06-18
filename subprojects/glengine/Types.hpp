#pragma once
#include <stdint.h>
#define NO_T(TYPE) using TYPE = TYPE##_t;

NO_T(uint32);
NO_T(uint64);
NO_T(int32);
NO_T(int64);

using uint = uint64;

using byte = unsigned char;
