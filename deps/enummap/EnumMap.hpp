#pragma once
#include <string>

// Use this to mark an enum for mapping
#define STRINGIFY __attribute__((annotate("Stringify")))

// This is the function you call to convert from a string to your enum. It returns T(UINT_MAX) if it fails 
template <typename T>
T from_string(const std::string& in);

// For enum->string, we use a specialization of to_string, so just call that.
