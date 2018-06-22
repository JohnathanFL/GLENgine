#pragma once
#include <stdint.h>
#define NO_T(TYPE) using TYPE = TYPE##_t;
#define STRIP_WITH_U_T(TYPE) \
   NO_T(TYPE);               \
   NO_T(u##TYPE);

// uint8, int64, etc
STRIP_WITH_U_T(int8);
STRIP_WITH_U_T(int16);
STRIP_WITH_U_T(int32);
STRIP_WITH_U_T(int64);


using sbyte = char;  // <S>igned byte
using ubyte = unsigned char;
using byte  = unsigned char;  // Default to a byte being unsigned. (Makes the most sense to me).
