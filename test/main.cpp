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
      bool running = true;
      Logger::SetLogFile("mcpp.log");
      RenderingBackend* renderer = new VulkanBackend();
      renderer->init("mcpp", {1600, 900});

      Input input{renderer->window};

      std::vector<Vert>     verts    = {{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
                                 {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
                                 {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}};
      std::vector<unsigned> indicies = {0, 1, 2};


      while (!input.shouldQuit()) {
         renderer->updateRender();
         input.update();
      }
   } catch (const std::runtime_error& e) {
      Logger::Error("UNCAUGHT EXCEPTION: ", e.what());
      // TODO: Forcible cleanup.
   }

   return 0;
}
