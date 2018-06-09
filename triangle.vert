#version 330 core
layout (location=0) in vec3 pos;

varying vec3 fromVert;

void main() {
  fromVert = pos;
	gl_Position = vec4(pos.xyz, 1.0);
}
