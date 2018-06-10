#pragma once
#include <functional>
#include <map>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

#include "GLWrapper.hpp"

#include <SDL2/SDL.h>

#include "Geometry.hpp"
#include "Shader.hpp"

class Renderer {
  public:
   Renderer(const std::string& title, int w, int h);
   ~Renderer() {}

   inline void addDrawable(const Geometry& geom, const ShaderProgram& prog) {
      drawables.push_back({false, geom, prog});
   }
   void updateRender();


   SDL_Window* getWindow() { return window; }

   const glm::vec4& getClearColor() const { return clearColor; }
   void             setClearColor(const glm::vec4& col) {
      clearColor = col;
      glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
   }

   struct GlobalUniforms {
      float     time;
      glm::mat4 projection;
      glm::mat4 view;
   };

   GPUBuffer       globalUBO;
   GlobalUniforms  globalUniforms = {0.0f, glm::mat4(0.0f), glm::mat4(0.0f)};
   GlobalUniforms* gpuGlobalUniforms;

  private:
   struct Drawable {
      bool          skip;
      Geometry      geom;
      ShaderProgram prog;
      // TODO: std::optional<Texture> tex;
   };
   std::vector<Drawable> drawables;  // TODO: Sorting based on state changes.
   SDL_GLContext         glCtx;
   SDL_Window*           window;

   glm::vec4 clearColor;
};
