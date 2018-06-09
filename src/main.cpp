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
   template <typename... Args>
   Vert(Args... args) {
      pos = glm::vec3(args...);
   }


   glm::vec3 pos;
};

void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                   GLsizei length, const GLchar* message,
                   const void* userParam) {
   printf("GL ERROR MESSAGE: %s\n\n", message);
}

void printStats() {
   printf("Version: %s\n", glGetString(GL_VERSION));
   printf("Vender: %s\n", glGetString(GL_VENDOR));
   printf("Renderer: %s\n", glGetString(GL_RENDERER));
   printf("===========================================================\n\n");
}

int main() {
   try {
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


      printf("Vertex: %i, Fragment: %i\n", vert.id, frag.id);

      ShaderProgram prog(vert, frag);

      VertexAttribute attrib = VertexAttribute()
                                   .withIndex(0)
                                   .withSize(3)
                                   .withType(VertexAttribute::Type::Float)
                                   .withNormalized(false)
                                   .withStride(3 * sizeof(float))
                                   .withInitialOffset(nullptr);


      std::vector<Vert> verts = {
          {-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f, 0.0f}, {0.0f, 0.5f, 0.0f}};
      std::vector<unsigned> indicies = {0, 1, 2};

      GPUBuffer vbuff(GPUBuffer::Type::Vertex, GPUBuffer::Storage::StaticDraw);
      vbuff.upload(&verts[0], vec_sizeof(verts));

      GPUBuffer ibuff(GPUBuffer::Type::Index, GPUBuffer::Storage::StaticDraw);
      ibuff.upload(&indicies[0], vec_sizeof(indicies));

      Geometry triangle{{attrib}, Geometry::DrawType::Triangles, 0, 3, vbuff,
                        ibuff};
      renderer.addDrawable(triangle, prog);

      while (input.handleEvents()) {
         renderer.updateRender();
         // TODO: Gamelogic
      }
   } catch (std::runtime_error& e) {
      cout << "ERROR: " << e.what() << endl << flush;
      // TODO: Forcible cleanup.
   }
}
