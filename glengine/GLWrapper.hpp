#pragma once
#include <memory>

#include "glad.h"


struct GLState {
   GLuint vao;
   GLuint texture;
   GLuint program;

   inline static GLState& Current() {
      static std::unique_ptr<GLState> current = std::make_unique<GLState>();
      return *current;
   }
};
