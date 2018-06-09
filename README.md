# GLENgine (HEAVILY WIP)
A simple 3D engine over top of OpenGL and SDL2. Currently basically just a wrapper over OpenGL. 

License: MIT


Basic usage: You send a Geometry object and a ShaderProgram to the renderer to draw.
* Geometry object is composed of:
* * A vertex buffer
* * Optionally, an index buffer
* * Some stuff used for drawing (standard glDrawArrays/Elements stuff)
* A ShaderProgram is composed of any number of Shader objects (Including Computes)
* * A Shader object is composed of a shader source and a Shader::Type

The vertex buffer is a GPUBuffer with GPUBuffer::Type::Vertex.
GPUBuffers are basically a thin wrapping over a normal buffer WITH reference counting (based on their OpenGL handles).


All Object::Type (+GPUBuffer::Storage) are thin wrappers over the appropriate GL_ constants.
