#include "Renderer.hpp"

Renderer::Renderer(std::__cxx11::string title, int w, int h) {
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) <
       0) {
      printf("Failed to init SDL2, exiting...\n");
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
   glCtx = SDL_GL_CreateContext(window);
   if (!glCtx) {
      printf("Failed to create an SDL_GLContext! Exiting...\n");
      exit(2);
   }

   SDL_GL_MakeCurrent(window, glCtx);

   gladLoadGL();
}

void Renderer::updateRender() {
   glViewport(0, 0, 1600, 900);
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   for (const auto& drawable : drawables) {
      // printf("Attempting to draw!");
      // printf("Prog id: %i, VAO: %i\n", drawable.prog.id, drawable.geom.vao);

      drawable.prog.use();
      drawable.geom.draw();
   }

   SDL_GL_SwapWindow(window);
}
