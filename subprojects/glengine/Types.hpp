#pragma once
#include <stdint.h>
#define NO_T(TYPE) using TYPE = TYPE##_t;

NO_T(uint32);
NO_T(uint64);
NO_T(int32);
NO_T(int64);

using sbyte = char;  // <S>igned byte
using ubyte = unsigned char;
using byte  = ubyte;  // Default to a byte being unsigned. (Makes the most sense to me).
