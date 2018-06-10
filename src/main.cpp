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

void printStats() {
   Logger::Write(
       "STATS",
       "Version: " + std::string((const char*)glGetString(GL_VERSION)));
   Logger::Write("STATS",
                 "Vendor: " + std::string((const char*)glGetString(GL_VENDOR)));
   Logger::Write(
       "STATS",
       "Renderer: " + std::string((const char*)glGetString(GL_RENDERER)));
}

struct App {
   App(const string& title, glm::ivec2 windowSize)
       : renderer{title, windowSize.x, windowSize.y},
         input{renderer.getWindow()} {}
   Renderer renderer;
   Input    input;
};

int main() {
   try {
      Logger::SetLogFile("mcpp.log");
      App app{"mcpp", {1600, 900}};
      printStats();

      app.renderer.setClearColor({44 / 255.0f, 53 / 255.0f, 57 / 255.0f, 1.0f});


      std::string tester = "Hello, world!";
      app.input.addEvent(0, Input::Event::Quit, [&](const EventData& e) {
         // printf("I said %s before I died!", tester.c_str());
      });


      Shader vert(loadFile("triangle.vert"), Shader::Type::Vertex);
      Shader frag(loadFile("triangle.frag"), Shader::Type::Fragment);
      Shader geom(loadFile("triangle.geom"), Shader::Type::Geometry);

      ShaderProgram prog(vert, frag);
      prog.use();
      // prog.setVecUniform(0, glm::vec3{0.3f, 0.5f, 0.0f});

      std::vector<VertexAttribute> attribs = {
          {VertexAttribute()
               .withIndex(0)
               .withSize(3)
               .withType(VertexAttribute::Type::Float)
               .withNormalized(false)
               .withStride(7 * sizeof(float))
               .withInitialOffset(nullptr)},
          {VertexAttribute()
               .withIndex(1)
               .withSize(4)
               .withType(VertexAttribute::Type::Float)
               .withNormalized(false)
               .withStride(7 * sizeof(float))
               .withInitialOffset((const void*)(3 * sizeof(float)))}};


      std::vector<Vert> verts = {
          {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
          {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
          {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}};
      std::vector<unsigned> indicies = {0, 1, 2};

      GPUBuffer vbuff(GPUBuffer::Type::Vertex, GPUBuffer::Storage::StaticDraw);
      vbuff.upload(&verts[0], vec_sizeof(verts));

      GPUBuffer ibuff(GPUBuffer::Type::Index, GPUBuffer::Storage::StaticDraw);
      ibuff.upload(&indicies[0], vec_sizeof(indicies));

      // If no ibuff desired, simply pass {} in its place
      Geometry triangle{attribs, Geometry::DrawType::Triangles, 0, 3, vbuff,
                        ibuff};
      // TODO: Should renderer be a full scene graph, or should that be
      // something else?
      app.renderer.addDrawable(triangle, prog);

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
