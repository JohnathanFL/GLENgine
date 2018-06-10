#pragma once
#include <iostream>
#include <memory>
#include <string>

#include "GLWrapper.hpp"
#include "Logger.hpp"

#include <glm/glm.hpp>


struct Shader {
   enum class Type : GLenum;

   Shader(const std::string& source, Shader::Type type);

   inline operator bool() { return valid; }

   enum class Type : GLenum {
      Vertex   = GL_VERTEX_SHADER,
      Fragment = GL_FRAGMENT_SHADER,
      Geometry = GL_GEOMETRY_SHADER,
      Compute  = GL_COMPUTE_SHADER,
   } type;

   bool                         valid;
   std::shared_ptr<std::string> infoLog = nullptr;

   GLuint id;
};

struct ShaderProgram {
   template <typename... Args>
   inline ShaderProgram(const Args&... shaders);

   template <typename... Args>
   inline void addShaders(const Args&... shaders);

   void compile();

   inline bool use() const;
   inline void dispatchCompute(const glm::uvec3& groupSize);

   // This must be specialized elsewhere.
   template <typename T>
   void setVecUniform(GLint location, const T& uni) const;

   template <typename T>
   void setMatUniform(GLint location, const T& uni, bool transpose) const;


   GLuint                       id;
   std::shared_ptr<std::string> name;

   bool                         valid;
   std::shared_ptr<std::string> infoLog = nullptr;
};


template <typename... Args>
inline ShaderProgram::ShaderProgram(const Args&... shaders) {
   id = glCreateProgram();
   addShaders(shaders...);
   compile();
}

template <typename... Args>
inline void ShaderProgram::addShaders(const Args&... shaders) {
   for (Shader shader : {shaders...}) {
      if (shader) {
         glAttachShader(id, shader.id);
      } else
         Logger::Write("ShaderProgram::addShaders",
                       "Invalid shader passed to program ", id, "!");
   }
}


bool ShaderProgram::use() const {
   if (GLState::Current().program != id) {
      glUseProgram(id);
      GLState::Current().program = id;
      return true;
   } else
      return false;
}
void ShaderProgram::dispatchCompute(const glm::uvec3& groupSize) {
   glDispatchCompute(groupSize.x, groupSize.y, groupSize.z);
}
