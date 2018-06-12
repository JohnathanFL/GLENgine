#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <vector>


#include "GLENgine.hpp"

using namespace std;

std::string loadFile(const std::string& fileName) {
   char*  buff;
   size_t size;
   FILE*  file = fopen(fileName.c_str(), "r");
   fseek(file, 0, SEEK_END);
   size = ftell(file);
   fseek(file, 0, SEEK_SET);

   buff = (char*)malloc(size + 1);
   memset(buff, '\0', size + 1);

   fread(buff, size, 1, file);
   fclose(file);

   return std::string(buff);
}

struct Vert {
   glm::vec3 pos;
   glm::vec4 color;
};

struct App {
   App(const string& title, glm::ivec2 windowSize)
       : renderer{title, windowSize}, input{renderer.getWindow()} {}
   Renderer renderer;
   Input    input;
};

int main() {
   try {
      Logger::SetLogFile("mcpp.log");
      App app{"mcpp", {1600, 900}};

      app.renderer.setClearColor({44 / 255.0f, 53 / 255.0f, 57 / 255.0f, 1.0f});


      std::string tester = "Hello, world!";
      app.input.addEvent(0, Input::Event::Quit, [&](const EventData& e) {
         Logger::Write("INFO", "Quitting!");
      });


      std::vector<Vert> verts = {
          {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
          {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
          {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}};
      std::vector<unsigned> indicies = {0, 1, 2};


      while (app.input.handleEvents()) {
         app.renderer.updateRender();
         // TODO: Gamelogic
      }
   } catch (const std::runtime_error& e) {
      Logger::Write("UNCAUGHT EXCEPTION", e.what());
      // TODO: Forcible cleanup.
   }

   return 0;
}
