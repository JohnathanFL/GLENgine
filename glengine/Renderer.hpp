// A note to the poor soul that wants to read my code: I'm still in the process of learning Vulkan itself, and 90% of
// this code will likely be completely changed/rearanged by the time I move on to another subsystem.
// Even so, I apoligize for this...thing I've created.

#pragma once


#include <vulkan/vk_layer.h>
#include <vulkan/vulkan.h>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <SDL2/SDL.h>


#include "Logger.hpp"
#include "Types.hpp"

#include "Macros.hpp"

#include "VulkanBackend.hpp"


#include "Shader.hpp"

// Current plan: Other than sprites and the like, everything will be treated as a Voxel volume, using either compute
// shaders, geometry shaders, or some combination of the two to make the meshes
// (Could use geom shaders to make a temp mesh, then swap to a compute-generated mesh when ready)


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
