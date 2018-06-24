#pragma once
#include <fstream>
#include <vector>

#include "Types.hpp"

#include "Logger.hpp"

#define typeindex(CLASS) std::type_index(typeid(CLASS))

#define GETTER(TYPE, VARNAME, FUNC_NAME) \
   inline TYPE get##FUNC_NAME() { return this->VARNAME; }

#define SETTER(TYPE, VARNAME, FUNC_NAME) \
   inline void set##FUNC_NAME(TYPE x) { this->VARNAME = x; }

#define REF_SETTER(TYPE, VARNAME, FUNC_NAME) \
   inline void set##FUNC_NAME(const TYPE& x) { this->VARNAME = x; }

#define REF_GETTER(TYPE, VARNAME, FUNC_NAME) \
   inline const TYPE& get##FUNC_NAME() { return this->VARNAME; }

#define GETTER_SETTER(TYPE, VARNAME, FUNC_NAME) \
   GETTER(TYPE, VARNAME, FUNC_NAME)             \
   SETTER(TYPE, VARNAME, FUNC_NAME)

#define REF_GETTER_SETTER(TYPE, VARNAME, FUNC_NAME) \
   REF_GETTER(TYPE, VARNAME, FUNC_NAME)             \
   REF_SETTER(TYPE, VARNAME, FUNC_NAME)

#define CONVERTABLE_TO_MEMBER(TYPE, MEMTYPE, MEMBER) \
   inline const operator MEMTYPE&() { return this->MEMBER; }

template <typename T>
constexpr bool AllAreNot(const T& target, const T& arg) {
   return (arg != target);
}

template <typename T, typename... Args>
constexpr bool AllAreNot(const T& target, const T& arg, const Args&... args) {
   return (arg != target) & AllAreNot(target, args...);
}

// Todo: Rework Macros/Logger/Types/etc into a separate library for base functions.
inline std::vector<byte> LoadFile(const std::string& fileName) {
   using namespace std;
   // Because for some stupid reason, std::vector<byte> is an error, yet (byte = unsigned char)....
   std::vector<unsigned char> res(0);

   ifstream file(fileName, ios::ate | ios::binary);


   if (!file)
      Logger::Error("Failed to LoadFile ", fileName);

   auto size = file.tellg();

   res.resize(size);
   file.seekg(0);
   file.read(reinterpret_cast<sbyte*>(res.data()), size);
   file.close();

   return res;
}
