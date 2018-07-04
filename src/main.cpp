#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <vector>


#include "GLENgine.hpp"

using namespace std;


struct Vert {
   glm::vec3 pos;
   glm::vec4 color;
};


int main() {
   try {
      Logger::SetLogFile("mcpp.log");
      Renderer renderer{"mcpp", {1600, 900}};


      std::vector<Vert>     verts    = {{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
                                 {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
                                 {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}};
      std::vector<unsigned> indicies = {0, 1, 2};


      bool running = true;
      while (running) {
         renderer.updateRender();
         SDL_Event event;
         while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT)
               running = false;
      }
   } catch (const std::runtime_error& e) {
      Logger::Error("UNCAUGHT EXCEPTION: ", e.what());
      // TODO: Forcible cleanup.
   }

   return 0;
}
