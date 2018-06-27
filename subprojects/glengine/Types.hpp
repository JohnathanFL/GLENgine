#pragma once
#include <functional>
#include <unordered_map>

#include <stdint.h>
#define STRIP_T(TYPE)        \
   using TYPE    = TYPE##_t; \
   using u##TYPE = u##TYPE##_t;

// uint8, int64, etc
STRIP_T(int8)
STRIP_T(int16)
STRIP_T(int32)
STRIP_T(int64)


using sbyte = char;  // <S>igned byte
using ubyte = unsigned char;
using byte  = unsigned char;  // Default to a byte being unsigned. (Makes the most sense to me).

template <typename T>
using NamedMap = std::unordered_map<std::string, T>;

template <typename T>
using MappedToString = std::unordered_map<T, std::string>;
