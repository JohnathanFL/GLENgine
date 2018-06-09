#include "Shader.hpp"

Shader::Shader(const std::string& source, Shader::Type type) {
   id         = glCreateShader((GLenum)type);
   this->type = type;

   const char* src = &source[0];
   glShaderSource(id, 1, &src, nullptr);
   glCompileShader(id);

   GLint status;
   glGetShaderiv(id, GL_COMPILE_STATUS, &status);
   valid = (status == GL_TRUE);
   if (!valid) {
      GLsizei size;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &size);

      infoLog->resize(size + 1);

      glGetShaderInfoLog(id, 1, nullptr, &infoLog->at(0));

      printf("%s\n", infoLog->c_str());
   }
}

void ShaderProgram::compile() {
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
