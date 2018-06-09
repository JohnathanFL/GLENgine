#pragma once
#include <iostream>
#include <memory>
#include <string>

#include "GLWrapper.hpp"

struct Shader {
   bool                         valid;
   std::shared_ptr<std::string> infoLog = nullptr;

   GLuint id;
   GLenum type;


   enum class Type : GLenum {
      Vertex = GL_VERTEX_SHADER,
      Fragment = GL_FRAGMENT_SHADER,
      Geometry = GL_GEOMETRY_SHADER,
   };

   Shader(const std::string& source, Shader::Type type);

   inline operator bool() {
      return valid;
   }
};

struct ShaderProgram {
   GLuint                       id;
   std::shared_ptr<std::string> name;

   bool                         valid;
   std::shared_ptr<std::string> infoLog = nullptr;

   template <typename... Args>
   inline ShaderProgram(const Args&... shaders) {
      id = glCreateProgram();
      addShader(shaders...);

      glLinkProgram(id);

      GLint status;
      glGetProgramiv(id, GL_LINK_STATUS, &status);
      valid = (status == GL_TRUE);
      if (!valid) {
         GLsizei size;
         glGetProgramiv(id, GL_INFO_LOG_LENGTH, &size);
         infoLog = std::make_unique<std::string>();
         infoLog->resize(size + 1);


         glGetProgramInfoLog(id, size, nullptr, &(*infoLog)[0]);
         printf("Failed to link program: %s", infoLog->c_str());
      }
   }

   template <typename... Args>
   inline void addShader(const Args&... shaders) {
      for (Shader shader : {shaders...}) {
         if (shader) {
            glAttachShader(id, shader.id);
         } else
            printf("Shader %i was not valid!\n", shader.id);
      }
   }

   inline void use() const {
       glUseProgram(id);
   }
};
