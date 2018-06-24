#include "Renderer.hpp"

using namespace std;

Renderer::Renderer(const string& windowTitle, glm::ivec2 windowDims) {
   if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
      Logger::ErrorOut("Failed to init SDL: ", SDL_GetError());

   // TODO: Decompose the blocks into their own functions. They should already be seperable, but keeping them this way
   // for now for faster dev.


   // See VulkanRendererBoilerplate.cpp
   vulkan.init(windowTitle, windowDims);

   Logger::Info("Initialized Renderer!");
}


Renderer::~Renderer() { SDL_Quit(); }
