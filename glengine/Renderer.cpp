#include "Renderer.hpp"

Renderer::Renderer(const std::__cxx11::string& windowTitle, glm::ivec2 windowDims) {
   if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
      Logger::ErrorOut("Failed to init SDL: ", SDL_GetError());

   window =
       SDL_CreateWindow(windowTitle.c_str(), 0, 0, windowDims.x, windowDims.y, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
   if (!window)
      Logger::ErrorOut("Failed to create a window: ", SDL_GetError());

   // TODO: Decompose the blocks into their own functions. They should already be seperable, but keeping them this way
   // for now for faster dev.

   {  // Get the instance
      auto appInfo = vk::ApplicationInfo()
                         .setApplicationVersion(1)
                         .setEngineVersion(1)
                         .setPEngineName("GLENgine")
                         .setApiVersion(VK_API_VERSION_1_1);

      uint32_t count = 0;
      SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
      std::vector<const char*> extensions(count);
      SDL_Vulkan_GetInstanceExtensions(window, &count, &extensions[0]);

      auto createInfo = vk::InstanceCreateInfo()
                            .setPApplicationInfo(&appInfo)
                            .setEnabledExtensionCount(count)
                            .setPpEnabledExtensionNames(&extensions[0]);

      if (!(vulkInstance = vk::createInstance(createInfo)))
         Logger::ErrorOut("Failed to create Vulkan instance!");
   }

   {  // Get the surface
      VkSurfaceKHR surface;
      if (!SDL_Vulkan_CreateSurface(window, vulkInstance, &surface))
         Logger::ErrorOut("Failed to create vulkan surface!");

      vulkSurface = vk::SurfaceKHR(surface);
   }

   {  // Get physical device. We'll just take the first available for now.
      uint32_t count;
      if (vulkInstance.enumeratePhysicalDevices(&count, nullptr) != vk::Result::eSuccess)
         Logger::ErrorOut("Failed to enumerate physical devices!");

      std::vector<vk::PhysicalDevice> physDevices(count);
      vulkInstance.enumeratePhysicalDevices(&count, &physDevices[0]);

      vulkPhys = physDevices[0];
   }
   std::vector<const char*> deviceExtensions;
   {
      auto                     extensions = vulkPhys.enumerateDeviceExtensionProperties();
      std::vector<const char*> desired    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DEBUG_MARKER_EXTENSION_NAME};


      for (auto& ext : extensions) {
         for (auto& desiredExt : desired)
            if (std::string(ext.extensionName) == desiredExt) {
               deviceExtensions.push_back(ext.extensionName);
               break;
            }
      }
   }
   std::vector<const char*> deviceLayers;
   {
      auto                     layers        = vulkPhys.enumerateDeviceLayerProperties();
      std::vector<const char*> desiredLayers = {"VK_LAYER_LUNARG_standard_validation"};

      for (auto& layer : layers) {
         for (auto& desiredLayer : desiredLayers)
            if (std::string(layer.layerName) == desiredLayer) {
               deviceLayers.push_back(layer.layerName);
               break;
            }
      }
   }


   Logger::Write("INFO", "Initialized Renderer!");
}

Renderer::~Renderer() {
   SDL_DestroyWindow(window);
   vulkInstance.destroy();

   SDL_Quit();
}
