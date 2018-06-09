#include "Geometry.hpp"

std::unordered_map<GLuint, unsigned> GPUBuffer::refCounts;
std::unordered_map<GLuint, unsigned> Geometry::refCounts;  // Indexed by id

Geometry::Geometry(std::vector<VertexAttribute> attribs,
                   Geometry::DrawType drawType, GLint offset, GLsizei numToDraw,
                   GPUBuffer vbuff, std::optional<GPUBuffer> ibuff)
    : vbuff{vbuff}, ibuff{ibuff} {
   this->attribs = attribs;

   this->drawType  = drawType;
   this->offset    = offset;
   this->numToDraw = numToDraw;

   glGenVertexArrays(1, &this->vao);
   glBindVertexArray(this->vao);

   vbuff.bind();
   if (ibuff)
      ibuff.value().bind();

   for (auto& attrib : attribs)
      attrib.bind();

   refCounts[vao]++;
}

Geometry::Geometry(Geometry&& geom)
    : drawType{geom.drawType},
      attribs{geom.attribs},
      offset{geom.offset},
      numToDraw{geom.numToDraw},
      vao{geom.vao},
      vbuff{geom.vbuff},
      ibuff{geom.ibuff} {
   refCounts[vao]++;
}

Geometry::Geometry(const Geometry& geom)
    : drawType{geom.drawType},
      attribs{geom.attribs},
      offset{geom.offset},
      numToDraw{geom.numToDraw},
      vao{geom.vao},
      vbuff{geom.vbuff},
      ibuff{geom.ibuff} {
   refCounts[vao]++;
}

Geometry::~Geometry() {
   auto& count = refCounts[vao];
   if (--count < 1) {
      glDeleteVertexArrays(1, &vao);
      printf("Deleting VAO!\n");
   }
}

GPUBuffer::GPUBuffer() {
   id      = 0;
   type    = Type::Invalid;
   storage = Storage::Invalid;
}

GPUBuffer::GPUBuffer(GPUBuffer::Type type, GPUBuffer::Storage storage) {
   this->type    = type;
   this->storage = storage;
   glGenBuffers(1, &id);

   refCounts[id]++;
}
