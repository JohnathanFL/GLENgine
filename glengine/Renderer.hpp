#pragma once


#include <vulkan/vulkan.h>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>


#include "Logger.hpp"

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

class Renderer {
  public:
   Renderer(const std::string& windowTitle, glm::ivec2 windowDims);
   ~Renderer();


   void updateRender() {}

   GETTER_SETTER(SDL_Window*, window, Window)
   REF_GETTER_SETTER(glm::vec4, clearColor, ClearColor)
  private:
   vk::PhysicalDevice vulkPhys;
   vk::Queue          vulkQueue;
   vk::SurfaceKHR     vulkSurface;
   vk::Instance       vulkInstance;
   SDL_Window*        window;

   glm::vec4 clearColor;
};
