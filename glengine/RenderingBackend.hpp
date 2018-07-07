#pragma once
/*
 * Defines a common base for all rendering backends, as well as common base classes for different objects.
 * The only things anything in here should ever know about are GLM and SDL2, since they're basically the lowest common
 * denom in the engine.
 *
 *
 *
 */

#include <string>

#include <SDL2/SDL.h>

#include <glm/glm.hpp>

#include "Types.hpp"

class RenderingBackend {
  public:
   // Thou shalt not instantiate this class. Derive from it.
   virtual ~RenderingBackend() {}


   virtual void init(const std::string& windowTitle, glm::ivec2 windowDims) = 0;
   SDL_Window*  window;
   glm::ivec2   windowDims;
};


struct GraphicsObject {
   uint32            id;  ///>! A platform and API agnostic ID for the object. Indexes into a vector/array
   RenderingBackend* backend;

   operator bool();  // Is the object valid API-side?
   // TODO: Is there anything else that is needed for all GraphicsObjects?
};

// Each of these will be specialized for each backend

enum class BufferFlags : uint8 {
   Mapped = Bit(0),
   Built  = Bit(1),
};

// This + a number (like 4 for vec4) to specify a data type
enum class DataType { Byte, UByte, Int, UInt, Float };
struct DataDescription {
   DataType type;
   struct Dimensions {
      ubyte x, y;  // i.e {4,1} for a vec4
   } dims;
};

using BufferDescription = std::vector<DataDescription>;

// Most basic types

struct Buffer : GraphicsObject {  // What do we draw/operate on (graphics/compute)
   bool  isMapped();
   void* data();
   void  setDescription(const BufferDescription& desc);
};

struct Mesh : GraphicsObject {};

// Note that the values for each map to Vulkan enums, so will need to be translated for OpenGL
enum class ShaderStage {
   Vertex      = 1,
   TessControl = 2,
   TessEval    = 4,
   Geometry    = 8,
   Fragment    = 16,
   Compute     = 32,
};

struct Shader : GraphicsObject {  // How do we draw it?
   // Maybe unify by embedding glslang?
};

struct ShaderCollection : GraphicsObject {
   // Passed to a graphics pipeline in vulkan, or used in a shader program in GL
};

struct RenderSurface : GraphicsObject {  // Where do we draw it to?
};

// Composites

struct Model : GraphicsObject {
   Mesh             mesh;
   ShaderCollection shaderColl;
};

struct IndexedModel : Model {
   Buffer* iBuff;
};
