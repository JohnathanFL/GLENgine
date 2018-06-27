#pragma once
#include <iostream>
#include <optional>
#include <unordered_map>

#include <vulkan/vulkan.hpp>

#include "Macros.hpp"

#include "Shader.hpp"

#include <EnumMap.hpp>

// I'm remaking all these enums instead of using vulkan-hpp versions because I won't be using the 'e' prefixing in my
// code, and so want to stay consistent.
enum class STRINGIFY Topology : VkFlags {
   Points            = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
   Lines             = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
   LineStrips        = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
   Triangles         = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
   TriangleStrips    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
   TriangleFans      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
   LinesAdj          = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
   LineStripsAdj     = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
   TrianglesAdj      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
   TriangleStripsAdj = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
   PatchList         = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
};

enum class STRINGIFY FillMode : VkFlags {
   Fill        = VK_POLYGON_MODE_FILL,
   Line        = VK_POLYGON_MODE_LINE,
   Point       = VK_POLYGON_MODE_POINT,
   RectangleNV = VK_POLYGON_MODE_FILL_RECTANGLE_NV,
};


enum class STRINGIFY CullMode : VkFlags {
   None  = VK_CULL_MODE_NONE,
   Front = VK_CULL_MODE_FRONT_BIT,
   Back  = VK_CULL_MODE_BACK_BIT,
   Both  = VK_CULL_MODE_FRONT_AND_BACK,
};

// Back to Physics, baby
enum class FrontFaceRule : VkFlags STRINGIFY{
    RightHand = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    LeftHand  = VK_FRONT_FACE_CLOCKWISE,
};


enum class STRINGIFY CompareOp {
   Never          = VK_COMPARE_OP_NEVER,
   Less           = VK_COMPARE_OP_LESS,
   Equal          = VK_COMPARE_OP_EQUAL,
   LessOrEqual    = VK_COMPARE_OP_LESS_OR_EQUAL,
   Greater        = VK_COMPARE_OP_GREATER,
   NotEqual       = VK_COMPARE_OP_NOT_EQUAL,
   GreaterOrEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
   Always         = VK_COMPARE_OP_ALWAYS
};


enum class STRINGIFY StencilOp {
   Keep              = VK_STENCIL_OP_KEEP,
   Zero              = VK_STENCIL_OP_ZERO,
   Replace           = VK_STENCIL_OP_REPLACE,
   IncrementAndClamp = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
   DecrementAndClamp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
   Invert            = VK_STENCIL_OP_INVERT,
   IncrementAndWrap  = VK_STENCIL_OP_INCREMENT_AND_WRAP,
   DecrementAndWrap  = VK_STENCIL_OP_DECREMENT_AND_WRAP
};

using MultisampleCount = vk::SampleCountFlagBits;
inline MultisampleCount MultiSample(uint8 count) {
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

struct VertexBufferDescription {
   vk::VertexInputBindingDescription*   bindingDesc;
   vk::VertexInputAttributeDescription* attribs;
};

// Todo: ComputePipeline; Better encapsulation
struct GraphicsPipeline {
   vk::Pipeline pipe;

   vk::PipelineCreateFlags flags;
   std::vector<Shader>     shaderStages;

   std::vector<VertexBufferDescription> inputDescs_;

   Topology              topology;
   std::optional<uint32> pointsPerPatch;

   // Viewport + Scissor
   std::vector<std::pair<vk::Viewport, vk::Rect2D>> viewPorts;


   // Would be nice to have C# style properties, so I could just delegate all these to their create infos
   FillMode                 fillMode;
   bool                     enableDepthClamp, discardRaster;
   CullMode                 cullMode;
   FrontFaceRule            faceRule;
   std::optional<DepthBias> depthBias;
   float                    lineWidth;


   MultisampleCount     msaaCount;
   std::optional<float> minSampleShading;
   uint32               sampleMask;
   bool                 enableAlphaCoverage, enableAlphaToOne;


   bool               enableDepthTest, enableDepthWrite, enableDepthBoundsTest;
   CompareOp          compOp;
   bool               enableStencilTest;
   vk::StencilOpState front, back;
   float              minDepth, maxDepth;


   bool enable;


   vk::PipelineDynamicStateCreateInfo dynState;
   vk::PipelineLayout                 layout;
   vk::RenderPass                     renderPass;
   uint32                             subpassIndex;
   vk::Pipeline                       basePipeline;
   int32                              basePipelineIndex;

   CONVERTABLE_TO_MEMBER(pipe)
   REF_SETTER(shaderStages, ShaderStages)

   // TODO: Probably load from a TOML file, using cpptoml
   // static GraphicsPipeline FromSrc(const TODO& src);
};
