#pragma once
#include <vulkan/vulkan.hpp>

#include "Macros.hpp"


struct Pipeline {
   vk::Pipeline                                   pipe;
   std::vector<vk::VertexInputBindingDescription> inputBindings;
};
