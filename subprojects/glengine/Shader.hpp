#pragma once
#include "Types.hpp"

#include "Renderer.hpp"

// General idea: internal:: will contain the actual objects, namespace :: will contain wrappers that index into the
// renderer's pools, to maintain better locality. We won't use pointers in the wrappers since we can't be sure that
// vectors won't be resized.

// namespace renderer_impl {
struct Shader {
   vk::Device       dev;
   vk::ShaderModule shaderMod;

   // Todo: I had this sorta thing in the OpenGL backend, maybe reintroduce it?
   // enum class Type { Vertex, Fragment, Compute, Geometry } type;

   static Shader FromSrc(const std::vector<byte>& src, vk::Device dev) {
      auto createInfo =
          vk::ShaderModuleCreateInfo().setCodeSize(src.size()).setPCode(reinterpret_cast<const uint32*>(src.data()));

      return Shader{dev, dev.createShaderModule(createInfo)};
   }

   // Since this is basically a thin wrapper, I figure this is appropriate.
   inline operator vk::ShaderModule&() { return shaderMod; }
};

struct ShaderProgram {};

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
