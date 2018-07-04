#pragma once
#include <functional>
#include <memory>
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

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <typename T>
using UniquePtr = std::unique_ptr<T>;

// Yes {} and {val} are simple, but they're not quite so readable. Let's go rust style for this.
template <typename T>
inline std::optional<T> None() {
   return {};
}

template <typename T>
inline std::optional<T> Some(const T& val) {
   return {val};
}


// Enforcing that all vectors have the same size.
// Useful for things like subpass deps + subpass descs

template <typename T, typename... TYPES>
struct NVector_Storage {
   NVector_Storage(size_t initial) : vec(initial), storage(initial) {}

   size_t size() { return vec.size(); }
   void   push(const T& t, const TYPES&... args) {
      vec.push_back(t);
      storage.push(args...);
   }

   std::tuple<T, TYPES...> pop() {
      auto el = vec.at(vec.back());
      vec.pop_back();
      return {el, storage.pop()};
   }

   std::tuple<T*, TYPES*...> data() { return {vec.data(), storage.data()}; }

   std::vector<T>            vec;
   NVector_Storage<TYPES...> storage;
};

template <typename T>
struct NVector_Storage<T> {
   NVector_Storage(size_t initial) : vec(initial) {}

   size_t size() { return vec.size(); }
   void   push(const T& t) { vec.push_back(t); }
   T      pop() {
      auto el = vec.at(vec.back());
      vec.pop_back();
      return el;
   }
   T* data() { return vec.data(); }

   std::vector<T> vec;
};

template <typename... TYPES>
struct NVector {
  public:
   NVector(size_t initial = 0) : storage(0) {}

   void                  push(const TYPES&... args) { storage.push(args...); }
   std::tuple<TYPES...>  pop() { return storage.pop(); }
   std::tuple<TYPES*...> data() { return storage.data(); }
   size_t                size() { return storage.size(); }  // The size of 1 is the same as the size of all


   NVector_Storage<TYPES...> storage;
};
