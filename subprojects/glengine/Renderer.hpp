// A note to the poor soul that wants to read my code: I'm still in the process of learning Vulkan itself, and 90% of
// this code will likely be completely changed/rearanged by the time I move on to another subsystem.
// Even so, I apoligize for this...thing I've created.

#pragma once


#include <vulkan/vk_layer.h>
#include <vulkan/vulkan.h>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>


#include "Logger.hpp"
#include "Types.hpp"

#include "Macros.hpp"

#include "Shader.hpp"

// Basic idea currently looks like this: A variable number of command buffer pairs. While 1 element is written to in
// another thread, the other is being drawn. Ideas for separate pairs: 1 pair for voxel terrain, 1 pair for entities, 1
// pair for completely static geometry, etc
// Possibly reserve cmdBuffs[0..1] for default static/dynamic geometry stuff (normal addGeom or other such func, rather
// than an addCustomGeom)

struct QueueIndices {
   int graphics = -1;
   int present  = -1;

   bool isComplete() { return AllAreNot(-1, graphics, present); }
};

struct SwapchainSupportInfo {
   vk::SurfaceCapabilitiesKHR        caps;
   std::vector<vk::SurfaceFormatKHR> formats;
   std::vector<vk::PresentModeKHR>   modes;
};

struct SwapchainInfo {
   vk::SurfaceFormatKHR format;
   vk::Extent2D         res;
};

struct VulkanBoilerplate {
   std::vector<const char*>   deviceExtensions, deviceLayers;
   vk::PhysicalDevice         physical;
   vk::UniqueDevice           logical;
   vk::Queue                  graphicsQueue, presentQueue;
   vk::UniqueSurfaceKHR       surface;
   vk::UniqueInstance         instance;
   vk::DebugReportCallbackEXT debugCallback;

   vk::SwapchainKHR           swapchain;
   SwapchainInfo              swapInfo;
   std::vector<vk::Image>     swapImages;
   std::vector<vk::ImageView> swapViews;

   vk::PipelineLayout pipeLayout;

   QueueIndices queueIndices;
};


class Renderer {
  public:
   Renderer(const std::string& windowTitle, glm::ivec2 windowDims);
   ~Renderer();


   void updateRender() {}

   GETTER_SETTER(SDL_Window*, window, Window)
   REF_GETTER_SETTER(glm::vec4, clearColor, ClearColor)
  private:
   // All the vulkan stuff. Note that many of these are defined in VulkanRendererBoilerplate.cpp instead
   // (Keeps Renderer.cpp readable)
   void setupDebugCallback();
   void initVulkan();
   void createInstance();
   void createSurface();
   void getPhysical();
   void getLogical();
   void createSwapchain();
   void createGraphicsPipeline();

   void getExtensions();
   void getLayers();

   // Perhaps exchange the references with a single (const) reference to a VulkanBoilerplate?
   static QueueIndices         getQueueFamilyIndices(const vk::PhysicalDevice& physical, const vk::SurfaceKHR& surface);
   static bool                 isDeviceSuitable(const vk::PhysicalDevice& dev, const vk::SurfaceKHR& surface);
   static vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
   static vk::PresentModeKHR   choosePresentMode(const std::vector<vk::PresentModeKHR>& modes);
   static vk::Extent2D         chooseResolution(const vk::SurfaceCapabilitiesKHR& caps, const glm::uvec2& curRes);

   static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                                                       uint64_t obj, size_t location, int32_t code,
                                                       const char* layerPrefix, const char* msg, void* userData);

   // Todo: once this is all settled, move this instance into its own full class that inherits from a common rendering
   // backend interface.
   // Todo-2: Add all vulkan boilerplate functions to their own backend class. This engine will be made primarily for
   // Vulkan, but having an OpenGL backend would be a nice fallback too.
   VulkanBoilerplate vulkan;

   std::vector<std::string>                                                 cmdBuffNames;
   std::vector<std::pair<vk::UniqueCommandBuffer, vk::UniqueCommandBuffer>> cmdBuffs;

   SDL_Window* window;

   glm::uvec2 windowDims;
   glm::vec4  clearColor;

   // Temp stuff for following the vulkan-tutorial
   Shader vert, frag;
};
