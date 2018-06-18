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

   Logger::Info("Initialized Renderer!");
}

void Renderer::initVulkan() {
   createInstance();
   createSurface();
   getPhysical();
   getLogical();
}

void Renderer::getLayers() {
   auto                layers        = vulkan.physical.enumerateDeviceLayerProperties();
   vector<const char*> desiredLayers = {"VK_LAYER_LUNARG_standard_validation"};

   for (auto& layer : layers) {
      for (auto& desiredLayer : desiredLayers)
         if (string(layer.layerName) == desiredLayer) {
            vulkan.deviceLayers.push_back(layer.layerName);

            break;
         } else
            Logger::Write("Did not add layer: ", layer.layerName);
   }
}

VkBool32 Renderer::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
                                 size_t location, int32_t code, const char* layerPrefix, const char* msg,
                                 void* userData) {
   Logger::Write("VULKAN", layerPrefix, " said: ", msg);
   return true;
}

void Renderer::createInstance() {
   Logger::Info("Creating instance...");
   auto appInfo = vk::ApplicationInfo()
                      .setApplicationVersion(1)
                      .setEngineVersion(1)
                      .setPEngineName("GLENgine")
                      .setApiVersion(VK_API_VERSION_1_1);

   // Get required instance extensions
   uint32_t count = 0;
   SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
   vector<const char*> extensions(count);
   SDL_Vulkan_GetInstanceExtensions(window, &count, &extensions[0]);

   // Enable debug reporting
   extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

   auto createInfo = vk::InstanceCreateInfo()
                         .setPApplicationInfo(&appInfo)
                         .setEnabledExtensionCount(extensions.size())
                         .setPpEnabledExtensionNames(extensions.data());


   if (!(vulkan.instance = vk::createInstanceUnique(createInfo)))
      Logger::ErrorOut("Failed to create Vulkan instance!");
   else
      Logger::Info("Created Vulkan instance!");

   setupDebugCallback();
}

void Renderer::setupDebugCallback() {
   auto debugInfo =
       vk::DebugReportCallbackCreateInfoEXT()
           .setFlags(vk::DebugReportFlagBitsEXT(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT |
                                                VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT))
           .setPfnCallback(debugCallback);

   VkDebugReportCallbackCreateInfoEXT info = debugInfo;
   VkDebugReportCallbackEXT           callBack;
   // Dear lord why isn't this in the main vulkan lib?
   auto func = vulkan.instance->getProcAddr("vkCreateDebugReportCallbackEXT");
   if (func) {
      reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(func)(vulkan.instance.get(), &info, nullptr, &callBack);
      vulkan.debugCallback = callBack;
      Logger::Info("Debug reporting enabled!");
   }
}


void Renderer::createSurface() {
   VkSurfaceKHR surface;
   if (!SDL_Vulkan_CreateSurface(window, vulkan.instance.get(), &surface))
      Logger::ErrorOut("Failed to create vulkan surface!");

   vulkan.surface = vk::UniqueSurfaceKHR(surface);
}

void Renderer::getPhysical() {
   // At the moment I only have 1 physical device, so this is rather useless, but whatever.

   uint32_t count;
   if (vulkan.instance->enumeratePhysicalDevices(&count, nullptr) != vk::Result::eSuccess)
      Logger::ErrorOut("Failed to enumerate physical devices!");

   vector<vk::PhysicalDevice> physDevices(count);
   vulkan.instance->enumeratePhysicalDevices(&count, &physDevices[0]);
   for (auto& dev : physDevices) {
      vk::PhysicalDeviceProperties props;
      vk::PhysicalDeviceFeatures   features;

      dev.getProperties(&props);
      dev.getFeatures(&features);

      if (features.geometryShader && props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
         vulkan.physical = dev;
         break;
      }
   }
}

void Renderer::getLogical() {
   getExtensions();
   getLayers();


   auto qFamilyProps = vulkan.physical.getQueueFamilyProperties();
   for (int i = 0; i < qFamilyProps.size(); i++) {
      if (qFamilyProps[i].queueCount > 0 && (qFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
         vulkan.queueIndices.graphics = i;
         break;
      }
   }

   vk::PhysicalDeviceFeatures deviceFeatures;

   float qPriority         = 1.0f;
   auto  graphicsQueueInfo = vk::DeviceQueueCreateInfo()
                                .setQueueCount(1)
                                .setQueueFamilyIndex(vulkan.queueIndices.graphics)
                                .setPQueuePriorities(&qPriority);


   auto logicalInfo = vk::DeviceCreateInfo()
                          .setEnabledExtensionCount(vulkan.deviceExtensions.size())
                          .setPpEnabledExtensionNames(vulkan.deviceExtensions.data())
                          .setEnabledLayerCount(vulkan.deviceLayers.size())
                          .setPpEnabledLayerNames(vulkan.deviceLayers.data())
                          .setPQueueCreateInfos(&graphicsQueueInfo)
                          .setQueueCreateInfoCount(1)
                          .setPEnabledFeatures(&deviceFeatures);

   vulkan.logical       = vk::UniqueDevice(vulkan.physical.createDevice(logicalInfo));
   vulkan.graphicsQueue = vulkan.logical->getQueue(vulkan.queueIndices.graphics, 0);
}

void Renderer::getExtensions() {
   auto                extensions = vulkan.physical.enumerateDeviceExtensionProperties();
   vector<const char*> desired    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DEBUG_MARKER_EXTENSION_NAME};
   for (auto& ext : extensions) {
      for (auto& desiredExt : desired)
         if (string(ext.extensionName) == desiredExt) {
            vulkan.deviceExtensions.push_back(ext.extensionName);
            Logger::Info("Added extension: ", ext.extensionName);
            break;
         }
   }
}


Renderer::~Renderer() {
   vulkan.logical->destroy();

   vulkan.instance->destroy();
   SDL_DestroyWindow(window);
   SDL_Quit();
}
