#pragma once
#include <fstream>
#include <vector>

#include "Types.hpp"

#include "Logger.hpp"

// This should really be in the std....
#define typeindex(CLASS) std::type_index(typeid(CLASS))

// Never write another dull getter/setter again! (Hopefully)
#define GETTER(VARNAME, FUNC_NAME) \
   inline TYPE get##FUNC_NAME() { return this->VARNAME; }

#define SETTER(VARNAME, FUNC_NAME) \
   inline void set##FUNC_NAME(decltype(VARNAME) x) { this->VARNAME = x; }

#define REF_SETTER(VARNAME, FUNC_NAME) \
   inline void set##FUNC_NAME(const decltype(VARNAME)& x) { this->VARNAME = x; }

#define REF_GETTER(VARNAME, FUNC_NAME) \
   inline const auto& get##FUNC_NAME() { return this->VARNAME; }

#define GETTER_SETTER(VARNAME, FUNC_NAME) \
   GETTER(TYPE, VARNAME, FUNC_NAME)       \
   SETTER(TYPE, VARNAME, FUNC_NAME)

#define REF_GETTER_SETTER(VARNAME, FUNC_NAME) \
   REF_GETTER(TYPE, VARNAME, FUNC_NAME)       \
   REF_SETTER(TYPE, VARNAME, FUNC_NAME)


#define GENERATE_ENUM_STREAM_OPS(ENUMNAME, ENUMMAPPINGS)                      \
   inline std::ostream& operator<<(std::ostream& out, const ENUMNAME& from) { \
      out << ENUMMAPPINGS.at(from);                                           \
                                                                              \
      return out;                                                             \
   }                                                                          \
                                                                              \
   inline std::istream& operator>>(std::istream& in, ENUMNAME& to) {          \
      std::string buff;                                                       \
      in >> buff;                                                             \
      for (const auto& pairing : ENUMMAPPINGS)                                \
         if (pairing.second == buff)                                          \
            to = pairing.first;                                               \
                                                                              \
                                                                              \
      return in;                                                              \
   }

//==========================================================================
// My own answer to not having rust-style #derive()'s

// Mainly for converting Shader or Pipeline, for example, to vk::ShaderModule and the like
#define CONVERTABLE_TO_MEMBER(MEMBER) \
   inline operator auto&() { return this->MEMBER; }


// Allows you to use pipeline.setFlags instead of pipeline.createInfo.setFlags, for example
#define FORWARD_TO_MEMBER(FUNCNAME, MEMBER)          \
   template <typename... Args>                       \
   inline auto FUNCNAME(const Args&... args) {       \
      return MEMBER.FUNCNAME(std::forward(args...)); \
   }

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
