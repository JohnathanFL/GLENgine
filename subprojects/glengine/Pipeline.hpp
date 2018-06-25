#pragma once
#include <iostream>
#include <optional>
#include <unordered_map>

#include <vulkan/vulkan.hpp>

#include "Macros.hpp"

#include "Shader.hpp"

// I'm remaking all these enums instead of using vulkan-hpp versions because I won't be using the 'e' prefixing in my
// code, and so want to stay consistent.
enum class Topology : VkFlags {
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

static const std::unordered_map<Topology, std::string> TopologyMappings = {
    {Topology::Points, "Points"},
    {Topology::Lines, "Lines"},
    {Topology::LineStrips, "LineStrips"},
    {Topology::Triangles, "Triangles"},
    {Topology::TriangleStrips, "TriangleStrips"},
    {Topology::TriangleFans, "TriangleFans"},
    {Topology::LinesAdj, "LinesAdj"},
    {Topology::LineStripsAdj, "LineStripsAdj"},
    {Topology::TrianglesAdj, "TrianglesAdj"},
    {Topology::TriangleStripsAdj, "TriangleStripsAdj"},
    {Topology::PatchList, "PatchList"},

};
GENERATE_ENUM_STREAM_OPS(Topology, TopologyMappings)

enum class FillMode : VkFlags {
   Fill        = VK_POLYGON_MODE_FILL,
   Line        = VK_POLYGON_MODE_LINE,
   Point       = VK_POLYGON_MODE_POINT,
   RectangleNV = VK_POLYGON_MODE_FILL_RECTANGLE_NV,
};

static const std::unordered_map<FillMode, std::string> FillModeMappings{{FillMode::Fill, "Fill"},
                                                                        {FillMode::Line, "Line"},
                                                                        {FillMode::Point, "Point"},
                                                                        {FillMode::RectangleNV, "RectangleNV"}};
GENERATE_ENUM_STREAM_OPS(FillMode, FillModeMappings)


enum class CullMode : VkFlags {
   None  = VK_CULL_MODE_NONE,
   Front = VK_CULL_MODE_FRONT_BIT,
   Back  = VK_CULL_MODE_BACK_BIT,
   Both  = VK_CULL_MODE_FRONT_AND_BACK,
};

// Back to Physics, baby
enum class FrontFaceRule : VkFlags {
   RightHand = VK_FRONT_FACE_COUNTER_CLOCKWISE,
   LeftHand  = VK_FRONT_FACE_CLOCKWISE,
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

// Todo: ComputePipeline; Better encapsulation
struct GraphicsPipeline {
   vk::Pipeline pipe;

   vk::PipelineCreateFlags flags;
   std::vector<Shader>     shaderStages;

   std::vector<vk::VertexInputAttributeDescription> inputAttributes;
   std::vector<vk::VertexInputBindingDescription>   inputBindings;

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
   vk::SampleMask       sampleMask;
   bool                 enableAlphaCoverage, enableAlphaToOne;


   vk::PipelineDepthStencilStateCreateInfo depthStencilState;


   vk::PipelineColorBlendStateCreateInfo colorBlendState;
   vk::PipelineDynamicStateCreateInfo    dynState;
   vk::PipelineLayout                    layout;
   vk::RenderPass                        renderPass;
   uint32                                subpassIndex;
   vk::Pipeline                          basePipeline;
   int32                                 basePipelineIndex;

   CONVERTABLE_TO_MEMBER(pipe)
   REF_SETTER(shaderStages, ShaderStages)

   // TODO: Probably load from a TOML file, using cpptoml
   // static GraphicsPipeline FromSrc(const TODO& src);
};
