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
   Renderer(std::string title, int w, int h);
   ~Renderer() {
   }

   inline void addDrawable(const Geometry& geom, const ShaderProgram& prog) {
      drawables.push_back({false, geom, prog});
   }
   void updateRender();


   SDL_Window* getWindow() {
      return window;
   }

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
};
