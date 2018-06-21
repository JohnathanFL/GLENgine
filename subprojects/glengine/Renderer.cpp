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
   vulkan.logical->destroyPipelineLayout(vulkan.pipeLayout);

   vulkan.logical->destroyShaderModule(vert);
   vulkan.logical->destroyShaderModule(frag);

   vulkan.logical->waitIdle();
   vkDestroySwapchainKHR(*vulkan.logical, vulkan.swapchain, nullptr);
   vulkan.logical->destroy();

   vulkan.instance->destroy();
   SDL_DestroyWindow(window);
   SDL_Quit();
}
