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

      Logger::Write("Shader::Shader", "Failed to compile shader: ", *infoLog);
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
      Logger::Write("ShaderProgram::compile",
                    "Failed to link program: ", *infoLog);
   }
}

using namespace glm;

// Forgive me father, for I have sinned...mightily
// Before you say anything, just look at how this makes things for the user...
// (prog.setVecUniform(loc, glm::vecx{}) vs prog.setVecxUniform and so on)
#define SET_VEC_UNIFORM_FUNC(CLASS, POSTFIX)                       \
   template <>                                                     \
   void ShaderProgram::setVecUniform<CLASS>(GLint        location, \
                                            const CLASS& uni) {    \
      glProgramUniform##POSTFIX(id, location, 1, &uni[0]);         \
   }

#define SET_ALL_VEC_UNIFORM_FUNC(CLASS, GLM_PREFIX, GL_PREFIX) \
   SET_VEC_UNIFORM_FUNC(GLM_PREFIX##CLASS##1, 1##GL_PREFIX##v) \
   SET_VEC_UNIFORM_FUNC(GLM_PREFIX##CLASS##2, 2##GL_PREFIX##v) \
   SET_VEC_UNIFORM_FUNC(GLM_PREFIX##CLASS##3, 3##GL_PREFIX##v) \
   SET_VEC_UNIFORM_FUNC(GLM_PREFIX##CLASS##4, 4##GL_PREFIX##v)

SET_ALL_VEC_UNIFORM_FUNC(vec, f, f);
SET_ALL_VEC_UNIFORM_FUNC(vec, i, i);
SET_ALL_VEC_UNIFORM_FUNC(vec, u, ui);

#define SET_MAT_UNIFORM_FUNC(CLASS, SIZE)                                     \
   template <>                                                                \
   void ShaderProgram::setMatUniform<CLASS>(GLint location, const CLASS& uni, \
                                            bool transpose) {                 \
      glProgramUniformMatrix##SIZE##fv(id, location, 1, transpose,            \
                                       &uni[0][0]);                           \
   }

#define SET_BOTH_MAT_UNIFORM_FUNC(CLASS, X, Y)   \
   SET_MAT_UNIFORM_FUNC(CLASS##X##x##Y, X##x##Y) \
   SET_MAT_UNIFORM_FUNC(CLASS##Y##x##X, Y##x##X)

#define SET_ALL_MAT_UNIFORM_FUNC(CLASS)   \
   SET_BOTH_MAT_UNIFORM_FUNC(CLASS, 2, 4) \
   SET_BOTH_MAT_UNIFORM_FUNC(CLASS, 2, 3) \
   SET_BOTH_MAT_UNIFORM_FUNC(CLASS, 3, 4) \
   SET_MAT_UNIFORM_FUNC(CLASS##4, 4)      \
   SET_MAT_UNIFORM_FUNC(CLASS##3, 3)      \
   SET_MAT_UNIFORM_FUNC(CLASS##2, 2)

SET_ALL_MAT_UNIFORM_FUNC(mat);
