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

// Current plan: Other than sprites and the like, everything will be treated as a Voxel volume, using either compute
// shaders, geometry shaders, or some combination of the two to make the meshes
// (Could use geom shaders to make a temp mesh, then swap to a compute-generated mesh when ready)

class RenderingBackend {
  public:
   virtual ~RenderingBackend() {}
   virtual void init(const std::string& windowTitle, glm::ivec2 windowDims) = 0;
   SDL_Window*  window;
   glm::ivec2   windowDims;
};

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

class VulkanBackend : RenderingBackend {
  public:
   virtual ~VulkanBackend();

   virtual void init(const std::string& windowTitle, glm::ivec2 windowDims);

   // Todo: Better encapsulation
   // private:
   void setupDebugCallback();
   void createInstance();
   void createSurface();
   void getPhysical();
   void getLogical();
   void createSwapchain();
   void createRenderPasses();
   void createGraphicsPipeline();
   void createFrameBuffers();
   void createCommandPools();
   void createCommandBuffs();
   void createSemaphores();

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


   // POD
   const size_t   maxFramesInFlight = 2;
   size_t         currentFrame      = 0;
   vk::ClearValue clearColor;

   // Constructor-ordered
   std::vector<const char*> deviceExtensions, deviceLayers;
   vk::PhysicalDevice       physical;
   vk::UniqueInstance       instance;
   vk::UniqueDevice         logical;
   vk::UniqueSurfaceKHR     surface;
   vk::Queue                graphicsQueue, presentQueue;


   vk::DebugReportCallbackEXT debugCallbackObj;

   vk::SwapchainKHR           swapchain;
   SwapchainInfo              swapInfo;
   std::vector<vk::Image>     swapImages;
   std::vector<vk::ImageView> swapViews;

   vk::RenderPass     renderPass;
   vk::Pipeline       pipeline;
   vk::PipelineLayout pipeLayout;

   std::vector<vk::UniqueFramebuffer> swapFramebuffers;

   vk::UniqueCommandPool commandPool;

   QueueIndices queueIndices;

   std::vector<vk::CommandBuffer> cmdBuffs;

   std::vector<vk::UniqueSemaphore> imageAvailSems, renderFinishedSems;

   // Temp stuff for following the vulkan-tutorial
   Shader vert, frag;
};


class Renderer {
  public:
   Renderer(const std::string& windowTitle, glm::ivec2 windowDims);
   ~Renderer();


   void updateRender() {
      // Todo: All of this should be re-encapsulated into a vulkan backend object.
      uint32 imageIndex = vulkan.logical
                              ->acquireNextImageKHR(vulkan.swapchain, std::numeric_limits<uint32>::max(),
                                                    *vulkan.imageAvailSems[vulkan.currentFrame], vk::Fence(nullptr))
                              .value;
      vk::Semaphore          waitSemaphores[] = {*vulkan.imageAvailSems[vulkan.currentFrame]};
      vk::PipelineStageFlags waitStages[]     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

      vk::Semaphore signalSemaphores[] = {*vulkan.renderFinishedSems[vulkan.currentFrame]};


      auto subInfo = vk::SubmitInfo()
                         .setWaitSemaphoreCount(1)
                         .setPWaitSemaphores(waitSemaphores)
                         .setPWaitDstStageMask(waitStages)
                         .setCommandBufferCount(1)
                         .setPCommandBuffers(&vulkan.cmdBuffs[imageIndex])
                         .setSignalSemaphoreCount(1)
                         .setPSignalSemaphores(signalSemaphores);

      vulkan.graphicsQueue.submit(subInfo, vk::Fence(nullptr));


      vk::SwapchainKHR swapchains[] = {vulkan.swapchain};

      auto presentInfo = vk::PresentInfoKHR()
                             .setWaitSemaphoreCount(1)
                             .setPWaitSemaphores(signalSemaphores)
                             .setSwapchainCount(1)
                             .setPSwapchains(swapchains)
                             .setPImageIndices(&imageIndex)
                             .setPResults(nullptr);
      vulkan.presentQueue.presentKHR(presentInfo);

      vulkan.currentFrame = (vulkan.currentFrame + 1) % vulkan.maxFramesInFlight;
   }

  private:
   // Todo: once this is all settled, move this instance into its own full class that inherits from a common rendering
   // backend interface.
   // Todo-2: Add all vulkan boilerplate functions to their own backend class. This engine will be made primarily for
   // Vulkan, but having an OpenGL backend would be a nice fallback too.
   VulkanBackend vulkan;
};
