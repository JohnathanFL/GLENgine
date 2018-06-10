# GLENgine (HEAVILY WIP)
A simple 3D engine over top of OpenGL and SDL2. Currently basically just a wrapper over OpenGL. 

License: MIT


Overview:
* ./glengine/: The actual engine component of the repository
* ./src/: Source of an app I'm building with GLEN, because I'd like to eat my own dogfood.
* ./subprojects/glad/: [glad](https://github.com/Dav1dde/glad) generated for opengl 4.5 and all extensions.
* ./triangle.\*: Shaders for a basic triangle program that currently work with GLEN.
* ./meson.build: Meson build defs. I might add a secondary CMake script eventually.
* ./GLENgine.log: The most recent log from running the test program. I leave it in git to give an idea of what the logs look like.

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
