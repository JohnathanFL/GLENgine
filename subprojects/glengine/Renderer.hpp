#pragma once


#include <vulkan/vk_layer.h>
#include <vulkan/vulkan.h>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>


#include "Logger.hpp"

#include "Macros.hpp"

// Basic idea currently looks like this: A variable number of command buffer pairs. While 1 element is written to in
// another thread, the other is being drawn. Ideas for separate pairs: 1 pair for voxel terrain, 1 pair for entities, 1
// pair for completely static geometry, etc
// Possibly reserve cmdBuffs[0..1] for default static/dynamic geometry stuff (normal addGeom or other such func, rather
// than an addCustomGeom)


struct VulkanObjects {
   std::vector<const char*>   deviceExtensions;
   std::vector<const char*>   deviceLayers;
   vk::PhysicalDevice         physical;
   vk::UniqueDevice           logical;
   vk::Queue                  graphicsQueue;
   vk::UniqueSurfaceKHR       surface;
   vk::UniqueInstance         instance;
   vk::DebugReportCallbackEXT debugCallback;

   struct QueueIndices {
      int graphics = -1;
   } queueIndices;
};

class Renderer {
  public:
   Renderer(const std::string& windowTitle, glm::ivec2 windowDims);
   ~Renderer();


   void updateRender() {}

   GETTER_SETTER(SDL_Window*, window, Window)
   REF_GETTER_SETTER(glm::vec4, clearColor, ClearColor)
  private:
   // All the vulkan stuff
   void setupDebugCallback();
   void initVulkan();
   void createInstance();
   void createSurface();
   void getPhysical();
   void getLogical();
   void getExtensions();
   void getLayers();

   static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                                                       uint64_t obj, size_t location, int32_t code,
                                                       const char* layerPrefix, const char* msg, void* userData);

   VulkanObjects vulkan;

   std::vector<std::string>                                                 cmdBuffNames;
   std::vector<std::pair<vk::UniqueCommandBuffer, vk::UniqueCommandBuffer>> cmdBuffs;

   SDL_Window* window;

   glm::ivec2 windowDims;
   glm::vec4  clearColor;
};
