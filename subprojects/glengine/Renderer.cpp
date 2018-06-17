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


   initVulkan();

   Logger::Write("INFO", "Initialized Renderer!");
}

void Renderer::initVulkan() {
   createInstance();
   createSurface();
   getPhysical();
   getExtensions();
   getLayers();
}

void Renderer::createInstance() {
   auto appInfo = vk::ApplicationInfo()
                      .setApplicationVersion(1)
                      .setEngineVersion(1)
                      .setPEngineName("GLENgine")
                      .setApiVersion(VK_API_VERSION_1_1);

   uint32_t count = 0;
   SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
   vector<const char*> extensions(count);
   SDL_Vulkan_GetInstanceExtensions(window, &count, &extensions[0]);
   extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
   auto createInfo = vk::InstanceCreateInfo()
                         .setPApplicationInfo(&appInfo)
                         .setEnabledExtensionCount(extensions.size())
                         .setPpEnabledExtensionNames(&extensions[0]);

   if (!(vulkInstance = vk::createInstanceUnique(createInfo)))
      Logger::ErrorOut("Failed to create Vulkan instance!");
   else
      Logger::Info("Created Vulkan instance!");

   auto debugInfo =
       vk::DebugReportCallbackCreateInfoEXT()
           .setFlags(vk::DebugReportFlagBitsEXT(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT |
                                                VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT))
           .setPfnCallback(debugCallback);

   VkDebugReportCallbackCreateInfoEXT info = debugInfo;
   VkDebugReportCallbackEXT           callBack;
   // Dear lord why isn't this in the main vulkan lib?
   auto func = vulkInstance->getProcAddr("vkCreateDebugReportCallbackEXT");
   if (func) {
      reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(func)(vulkInstance.get(), &info, nullptr, &callBack);
      vkCallback = callBack;
      Logger::Info("Debug reporting enabled!");
   }
}

void Renderer::createSurface() {
   VkSurfaceKHR surface;
   if (!SDL_Vulkan_CreateSurface(window, vulkInstance.get(), &surface))
      Logger::ErrorOut("Failed to create vulkan surface!");

   vulkSurface = vk::UniqueSurfaceKHR(surface);
}

void Renderer::getPhysical() {
   uint32_t count;
   if (vulkInstance.get().enumeratePhysicalDevices(&count, nullptr) != vk::Result::eSuccess)
      Logger::ErrorOut("Failed to enumerate physical devices!");

   vector<vk::PhysicalDevice> physDevices(count);
   vulkInstance.get().enumeratePhysicalDevices(&count, &physDevices[0]);

   vulkPhys = physDevices[0];
}

void Renderer::getExtensions() {
   auto                extensions = vulkPhys.enumerateDeviceExtensionProperties();
   vector<const char*> desired    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DEBUG_MARKER_EXTENSION_NAME};

   for (auto& ext : extensions) {
      for (auto& desiredExt : desired)
         if (string(ext.extensionName) == desiredExt) {
            deviceExtensions.push_back(ext.extensionName);
            Logger::Info("Added extension: ", ext.extensionName);
            break;
         }
   }
}

void Renderer::getLayers() {
   auto                layers        = vulkPhys.enumerateDeviceLayerProperties();
   vector<const char*> desiredLayers = {"VK_LAYER_LUNARG_standard_validation"};

   for (auto& layer : layers) {
      for (auto& desiredLayer : desiredLayers)
         if (string(layer.layerName) == desiredLayer) {
            deviceLayers.push_back(layer.layerName);

            break;
         } else
            Logger::Write("Did not add layer: ", layer.layerName);
   }
}

Renderer::~Renderer() {
   SDL_DestroyWindow(window);
   vulkInstance.get().destroy();

   SDL_Quit();
}
