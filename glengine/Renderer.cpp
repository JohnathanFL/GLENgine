#include "Renderer.hpp"

void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                   GLsizei length, const GLchar* message,
                   const void* userParam) {
   Logger::Write("GL", message);
}

Renderer::Renderer(const std::string& title, int w, int h) {
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) <
       0) {
      Logger::Write("Renderer::Renderer",
                    "Failed to init SDL2 (error: ", SDL_GetError(),
                    ") exiting");
      exit(1);
   }
   atexit(SDL_Quit);

   SDL_GL_LoadLibrary(nullptr);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
   SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, SDL_TRUE);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                       SDL_GL_CONTEXT_PROFILE_CORE);


   window = SDL_CreateWindow(
       title.c_str(), 0, 0, w, h,
       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

   if (!window) {
      Logger::Write("Renderer::Renderer",
                    "Failed to create window (error: ", SDL_GetError(),
                    "), exiting!");
      exit(2);
   }

   glCtx = SDL_GL_CreateContext(window);
   if (!glCtx) {
      Logger::Write("Renderer::Renderer",
                    "Failed to create a GL context (error: ", SDL_GetError(),
                    "), exiting!");
      exit(3);
   }

   SDL_GL_MakeCurrent(window, glCtx);

   gladLoadGL();
   glEnable(GL_DEBUG_OUTPUT);
   glDebugMessageCallback(debugCallback, nullptr);


   globalUBO =
       GPUBuffer{GPUBuffer::Type::Uniform, GPUBuffer::Storage::StreamCopy};
   globalUBO.bufferStorage(sizeof(GlobalUniforms), nullptr,
                           GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                               GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

   gpuGlobalUniforms = globalUBO.mapRange<GlobalUniforms>(
       0, sizeof(GlobalUniforms),
       GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT);

   if (gpuGlobalUniforms) {
      memcpy(gpuGlobalUniforms, &globalUniforms, sizeof(GlobalUniforms));
   }

   setClearColor({1.0f, 1.0f, 1.0f, 1.0f});
}

void Renderer::updateRender() {
   glViewport(0, 0, 1600, 900);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   for (const auto& drawable : drawables) {
      // Debug stuff
      // printf("Attempting to draw!");
      // printf("Prog id: %i, VAO: %i\n", drawable.prog.id, drawable.geom.vao);

      // use returns true if any state changes were made.
      if (drawable.prog.use()) {
         // If there were state changes, we probably need to rebind things
         drawable.prog.setVecUniform(0, glm::vec3{-0.5f, -0.1f, 0.0f});
      }
      drawable.geom.draw();
   }

   SDL_GL_SwapWindow(window);
}
