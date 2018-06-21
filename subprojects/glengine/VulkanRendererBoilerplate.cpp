#include "Renderer.hpp"

#include <set>

using namespace std;

void Renderer::initVulkan() {
   createInstance();
   createSurface();
   getPhysical();
   getLogical();
   createSwapchain();
   createGraphicsPipeline();
}


VkBool32 Renderer::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
                                 size_t location, int32_t code, const char* layerPrefix, const char* msg,
                                 void* userData) {
   // if (!(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT))
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


bool Renderer::isDeviceSuitable(const vk::PhysicalDevice& dev, const vk::SurfaceKHR& surface) {
   vk::PhysicalDeviceProperties props;
   vk::PhysicalDeviceFeatures   features;

   dev.getProperties(&props);
   dev.getFeatures(&features);

   bool suitable = features.geometryShader && (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu);


   if (suitable) {
      // Avoid querying anything else if we're already false.

      SwapchainSupportInfo swapInfo;

      dev.getSurfaceCapabilitiesKHR(surface, &swapInfo.caps);
      swapInfo.formats = dev.getSurfaceFormatsKHR(surface);
      swapInfo.modes   = dev.getSurfacePresentModesKHR(surface);

      suitable = suitable && !swapInfo.formats.empty() && !swapInfo.modes.empty();

      suitable = suitable && getQueueFamilyIndices(dev, surface).isComplete();
   }

   return suitable;
}

vk::SurfaceFormatKHR Renderer::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
   static const vk::SurfaceFormatKHR idealFormat = {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};

   if ((formats.size() == 1) && (formats[0].format == vk::Format::eUndefined))
      return idealFormat;
   else {
      for (const auto& format : formats) {
         if (format == idealFormat)
            return format;
      }
      // We got here, we can't have the ideal format.

      return formats[0];
   }
}

vk::PresentModeKHR Renderer::choosePresentMode(const std::vector<vk::PresentModeKHR>& modes) {
   const vk::PresentModeKHR rankedModes[] = {vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate};

   // Try to get a preferred mode
   for (const auto& ideal : rankedModes)
      for (const auto& mode : modes)
         if (mode == ideal)
            return mode;


   // But if we can't just go with the LCD
   return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Renderer::chooseResolution(const vk::SurfaceCapabilitiesKHR& caps, const glm::uvec2& curRes) {
   if (caps.currentExtent.width != numeric_limits<uint32>::max())
      return caps.currentExtent;
   else {
      vk::Extent2D actualRes{curRes.x, curRes.y};

      actualRes.width  = max(caps.minImageExtent.width, min(caps.maxImageExtent.width, actualRes.width));
      actualRes.height = max(caps.minImageExtent.height, min(caps.maxImageExtent.height, actualRes.height));

      return actualRes;
   }
}

void Renderer::getPhysical() {
   Logger::Info("Getting Physical Device...");

   // At the moment I only have 1 physical device, so this is rather useless, but whatever.

   uint32_t count;
   if (vulkan.instance->enumeratePhysicalDevices(&count, nullptr) != vk::Result::eSuccess)
      Logger::ErrorOut("Failed to enumerate physical devices!");

   vector<vk::PhysicalDevice> physDevices(count);
   vulkan.instance->enumeratePhysicalDevices(&count, &physDevices[0]);
   for (auto& dev : physDevices) {
      if (isDeviceSuitable(dev, vulkan.surface.get())) {
         vulkan.physical = dev;
         break;
      }
   }
}

QueueIndices Renderer::getQueueFamilyIndices(const vk::PhysicalDevice& physical, const vk::SurfaceKHR& surface) {
   QueueIndices result;

   auto qFamilyProps = physical.getQueueFamilyProperties();
   for (int i = 0; i < qFamilyProps.size(); i++) {
      if (qFamilyProps[i].queueCount > 0 && (qFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics))
         result.graphics = i;

      if (qFamilyProps[i].queueCount > 0 && physical.getSurfaceSupportKHR(i, surface))
         result.present = i;

      if (result.isComplete())
         break;
   }

   return result;
}

void Renderer::getExtensions() {
   // Logger::Info("Getting Extensions");
   auto                extensions = vulkan.physical.enumerateDeviceExtensionProperties();
   vector<const char*> desired    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
   for (auto& ext : extensions) {
      for (auto& desiredExt : desired)
         if (string(ext.extensionName) == desiredExt) {
            vulkan.deviceExtensions.push_back(ext.extensionName);
            // Logger::Info("Added extension: ", ext.extensionName);
            break;
         }
   }
}

void Renderer::getLayers() {
   Logger::Info("Getting Layers");
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

void Renderer::getLogical() {
   Logger::Info("Creating Logical Device...");
   getExtensions();
   getLayers();


   // By the time we're here, we have finalized our physical device, so cache the indices.
   vulkan.queueIndices = getQueueFamilyIndices(vulkan.physical, vulkan.surface.get());

   vk::PhysicalDeviceFeatures deviceFeatures;

   set<int>                          queueFamilies = {vulkan.queueIndices.present, vulkan.queueIndices.graphics};
   vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

   float qPriority = 1.0f;
   for (auto queue : queueFamilies)
      queueCreateInfos.push_back(
          vk::DeviceQueueCreateInfo().setQueueCount(1).setQueueFamilyIndex(queue).setPQueuePriorities(&qPriority));

   Logger::Write("Adding ", vulkan.deviceExtensions.size(), " extensions!");

   auto logicalInfo = vk::DeviceCreateInfo()
                          .setEnabledExtensionCount(vulkan.deviceExtensions.size())
                          .setPpEnabledExtensionNames(vulkan.deviceExtensions.data())
                          //.setEnabledLayerCount(vulkan.deviceLayers.size())
                          //.setPpEnabledLayerNames(vulkan.deviceLayers.data())
                          .setPQueueCreateInfos(queueCreateInfos.data())
                          .setQueueCreateInfoCount(queueCreateInfos.size())
                          .setPEnabledFeatures(&deviceFeatures);

   vulkan.logical = vk::UniqueDevice(vulkan.physical.createDevice(logicalInfo));


   vulkan.graphicsQueue = vulkan.logical->getQueue(vulkan.queueIndices.graphics, 0);
   vulkan.presentQueue  = vulkan.logical->getQueue(vulkan.queueIndices.present, 0);
}

void Renderer::createSwapchain() {
   Logger::Info("Creating Swapchain");
   SwapchainSupportInfo info = {vulkan.physical.getSurfaceCapabilitiesKHR(*vulkan.surface),
                                vulkan.physical.getSurfaceFormatsKHR(*vulkan.surface),
                                vulkan.physical.getSurfacePresentModesKHR(*vulkan.surface)};

   vk::SurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(info.formats);
   vk::PresentModeKHR   presentMode   = choosePresentMode(info.modes);
   vk::Extent2D         res           = chooseResolution(info.caps, windowDims);

   vulkan.swapInfo = {surfaceFormat, res};  // Cache these for later usage.

   uint32 imageCount = clamp(info.caps.minImageCount + 1, (uint32)0, info.caps.maxImageCount);

   // Basic info
   auto swapCreateInfo = vk::SwapchainCreateInfoKHR()
                             .setMinImageCount(imageCount)
                             .setImageFormat(surfaceFormat.format)
                             .setImageColorSpace(surfaceFormat.colorSpace)
                             .setImageExtent(res)
                             .setImageArrayLayers(1)
                             .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                             .setPreTransform(info.caps.currentTransform)
                             .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                             .setPresentMode(presentMode)
                             .setClipped(true)
                             .setOldSwapchain(vk::SwapchainKHR(nullptr))
                             .setSurface(*vulkan.surface);


   auto   indices        = getQueueFamilyIndices(vulkan.physical, *vulkan.surface);
   uint32 queueIndices[] = {(uint32)indices.graphics, (uint32)indices.present};
   if (indices.graphics != indices.present)
      swapCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
          .setQueueFamilyIndexCount(2)
          .setPQueueFamilyIndices(queueIndices);
   else
      swapCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive)
          .setQueueFamilyIndexCount(0)
          .setPQueueFamilyIndices(nullptr);


   vulkan.swapchain  = vulkan.logical->createSwapchainKHR(swapCreateInfo);
   vulkan.swapImages = vulkan.logical->getSwapchainImagesKHR(vulkan.swapchain);

   for (const auto& image : vulkan.swapImages) {
      // Method chaining factories are truly beautiful.
      //... or perhaps terrible, I can't tell which yet.
      auto viewInfo = vk::ImageViewCreateInfo()
                          .setImage(image)
                          .setViewType(vk::ImageViewType::e2D)
                          .setFormat(vulkan.swapInfo.format.format)
                          .setComponents(vk::ComponentMapping()
                                             .setA(vk::ComponentSwizzle::eIdentity)
                                             .setB(vk::ComponentSwizzle::eIdentity)
                                             .setG(vk::ComponentSwizzle::eIdentity)
                                             .setR(vk::ComponentSwizzle::eIdentity))
                          .setSubresourceRange(vk::ImageSubresourceRange()
                                                   .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                                   .setBaseMipLevel(0)
                                                   .setLevelCount(1)
                                                   .setBaseArrayLayer(0)
                                                   .setLayerCount(1));

      vulkan.swapViews.push_back(vulkan.logical->createImageView(viewInfo));
   }
}

void Renderer::createGraphicsPipeline() {
   auto vertSrc = LoadFile("vert.spv");
   auto fragSrc = LoadFile("frag.spv");

   vert = Shader::FromSrc(vertSrc, *vulkan.logical);
   frag = Shader::FromSrc(fragSrc, *vulkan.logical);

   vk::PipelineShaderStageCreateInfo stages[2];

   stages[0].setModule(vert).setStage(vk::ShaderStageFlagBits::eFragment).setPName("main");
   stages[1].setModule(frag).setStage(vk::ShaderStageFlagBits::eFragment).setPName("main");

   // Todo: Find a way to force clang-format to put this stuff on multiple lines, like a sane person.
   auto vertPipelineInfo =
       vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(0).setVertexAttributeDescriptionCount(
           0);

   auto inputAsmInfo = vk::PipelineInputAssemblyStateCreateInfo()
                           .setTopology(vk::PrimitiveTopology::eTriangleList)
                           .setPrimitiveRestartEnable(false);

   auto viewport = vk::Viewport()
                       .setX(0.0f)
                       .setY(0.0f)
                       .setHeight(vulkan.swapInfo.res.height)
                       .setWidth(vulkan.swapInfo.res.width)
                       .setMinDepth(0.0f)
                       .setMaxDepth(1.0f);

   auto scissor = vk::Rect2D().setOffset({0, 0}).setExtent(vulkan.swapInfo.res);

   auto viewportStateInfo = vk::PipelineViewportStateCreateInfo()
                                .setViewportCount(1)
                                .setPViewports(&viewport)
                                .setScissorCount(1)
                                .setPScissors(&scissor);

   auto rasterizerInfo = vk::PipelineRasterizationStateCreateInfo()
                             .setDepthClampEnable(false)
                             .setRasterizerDiscardEnable(false)
                             .setPolygonMode(vk::PolygonMode::eFill)
                             .setLineWidth(1.0f)
                             .setCullMode(vk::CullModeFlagBits::eBack)
                             .setFrontFace(vk::FrontFace::eClockwise)
                             .setDepthBiasEnable(false);

   auto multisampleInfo = vk::PipelineMultisampleStateCreateInfo()
                              .setSampleShadingEnable(false)
                              .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                              .setMinSampleShading(1.0f)
                              .setPSampleMask(nullptr);

   auto colorBlendState = vk::PipelineColorBlendAttachmentState()
                              .setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB |
                                                 vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR)
                              .setBlendEnable(false);

   auto colorBlendInfo =
       vk::PipelineColorBlendStateCreateInfo().setLogicOpEnable(false).setAttachmentCount(1).setPAttachments(
           &colorBlendState);

   auto pipeLayoutInfo = vk::PipelineLayoutCreateInfo().setSetLayoutCount(0);
   vulkan.pipeLayout   = vulkan.logical->createPipelineLayout(pipeLayoutInfo);
}
