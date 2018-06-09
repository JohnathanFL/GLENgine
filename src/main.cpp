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

void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                   GLsizei length, const GLchar* message,
                   const void* userParam) {
   Logger::Write("GL", message);
}

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

int main() {
   try {
      Logger::SetLogFile("mcpp.log");
      Renderer renderer{"mcpp", 1600, 900};
      printStats();

      Input       input{renderer.getWindow()};
      std::string tester = "Hello, world!";
      input.addEvent(0, Input::Event::Quit, [&](const EventData& e) {
         // printf("I said %s before I died!", tester.c_str());
      });

      glEnable(GL_DEBUG_OUTPUT);
      glDebugMessageCallback(debugCallback, nullptr);

      Shader vert(loadFile("triangle.vert"), Shader::Type::Vertex);
      Shader frag(loadFile("triangle.frag"), Shader::Type::Fragment);
      Shader geom(loadFile("triangle.geom"), Shader::Type::Geometry);

      ShaderProgram prog(vert, frag);

      VertexAttribute posAttrib = VertexAttribute()
                                      .withIndex(0)
                                      .withSize(3)
                                      .withType(VertexAttribute::Type::Float)
                                      .withNormalized(false)
                                      .withStride(7 * sizeof(float))
                                      .withInitialOffset(nullptr);
      VertexAttribute colorAttrib =
          VertexAttribute()
              .withIndex(1)
              .withSize(4)
              .withType(VertexAttribute::Type::Float)
              .withNormalized(false)
              .withStride(7 * sizeof(float))
              .withInitialOffset((const void*)(3 * sizeof(float)));


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
      Geometry triangle{{posAttrib, colorAttrib},
                        Geometry::DrawType::Triangles,
                        0,
                        3,
                        vbuff,
                        ibuff};
      renderer.addDrawable(triangle, prog);

      while (input.handleEvents()) {
         renderer.updateRender();
         // TODO: Gamelogic
      }
   } catch (const std::runtime_error& e) {
      cout << "ERROR: " << e.what() << endl << flush;
      // TODO: Forcible cleanup.
   }

   return 0;
}
