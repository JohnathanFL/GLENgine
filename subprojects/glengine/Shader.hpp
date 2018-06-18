#pragma once
#include "Types.hpp"

#include "Renderer.hpp"

// General idea: internal:: will contain the actual objects, namespace :: will contain wrappers that index into the
// renderer's pools, to maintain better locality. We won't use pointers in the wrappers since we can't be sure that
// vectors won't be resized.

namespace renderer_impl {
struct Shader {
   vk::ShaderModule shaderMod;
};

struct ShaderProgram {};

struct Geometry {};


}  // namespace renderer_impl

struct Shader {
   uint32                 id;
   Renderer*              renderer;  // What we index into with id
   renderer_impl::Shader& operator->() {
      // Todo: return renderer.shaderPool, or something like that
   }
};

struct ShaderProgram {
   uint32                        id;
   Renderer*                     renderer;  // What we index into with id
   renderer_impl::ShaderProgram& operator->() {
      // Todo: return renderer.shaderPool, or something like that
   }
};

struct Geometry {
   uint32                   id;
   Renderer*                renderer;  // What we index into with id
   renderer_impl::Geometry& operator->() {
      // Todo: return renderer.shaderPool, or something like that
   }
};
