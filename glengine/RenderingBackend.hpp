#pragma once

#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>

class RenderingBackend {
  public:
   // Thou shalt not instantiate this class. Derive from it.
   virtual ~RenderingBackend() {}


   virtual void init(const std::string& windowTitle, glm::ivec2 windowDims) = 0;
   SDL_Window*  window;
   glm::ivec2   windowDims;
};
