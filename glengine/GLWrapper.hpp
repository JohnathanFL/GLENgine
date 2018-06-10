#pragma once
#include <memory>

#include "glad.h"

struct GLState {
   GLState() { vao = texture = program = 0; }

   GLuint vao;
   GLuint texture;
   GLuint program;


   inline static GLState& Current() {
      static std::unique_ptr<GLState> current = std::make_unique<GLState>();
      return *current;
   }
};
