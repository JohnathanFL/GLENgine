#pragma once
#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>

#include "RenderingBackend.hpp"

#include "Macros.hpp"

#include "Shader.hpp"

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

class VulkanBackend : public RenderingBackend {
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

   virtual void updateRender() {
      // Todo: All of this should be re-encapsulated into a vulkan backend object.
      uint32 imageIndex = logical
                              ->acquireNextImageKHR(swapchain, std::numeric_limits<uint32>::max(),
                                                    *imageAvailSems[currentFrame], vk::Fence(nullptr))
                              .value;
      vk::Semaphore          waitSemaphores[] = {*imageAvailSems[currentFrame]};
      vk::PipelineStageFlags waitStages[]     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

      vk::Semaphore signalSemaphores[] = {*renderFinishedSems[currentFrame]};


      auto subInfo = vk::SubmitInfo()
                         .setWaitSemaphoreCount(1)
                         .setPWaitSemaphores(waitSemaphores)
                         .setPWaitDstStageMask(waitStages)
                         .setCommandBufferCount(1)
                         .setPCommandBuffers(&cmdBuffs[imageIndex])
                         .setSignalSemaphoreCount(1)
                         .setPSignalSemaphores(signalSemaphores);

      graphicsQueue.submit(subInfo, vk::Fence(nullptr));


      vk::SwapchainKHR swapchains[] = {swapchain};

      auto presentInfo = vk::PresentInfoKHR()
                             .setWaitSemaphoreCount(1)
                             .setPWaitSemaphores(signalSemaphores)
                             .setSwapchainCount(1)
                             .setPSwapchains(swapchains)
                             .setPImageIndices(&imageIndex)
                             .setPResults(nullptr);
      presentQueue.presentKHR(presentInfo);

      currentFrame = (currentFrame + 1) % maxFramesInFlight;
   }

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


   std::vector<vk::UniqueFramebuffer> swapFramebuffers;

   vk::UniqueCommandPool commandPool;

   QueueIndices queueIndices;

   std::vector<vk::CommandBuffer> cmdBuffs;

   std::vector<vk::UniqueSemaphore> imageAvailSems, renderFinishedSems;

   // Temp stuff for following the vulkan-tutorial
   VulkanShader vert, frag;
};
