#pragma once

#include <vulkan/vulkan.hpp>

static vk::Device NullDevice;  // For allowing VulkanObjects to be constructed with no params and be null.


// TODO: Some more base things would be nice.

struct VulkanObject {
   vk::Device& dev;
   VulkanObject() : dev{NullDevice} {}
   VulkanObject(vk::Device& device) : dev{device} {}
};

#define DEFAULT_VULKANOBJECT_CTOR(TYPE) \
                                        \
   TYPE(vk::Device dev) : VulkanObject(dev) {}
