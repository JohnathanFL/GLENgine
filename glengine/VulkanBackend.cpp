#include "Renderer.hpp"

#include <set>

#include "Logger.hpp"

using namespace std;

VulkanBackend::~VulkanBackend() {
   const auto& dev = logical;

   dev->destroyShaderModule(this->vert);
   dev->destroyShaderModule(this->frag);

   dev->waitIdle();
   vkDestroySwapchainKHR(*dev, this->swapchain, nullptr);
   dev->destroy();

   this->instance->destroy();
   SDL_DestroyWindow(window);
}

void VulkanBackend::init(const string& windowTitle, glm::ivec2 windowDims) {
   this->windowDims = windowDims;
   window =
       SDL_CreateWindow(windowTitle.c_str(), 0, 0, windowDims.x, windowDims.y, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
   if (!window)
      Logger::ErrorOut("Failed to create a window: ", SDL_GetError());

   createInstance();
   createSurface();
   getPhysical();
   getLogical();
   createSwapchain();
   createRenderPasses();
   createGraphicsPipeline();
   createFrameBuffers();
   createCommandPools();
   createCommandBuffs();
   createSemaphores();
}


VkBool32 VulkanBackend::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
                                      size_t location, int32_t code, const char* layerPrefix, const char* msg,
                                      void* userData) {
   // if (!(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT))
   Logger::Write("VULKAN", layerPrefix, " said: ", msg);
   return true;
}

void VulkanBackend::createInstance() {
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


   if (!(this->instance = vk::createInstanceUnique(createInfo)))
      Logger::ErrorOut("Failed to create Vulkan instance!");
   else
      Logger::Info("Created Vulkan instance!");

   setupDebugCallback();
}

void VulkanBackend::setupDebugCallback() {
   auto debugInfo =
       vk::DebugReportCallbackCreateInfoEXT()
           .setFlags(vk::DebugReportFlagBitsEXT(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT |
                                                VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT))
           .setPfnCallback(debugCallback);

   VkDebugReportCallbackCreateInfoEXT info = debugInfo;
   VkDebugReportCallbackEXT           callBack;
   // Dear lord why isn't this in the main vulkan lib?
   auto func = this->instance->getProcAddr("vkCreateDebugReportCallbackEXT");
   if (func) {
      reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(func)(this->instance.get(), &info, nullptr, &callBack);
      this->debugCallbackObj = callBack;
      Logger::Info("Debug reporting enabled!");
   }
}


void VulkanBackend::createSurface() {
   VkSurfaceKHR surface;
   if (!SDL_Vulkan_CreateSurface(window, this->instance.get(), &surface))
      Logger::ErrorOut("Failed to create vulkan surface!");

   this->surface = vk::UniqueSurfaceKHR(surface);
}


bool VulkanBackend::isDeviceSuitable(const vk::PhysicalDevice& dev, const vk::SurfaceKHR& surface) {
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

vk::SurfaceFormatKHR VulkanBackend::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
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

vk::PresentModeKHR VulkanBackend::choosePresentMode(const std::vector<vk::PresentModeKHR>& modes) {
   const vk::PresentModeKHR rankedModes[] = {vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate};

   // Try to get a preferred mode
   for (const auto& ideal : rankedModes)
      for (const auto& mode : modes)
         if (mode == ideal)
            return mode;


   // But if we can't just go with the LCD
   return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanBackend::chooseResolution(const vk::SurfaceCapabilitiesKHR& caps, const glm::uvec2& curRes) {
   if (caps.currentExtent.width != numeric_limits<uint32>::max())
      return caps.currentExtent;
   else {
      vk::Extent2D actualRes{curRes.x, curRes.y};

      actualRes.width  = max(caps.minImageExtent.width, min(caps.maxImageExtent.width, actualRes.width));
      actualRes.height = max(caps.minImageExtent.height, min(caps.maxImageExtent.height, actualRes.height));

      return actualRes;
   }
}

void VulkanBackend::getPhysical() {
   Logger::Info("Getting Physical Device...");

   // At the moment I only have 1 physical device, so this is rather useless, but whatever.

   uint32_t count;
   if (this->instance->enumeratePhysicalDevices(&count, nullptr) != vk::Result::eSuccess)
      Logger::ErrorOut("Failed to enumerate physical devices!");

   vector<vk::PhysicalDevice> physDevices(count);
   this->instance->enumeratePhysicalDevices(&count, &physDevices[0]);
   for (auto& dev : physDevices) {
      if (isDeviceSuitable(dev, this->surface.get())) {
         this->physical = dev;
         break;
      }
   }
}

QueueIndices VulkanBackend::getQueueFamilyIndices(const vk::PhysicalDevice& physical, const vk::SurfaceKHR& surface) {
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

void VulkanBackend::getExtensions() {
   // Logger::Info("Getting Extensions");
   auto                extensions = this->physical.enumerateDeviceExtensionProperties();
   vector<const char*> desired    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
   for (auto& ext : extensions) {
      for (auto& desiredExt : desired)
         if (string(ext.extensionName) == desiredExt) {
            this->deviceExtensions.push_back(ext.extensionName);
            // Logger::Info("Added extension: ", ext.extensionName);
            break;
         }
   }
}

void VulkanBackend::getLayers() {
   Logger::Info("Getting Layers");
   auto                layers        = this->physical.enumerateDeviceLayerProperties();
   vector<const char*> desiredLayers = {"VK_LAYER_LUNARG_standard_validation"};

   for (auto& layer : layers) {
      for (auto& desiredLayer : desiredLayers)
         if (string(layer.layerName) == desiredLayer) {
            this->deviceLayers.push_back(layer.layerName);

            break;
         } else
            Logger::Write("Did not add layer: ", layer.layerName);
   }
}

void VulkanBackend::getLogical() {
   Logger::Info("Creating Logical Device...");
   getExtensions();
   getLayers();


   // By the time we're here, we have finalized our physical device, so cache the indices.
   this->queueIndices = getQueueFamilyIndices(this->physical, this->surface.get());

   vk::PhysicalDeviceFeatures deviceFeatures;

   set<int>                          queueFamilies = {this->queueIndices.present, this->queueIndices.graphics};
   vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

   float qPriority = 1.0f;
   for (auto queue : queueFamilies)
      queueCreateInfos.push_back(
          vk::DeviceQueueCreateInfo().setQueueCount(1).setQueueFamilyIndex(queue).setPQueuePriorities(&qPriority));

   Logger::Write("Adding ", this->deviceExtensions.size(), " extensions!");

   auto logicalInfo = vk::DeviceCreateInfo()
                          .setEnabledExtensionCount(this->deviceExtensions.size())
                          .setPpEnabledExtensionNames(this->deviceExtensions.data())
                          //.setEnabledLayerCount(this->deviceLayers.size())
                          //.setPpEnabledLayerNames(this->deviceLayers.data())
                          .setPQueueCreateInfos(queueCreateInfos.data())
                          .setQueueCreateInfoCount(queueCreateInfos.size())
                          .setPEnabledFeatures(&deviceFeatures);

   this->logical = vk::UniqueDevice(this->physical.createDevice(logicalInfo));


   this->graphicsQueue = this->logical->getQueue(this->queueIndices.graphics, 0);
   this->presentQueue  = this->logical->getQueue(this->queueIndices.present, 0);
}

void VulkanBackend::createSwapchain() {
   Logger::Info("Creating Swapchain");
   SwapchainSupportInfo info = {this->physical.getSurfaceCapabilitiesKHR(*this->surface),
                                this->physical.getSurfaceFormatsKHR(*this->surface),
                                this->physical.getSurfacePresentModesKHR(*this->surface)};

   vk::SurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(info.formats);
   vk::PresentModeKHR   presentMode   = choosePresentMode(info.modes);
   vk::Extent2D         res           = chooseResolution(info.caps, windowDims);

   this->swapInfo = {surfaceFormat, res};  // Cache these for later usage.

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
                             .setSurface(*this->surface);


   auto   indices        = getQueueFamilyIndices(this->physical, *this->surface);
   uint32 queueIndices[] = {(uint32)indices.graphics, (uint32)indices.present};
   if (indices.graphics != indices.present)
      swapCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
          .setQueueFamilyIndexCount(2)
          .setPQueueFamilyIndices(queueIndices);
   else
      swapCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive)
          .setQueueFamilyIndexCount(0)
          .setPQueueFamilyIndices(nullptr);


   this->swapchain  = this->logical->createSwapchainKHR(swapCreateInfo);
   this->swapImages = this->logical->getSwapchainImagesKHR(this->swapchain);

   for (const auto& image : this->swapImages) {
      // Method chaining factories are truly beautiful.
      //... or perhaps terrible, I can't tell which yet.
      auto viewInfo = vk::ImageViewCreateInfo()
                          .setImage(image)
                          .setViewType(vk::ImageViewType::e2D)
                          .setFormat(this->swapInfo.format.format)
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

      this->swapViews.push_back(this->logical->createImageView(viewInfo));
   }
}

void VulkanBackend::createRenderPasses() {
   auto colorAttachDesc = vk::AttachmentDescription()
                              .setFormat(this->swapInfo.format.format)
                              .setSamples(vk::SampleCountFlagBits::e1)
                              .setLoadOp(vk::AttachmentLoadOp::eClear)
                              .setStoreOp(vk::AttachmentStoreOp::eStore)
                              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                              .setInitialLayout(vk::ImageLayout::eUndefined)
                              .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);


   // Todo: Factorize this
   auto subpass              = SubpassDescription();
   subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
   subpass.colorAttachments  = {
       vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal)};


   auto dep =
       vk::SubpassDependency()
           .setSrcSubpass(VK_SUBPASS_EXTERNAL)
           .setDstSubpass(0)
           .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
           .setSrcAccessMask(vk::AccessFlags(0))
           .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
           .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);


   this->pipe             = std::make_shared<GraphicsPipeline>(*this->logical);
   this->pipe->renderPass = std::make_shared<RenderPass>(*this->logical);
   this->pipe->renderPass->addPass(subpass).addDep(dep).addAttachment(colorAttachDesc).build();
}

void VulkanBackend::createGraphicsPipeline() {
   auto vertSrc = LoadFile("vert.spv");
   auto fragSrc = LoadFile("frag.spv");

   // Todo: Factorize these too
   this->vert = Shader::FromSrc(vertSrc, Shader::Stage::Vertex, *this->logical);
   this->frag = Shader::FromSrc(fragSrc, Shader::Stage::Fragment, *this->logical);

   this->pipe->addStages(this->vert, this->frag);


   // Todo: Find a way to force clang-format to put this stuff on multiple lines, like a sane person.
   auto vertPipelineInfo =
       vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(0).setVertexAttributeDescriptionCount(
           0);

   this->pipe->vertInputState = vertPipelineInfo;

   auto inputAsmInfo = vk::PipelineInputAssemblyStateCreateInfo()
                           .setTopology(vk::PrimitiveTopology::eTriangleList)
                           .setPrimitiveRestartEnable(false);

   this->pipe->setTopology(Topology::Triangles);

   auto viewport = vk::Viewport()
                       .setX(0.0f)
                       .setY(0.0f)
                       .setHeight(this->swapInfo.res.height)
                       .setWidth(this->swapInfo.res.width)
                       .setMinDepth(0.0f)
                       .setMaxDepth(1.0f);


   auto scissor = vk::Rect2D().setOffset({0, 0}).setExtent(this->swapInfo.res);

   this->pipe->addViewport(viewport, scissor);

   this->pipe->setEnableDepthClamp(false)
       .setFillMode(FillMode::Fill)
       .setLineWidth(1.0f)
       .setCullMode(vk::CullModeFlagBits::eBack)
       .setFrontFaceRule(FrontFaceRule::LeftHand)
       .withDepthBias(None<DepthBias>());

   this->pipe->setMSAA(2);

   auto colorBlendState = vk::PipelineColorBlendAttachmentState()
                              .setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB |
                                                 vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR)
                              .setBlendEnable(false);

   this->pipe->addColorBlendAttachments(colorBlendState).setColorBlendOp(None<LogicOp>());

   this->pipe->build();
}

void VulkanBackend::createFrameBuffers() {
   this->swapFramebuffers.resize(this->swapViews.size());

   for (auto i = 0; i < this->swapViews.size(); i++) {
      vk::ImageView attachments[] = {this->swapViews[i]};

      auto frameBuffInfo = vk::FramebufferCreateInfo()
                               .setRenderPass(*this->pipe->renderPass)
                               .setAttachmentCount(1)
                               .setPAttachments(attachments)
                               .setWidth(this->swapInfo.res.width)
                               .setHeight(this->swapInfo.res.height)
                               .setLayers(1);
      this->swapFramebuffers[i] = this->logical->createFramebufferUnique(frameBuffInfo);
   }
}

void VulkanBackend::createCommandPools() {
   auto poolInfo = vk::CommandPoolCreateInfo()
                       .setQueueFamilyIndex(getQueueFamilyIndices(this->physical, *this->surface).graphics)
                       .setFlags(vk::CommandPoolCreateFlagBits(0));

   this->commandPool = this->logical->createCommandPoolUnique(poolInfo);
}

void VulkanBackend::createCommandBuffs() {
   auto allocInfo = vk::CommandBufferAllocateInfo()
                        .setCommandPool(*this->commandPool)
                        .setLevel(vk::CommandBufferLevel::ePrimary)
                        .setCommandBufferCount(this->swapFramebuffers.size());

   this->cmdBuffs = this->logical->allocateCommandBuffers(allocInfo);

   for (size_t i = 0; i < this->cmdBuffs.size(); i++) {
      const auto& cmd = this->cmdBuffs[i];

      auto beginInfo = vk::CommandBufferBeginInfo()
                           .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
                           .setPInheritanceInfo(nullptr);
      cmd.begin(beginInfo);

      this->clearColor.setColor(array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
      cmd.beginRenderPass(vk::RenderPassBeginInfo()
                              .setRenderPass(*this->pipe->renderPass)
                              .setRenderArea({{0, 0}, this->swapInfo.res})
                              .setFramebuffer(*this->swapFramebuffers[i])
                              .setClearValueCount(1)
                              .setPClearValues(&clearColor),
                          vk::SubpassContents::eInline);

      cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *this->pipe->pipe);
      cmd.draw(3, 1, 0, 0);
      cmd.endRenderPass();
      cmd.end();
   }
}

void VulkanBackend::createSemaphores() {
   vk::SemaphoreCreateInfo semInfo;
   for (size_t i = 0; i < this->maxFramesInFlight; i++) {
      this->imageAvailSems.push_back(this->logical->createSemaphoreUnique(semInfo));
      this->renderFinishedSems.push_back(this->logical->createSemaphoreUnique(semInfo));
   }
}
