#pragma once
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

#include "GLWrapper.hpp"

//==============================================================================
//========================  Useful Macros  =====================================
//==============================================================================


// Allows the use of things like VertexAttribute.withNormalized.withType, etc
// instead of a long constructor.
#define FACTORY_METHOD(CLASS, TYPE, NAME, VARIABLE) \
   inline CLASS& with##NAME(TYPE VARIABLE) {        \
      this->VARIABLE = VARIABLE;                    \
      return *this;                                 \
   }

template <typename T>
inline constexpr size_t vec_sizeof(const std::vector<T>& vec) {
   return vec.size() * sizeof(T);
}

//==============================================================================
//========================  VertexAttribute  ===================================
//==============================================================================

struct VertexAttribute {
   inline VertexAttribute();

   // Comments are from official OpenGL reference

   // Specifies the index of the generic vertex attribute to be modified.
   // size
   GLuint index;

   // Specifies the number of components per generic vertex attribute. Must be
   // 1, 2, 3, 4. Additionally, the symbolic constant GL_BGRA is accepted by
   // glVertexAttribPointer. The initial value is 4. type
   GLint size;

   // Specifies the data type of each component in the array. The symbolic
   // constants GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT,
   // and GL_UNSIGNED_INT are accepted by glVertexAttribPointer and
   // glVertexAttribIPointer. Additionally GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE,
   // GL_FIXED, GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_2_10_10_10_REV and
   // GL_UNSIGNED_INT_10F_11F_11F_REV are accepted by glVertexAttribPointer.
   // GL_DOUBLE is also accepted by glVertexAttribLPointer and is the only token
   // accepted by the type parameter for that function. The initial value is
   // GL_FLOAT. normalized
   enum class Type : GLenum {
      Invalid       = GL_INVALID_ENUM,
      Byte          = GL_BYTE,
      UnsignedByte  = GL_UNSIGNED_BYTE,
      Short         = GL_SHORT,
      UnsignedShort = GL_UNSIGNED_SHORT,
      Int           = GL_INT,
      UnsignedInt   = GL_UNSIGNED_INT,
      HalfFloat     = GL_HALF_FLOAT,
      Float         = GL_FLOAT,
      Double        = GL_DOUBLE,
   } type;

   // For glVertexAttribPointer, specifies whether fixed-point data values
   // should be normalized (GL_TRUE) or converted directly as fixed-point values
   // (GL_FALSE) when they are accessed. stride
   bool normalized;

   // Specifies the byte offset between consecutive generic vertex attributes.
   // If stride is 0, the generic vertex attributes are understood to be tightly
   // packed in the array. The initial value is 0. pointer
   GLsizei stride;

   // Specifies a offset of the first component of the first generic vertex
   // attribute in the array in the data store of the buffer currently bound to
   // the GL_ARRAY_BUFFER target. The initial value is 0.
   const GLvoid* initialOffset;

   inline void bind();

   FACTORY_METHOD(VertexAttribute, GLuint, Index, index)
   FACTORY_METHOD(VertexAttribute, GLint, Size, size)
   FACTORY_METHOD(VertexAttribute, Type, Type, type)
   FACTORY_METHOD(VertexAttribute, GLboolean, Normalized, normalized)
   FACTORY_METHOD(VertexAttribute, GLsizei, Stride, stride)
   FACTORY_METHOD(VertexAttribute, const GLvoid*, InitialOffset, initialOffset)
};

VertexAttribute::VertexAttribute() {
   initialOffset = nullptr;
   index = size = normalized = (stride = 0);
   type = Type::Invalid;  // Make sure the user sets this manually.
}

void VertexAttribute::bind() {
   glVertexAttribPointer(index, size, (GLenum)type, normalized, stride,
                         initialOffset);
   glEnableVertexAttribArray(index);
}


//==============================================================================
//========================  GPUBuffer  =========================================
//==============================================================================

// TODO: Make this track usages of itself and delete when ALL are gone.
struct GPUBuffer {
   enum class Type : GLenum;
   enum class Storage : GLenum;

   // Only provided for things like optional that need a default constructor.
   // Constructed buffer is NOT valid to use.
   GPUBuffer();

   GPUBuffer(GPUBuffer&& buf) {
      this->id      = buf.id;
      this->type    = buf.type;
      this->storage = buf.storage;

      refCounts[id]++;
   }
   GPUBuffer(const GPUBuffer& buf) {
      this->id      = buf.id;
      this->type    = buf.type;
      this->storage = buf.storage;

      refCounts[id]++;
   }

   GPUBuffer(Type type, Storage storage);

   ~GPUBuffer() {
      auto& count = refCounts[id];
      if (--count < 1)
         glDeleteBuffers(1, &id);
   }

   // Example: buff.upload(&vertVector[0], vec_sizeof(vertVector));
   inline void upload(const void* data, size_t size);

   // Likely rarely used, as the VAO stores this.
   inline void bind() const;

   // What kind of data are we storing?
   enum class Type : GLenum {
      Invalid = GL_INVALID_ENUM,
      Vertex  = GL_ARRAY_BUFFER,
      Index   = GL_ELEMENT_ARRAY_BUFFER,
      Uniform = GL_UNIFORM_BUFFER,  // TODO: Special handling in other stuff.
   } type;

   // What kind of memory should we use?
   enum class Storage : GLenum {
      Invalid     = GL_INVALID_ENUM,
      StaticDraw  = GL_STATIC_DRAW,
      DynamicDraw = GL_DYNAMIC_DRAW,
      StreamDraw  = GL_STREAM_DRAW,
   } storage;

   GLuint id;


   // So we can have a destructor, but also prevent deleting something in use.
   static std::unordered_map<GLuint, unsigned> refCounts;  // Indexed by id
};

void GPUBuffer::upload(const void* data, size_t size) {
   glBindBuffer((GLenum)type, id);
   glBufferData((GLenum)type, size, data, (GLenum)storage);
}

void GPUBuffer::bind() const { glBindBuffer((GLenum)type, id); }

//==============================================================================
//========================  Geometry  ==========================================
//==============================================================================

struct Geometry {
   enum class DrawType : GLenum;

   // Note: If no ibuff is desired, simply pass '{}'
   Geometry(std::vector<VertexAttribute> attribs, DrawType drawType,
            GLint offset, GLsizei numToDraw, GPUBuffer vbuff,
            std::optional<GPUBuffer> ibuff);
   Geometry(Geometry&& geom);

   Geometry(const Geometry& geom);

   ~Geometry();

   std::vector<VertexAttribute> attribs;
   enum class DrawType : GLenum {
      Points                 = GL_POINTS,
      Strip                  = GL_LINE_STRIP,
      LineLoop               = GL_LINE_LOOP,
      Lines                  = GL_LINES,
      LineStripAdjacency     = GL_LINE_STRIP_ADJACENCY,
      LinesAdjacency         = GL_LINES_ADJACENCY,
      TriangleStrip          = GL_TRIANGLE_STRIP,
      TriangleFan            = GL_TRIANGLE_FAN,
      Triangles              = GL_TRIANGLES,
      TriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
      TrianglesAdjacency     = GL_TRIANGLES_ADJACENCY,
      Patches                = GL_PATCHES
   } drawType;
   GLint   offset;
   GLsizei numToDraw;

   GLuint    vao;
   GPUBuffer vbuff;

   std::optional<GPUBuffer> ibuff;

   inline void draw() const;

   // So we can have a destructor, but also prevent deleting something in use.
   // We could technically use a shared_ptr, but that doesn't keep locality.
   static std::unordered_map<GLuint, unsigned> refCounts;  // Indexed by id
};

void Geometry::draw() const {
   glBindVertexArray(vao);

   if (ibuff.has_value())
      glDrawElements((GLenum)drawType, numToDraw, GL_UNSIGNED_INT, 0);

   else
      glDrawArrays((GLenum)drawType, offset, numToDraw);
}
