#pragma once
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <typeindex>
#include <unordered_map>

#include <SDL2/SDL.h>

#include "RenderingBackend.hpp"
#include "VulkanBackend.hpp"

#include "Input.hpp"

#include "Logger.hpp"

// Note: Consider pulling in transwarp for overall library directing.
// // Maybe just in the client app?
