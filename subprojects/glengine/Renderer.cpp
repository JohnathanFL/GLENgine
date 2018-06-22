#include "Renderer.hpp"

using namespace std;

Renderer::Renderer(const string& windowTitle, glm::ivec2 windowDims) {
   if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
      Logger::ErrorOut("Failed to init SDL: ", SDL_GetError());

   window =
       SDL_CreateWindow(windowTitle.c_str(), 0, 0, windowDims.x, windowDims.y, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
   if (!window)
      Logger::ErrorOut("Failed to create a window: ", SDL_GetError());

   // TODO: Decompose the blocks into their own functions. They should already be seperable, but keeping them this way
   // for now for faster dev.


   // See VulkanRendererBoilerplate.cpp
   initVulkan();

   Logger::Info("Initialized Renderer!");
}


Renderer::~Renderer() {
   const auto& dev = vulkan.logical;

   dev->destroyPipeline(vulkan.pipeline);
   dev->destroyPipelineLayout(vulkan.pipeLayout);
   dev->destroyRenderPass(vulkan.renderPass);

   dev->destroyShaderModule(vert);
   dev->destroyShaderModule(frag);

   dev->waitIdle();
   vkDestroySwapchainKHR(*dev, vulkan.swapchain, nullptr);
   dev->destroy();

   vulkan.instance->destroy();
   SDL_DestroyWindow(window);
   SDL_Quit();
}
