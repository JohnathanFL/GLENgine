#pragma once
#include <iostream>
#include <optional>
#include <typeindex>
#include <unordered_map>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include "Macros.hpp"

#include "Shader.hpp"
#include "VulkanBase.hpp"

#include <EnumMap.hpp>

#include "PipelineEnums.hpp"


using MultisampleCount = vk::SampleCountFlagBits;
constexpr MultisampleCount MultiSample(uint8 count) {
   // Thanks to https://stackoverflow.com/questions/600293/how-to-check-if-a-number-is-a-power-of-2#600306 for the
   // elegant solution
   if (count != 0 && !(count & (count - 1)))  // Is a power of 2
      return vk::SampleCountFlagBits(count);
   else
      return vk::SampleCountFlagBits(2);  // Default to 2 if invalid input
}

struct DepthBias {
   float constantFactor;
   float clamp;
   float slopeFactor;
};
struct DepthTest {
   bool      writeEnable;
   CompareOp compareOp;
   bool      boundsTest;
};
struct StencilTest {
   StencilOp front;
   StencilOp back;
};

struct VertexBufferDescription {
   vk::VertexInputBindingDescription*   bindingDesc;
   vk::VertexInputAttributeDescription* attribs;
};


const std::unordered_map<std::type_index, vk::Format> TypeToVkFormat = {
    {typeindex(glm::vec3), vk::Format::eR32G32B32Sfloat},
    {typeindex(glm::vec2), vk::Format::eR32G32Sfloat},
    {typeindex(glm::vec1), vk::Format::eR32Sfloat}};

// For example: ... = VertexInputAttribsBuilder().then<glm::vec3>().then<glm::vec3>().build();
class VertexDescBuilder {
  public:
   // Thou shalt not store this object
   explicit VertexDescBuilder(uint32 buffBinding = 0, bool perInstance = false) {
      this->buffBinding = buffBinding;
      this->perInstance = perInstance;
      curOffset = curLoc = 0;
   }
   VertexDescBuilder(const VertexDescBuilder& other)  = delete;
   VertexDescBuilder(const VertexDescBuilder&& other) = delete;
   template <typename T>
   VertexDescBuilder& then(uint32 dataFrom = 0) {
      descs.push_back(vk::VertexInputAttributeDescription()
                          .setLocation(curLoc)
                          .setBinding(dataFrom)
                          .setFormat(TypeToVkFormat.at(typeindex(T)))
                          .setOffset(curOffset));
      curLoc++;
      curOffset += sizeof(T);
   }

   inline std::pair<vk::VertexInputBindingDescription, std::vector<vk::VertexInputAttributeDescription>> build() {
      return {vk::VertexInputBindingDescription()
                  .setBinding(buffBinding)
                  .setInputRate(perInstance ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex)
                  .setStride(curOffset),  // Cur offset after vec3,vec3,vec3 would already be 36, for ex
              descs};
   }

  private:
   uint32 buffBinding;
   bool   perInstance;

   size_t                                           curOffset;
   uint32                                           curLoc;
   std::vector<vk::VertexInputAttributeDescription> descs;
};

struct PipelineLayout : VulkanObject {
   DEFAULT_VULKANOBJECT_CTOR(PipelineLayout)

   vk::UniquePipelineLayout layout = vk::UniquePipelineLayout(nullptr);
   CONVERTABLE_TO_MEMBER(layout)
   inline operator bool() { return layout.operator bool(); }

   inline void build() {
      auto createInfo = vk::PipelineLayoutCreateInfo()
                            .setSetLayoutCount(setLayouts.size())
                            .setPSetLayouts(setLayouts.data())
                            .setPushConstantRangeCount(pushConstRanges.size())
                            .setPPushConstantRanges(pushConstRanges.data());
      layout = dev.createPipelineLayoutUnique(createInfo);
   }

   std::vector<vk::DescriptorSetLayout> setLayouts;
   std::vector<vk::PushConstantRange>   pushConstRanges;
};

struct RenderPass : VulkanObject {
   DEFAULT_VULKANOBJECT_CTOR(RenderPass)

   vk::UniqueRenderPass pass;
   CONVERTABLE_TO_MEMBER(pass)

   inline void build() {
      vk::SubpassDescription* descs;
      vk::SubpassDependency*  deps;
      std::tie(descs, deps) = subpasses.data();

      auto createInfo = vk::RenderPassCreateInfo()
                            .setAttachmentCount(attachDescs.size())
                            .setPAttachments(attachDescs.data())
                            .setSubpassCount(subpasses.size())
                            .setPSubpasses(descs)
                            .setPDependencies(deps);
      pass = dev.createRenderPassUnique(createInfo);
   }
   inline operator bool() { return pass.operator bool(); }

   std::vector<vk::AttachmentDescription>                 attachDescs;
   NVector<vk::SubpassDescription, vk::SubpassDependency> subpasses;
};

// Todo: ComputePipeline; Better encapsulation
class GraphicsPipeline : VulkanObject {
  protected:
   GraphicsPipeline(vk::Device dev) : VulkanObject(dev) {}

  public:
   friend class std::shared_ptr<GraphicsPipeline>;  // Since this has so much stuff and should rarely be remade, force
                                                    // us to use a shared_ptr to access it.

   vk::UniquePipeline pipe;

   vk::PipelineCreateFlags                        flags;
   std::vector<vk::PipelineShaderStageCreateInfo> stages;

   std::vector<vk::VertexInputAttributeDescription> inputAttribDescs;  // Per vertex
   std::vector<vk::VertexInputBindingDescription>   inputBindDescs;    // Per buffer

   vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;  // see setTopology and setPrimRestart
   vk::PipelineTessellationStateCreateInfo  tessellationState;

   std::vector<vk::Viewport> viewports;
   std::vector<vk::Rect2D>   viewScissors;

   vk::PipelineRasterizationStateCreateInfo rasterizationState;
   vk::PipelineMultisampleStateCreateInfo   multisampleState;
   vk::PipelineDepthStencilStateCreateInfo  depthStencilState;

   std::optional<LogicOp>                             colorBlendLogicOp;
   std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
   glm::vec4                                          colorBlendConstants;

   // Skipping dynamic states for now

   SharedPtr<PipelineLayout>   layout;
   SharedPtr<RenderPass>       renderPass;
   uint32_t                    subpassIndex;
   SharedPtr<GraphicsPipeline> basePipeline;
   int32_t                     basePipelineIndex;

   // I pity the fool who has to read this, but that's just what the vk pipeline is like.
   inline GraphicsPipeline& setFlags(vk::PipelineCreateFlags fl);
   inline GraphicsPipeline& clearStages();
   template <typename... Args>
   inline GraphicsPipeline& withStages(const Args&... shaders);
   inline GraphicsPipeline& setTopology(const Topology& top);
   inline GraphicsPipeline& setPrimitiveRestart(bool v);
   inline GraphicsPipeline& withAttribs(std::vector<vk::VertexInputAttributeDescription>&& attribs);
   inline GraphicsPipeline& setPatchControlPoints(uint32 p);
   inline GraphicsPipeline& addViewport(vk::Viewport view, vk::Rect2D scissor);
   inline GraphicsPipeline& enableDepthClamp(bool enable = true);
   inline GraphicsPipeline& setfillMode(const FillMode& mode);
   inline GraphicsPipeline& setCullMode(const CullMode& mode);
   inline GraphicsPipeline& setFrontFaceRule(const FrontFaceRule& rule);
   inline GraphicsPipeline& withDepthBias(std::optional<DepthBias> bias);
   inline GraphicsPipeline& setLineWidth(float width);
   inline GraphicsPipeline& setMSAA(uint32 samples);
   inline GraphicsPipeline& withDepthTest(std::optional<DepthTest> test);
   inline GraphicsPipeline& withStencilTest(std::optional<StencilTest> test);
   inline GraphicsPipeline& withDepthBounds(glm::vec2&& bounds);
   inline GraphicsPipeline& withColorBlendOp(std::optional<LogicOp> op);
   template <typename... Args>
   inline GraphicsPipeline& addColorBlendAttachments(const Args&... args);
   inline GraphicsPipeline& setBlendConstants(const glm::vec4& consts);
   inline GraphicsPipeline& setLayout(SharedPtr<PipelineLayout> layout);
   inline GraphicsPipeline& setRenderPass(SharedPtr<RenderPass> pass, uint32 subpass);
   inline GraphicsPipeline& setBasePipeline(SharedPtr<GraphicsPipeline> base);
   inline GraphicsPipeline& setBasePipelineIndex(uint32 i);

   // TODO: sample shading params

   CONVERTABLE_TO_MEMBER(pipe)


   // TODO: Probably load from a TOML file, using cpptoml
   // static GraphicsPipeline FromSrc(const TODO& src);
};


GraphicsPipeline& GraphicsPipeline::setFlags(vk::PipelineCreateFlags fl) {
   flags = fl;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::clearStages() {
   stages.clear();
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setTopology(const Topology& top) {
   inputAssemblyState.topology = vk::PrimitiveTopology(top);
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setPrimitiveRestart(bool v) {
   inputAssemblyState.primitiveRestartEnable = v;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::withAttribs(std::vector<vk::VertexInputAttributeDescription>&& attribs) {
   inputAttribDescs = attribs;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setPatchControlPoints(uint32 p) {
   tessellationState.patchControlPoints = p;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::addViewport(vk::Viewport view, vk::Rect2D scissor) {
   viewports.push_back(view);
   viewScissors.push_back(scissor);
   return *this;
}

GraphicsPipeline& GraphicsPipeline::enableDepthClamp(bool enable) {
   rasterizationState.depthClampEnable = enable;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setfillMode(const FillMode& mode) {
   rasterizationState.polygonMode = vk::PolygonMode(mode);
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setCullMode(const CullMode& mode) {
   rasterizationState.cullMode = vk::CullModeFlagBits(mode);
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setFrontFaceRule(const FrontFaceRule& rule) {
   rasterizationState.frontFace = vk::FrontFace(rule);
   return *this;
}

GraphicsPipeline& GraphicsPipeline::withDepthBias(std::optional<DepthBias> bias) {
   if (bias.has_value())
      rasterizationState.setDepthBiasEnable(true)
          .setDepthBiasClamp(bias->clamp)
          .setDepthBiasConstantFactor(bias->constantFactor)
          .setDepthBiasSlopeFactor(bias->slopeFactor);
   else
      rasterizationState.setDepthBiasEnable(false);

   return *this;
}

GraphicsPipeline& GraphicsPipeline::setLineWidth(float width) {
   rasterizationState.lineWidth = width;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setMSAA(uint32 samples) {
   multisampleState.setRasterizationSamples(MultiSample(samples));
   return *this;
}

GraphicsPipeline& GraphicsPipeline::withDepthTest(std::optional<DepthTest> test) {
   if (test.has_value())
      depthStencilState.setDepthTestEnable(true)
          .setDepthBoundsTestEnable(test->boundsTest)
          .setDepthCompareOp(vk::CompareOp(test->compareOp))
          .setDepthWriteEnable(test->writeEnable);
   else
      depthStencilState.setDepthTestEnable(false);
   return *this;
}

GraphicsPipeline& GraphicsPipeline::withStencilTest(std::optional<StencilTest> test) {
   if (test.has_value())
      depthStencilState.setStencilTestEnable(true)
          .setFront(vk::StencilOp(test->front))
          .setBack(vk::StencilOp(test->back));
   else
      depthStencilState.setStencilTestEnable(false);

   return *this;
}

GraphicsPipeline& GraphicsPipeline::withDepthBounds(glm::vec2&& bounds) {  // using {min, max}
   depthStencilState.setMinDepthBounds(bounds.x).setMaxDepthBounds(bounds.y);
   return *this;
}

GraphicsPipeline& GraphicsPipeline::withColorBlendOp(std::optional<LogicOp> op) {
   colorBlendLogicOp = op;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setBlendConstants(const glm::vec4& consts) {
   colorBlendConstants = consts;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setLayout(SharedPtr<PipelineLayout> layout) {
   this->layout = layout;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setRenderPass(SharedPtr<RenderPass> pass, uint32 subpass) {
   renderPass   = pass;
   subpassIndex = subpass;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setBasePipeline(SharedPtr<GraphicsPipeline> base) {
   basePipeline = base;
   return *this;
}

GraphicsPipeline& GraphicsPipeline::setBasePipelineIndex(uint32 i) {
   basePipelineIndex = i;
   return *this;
}
template <typename... Args>
inline GraphicsPipeline& GraphicsPipeline::withStages(const Args&... shaders) {
   for (const Shader& shader : {shaders...})
      stages.push_back(shader.toPipelineCreateInfo());

   return *this;
}

template <typename... Args>
inline GraphicsPipeline& GraphicsPipeline::addColorBlendAttachments(const Args&... args) {
   for (const vk::PipelineColorBlendAttachmentState& state : {args...})
      colorBlendAttachments.push_back(state);
}

enum class STRINGIFY PipelineType { Graphics, Compute };
