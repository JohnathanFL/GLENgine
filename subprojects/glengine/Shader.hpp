#pragma once
#include <vulkan/vulkan.hpp>

#include "Macros.hpp"

// General idea: internal:: will contain the actual objects, namespace :: will contain wrappers that index into the
// renderer's pools, to maintain better locality. We won't use pointers in the wrappers since we can't be sure that
// vectors won't be resized.

// namespace renderer_impl {
struct Shader {
   vk::Device       dev;
   vk::ShaderModule shaderMod;
   // For now, the entry must be main.
   // TODO: Shader specialization

   enum class Stage : uint {
      Vertex      = VK_SHADER_STAGE_VERTEX_BIT,
      TessControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
      TessEval    = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
      Geometry    = VK_SHADER_STAGE_GEOMETRY_BIT,
      Fragment    = VK_SHADER_STAGE_FRAGMENT_BIT,
      Compute     = VK_SHADER_STAGE_COMPUTE_BIT,
      AllGraphics = VK_SHADER_STAGE_ALL_GRAPHICS,
      All         = VK_SHADER_STAGE_ALL,
      MaxEnum     = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM,
   } stage;

   // Todo: I had this sorta thing in the OpenGL backend, maybe reintroduce it?
   // enum class Type { Vertex, Fragment, Compute, Geometry } type;

   static Shader FromSrc(const std::vector<byte>& src, Stage stage, vk::Device dev) {
      auto createInfo =
          vk::ShaderModuleCreateInfo().setCodeSize(src.size()).setPCode(reinterpret_cast<const uint32*>(src.data()));

      return Shader{dev, dev.createShaderModule(createInfo), stage};
   }

   vk::PipelineShaderStageCreateInfo toPipelineCreateInfo() const {
      return vk::PipelineShaderStageCreateInfo()
          .setStage(vk::ShaderStageFlagBits(stage))
          .setModule(shaderMod)
          .setPName("main")
          .setPSpecializationInfo(nullptr);
   }

   // Since this is basically a thin wrapper, I figure this is appropriate.
   CONVERTABLE_TO_MEMBER(shaderMod)
};


struct Geometry {};


//}  // namespace renderer_impl

// TODO
// struct Shader {
//   uint32                 id;
//   Renderer*              renderer;  // What we index into with id
//   renderer_impl::Shader& operator->() {
//      // Todo: return renderer.shaderPool, or something like that
//   }
//};

// struct ShaderProgram {
//   uint32                        id;
//   Renderer*                     renderer;  // What we index into with id
//   renderer_impl::ShaderProgram& operator->() {
//      // Todo: return renderer.shaderPool, or something like that
//   }
//};

// struct Geometry {
//   uint32                   id;
//   Renderer*                renderer;  // What we index into with id
//   renderer_impl::Geometry& operator->() {
//      // Todo: return renderer.shaderPool, or something like that
//   }
//};
