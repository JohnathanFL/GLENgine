#version 330 core
layout (location=0) in vec3 pos;
layout(location = 1) in vec4 color;

varying vec4 vertColor;

void main() {
    vertColor = color;
	gl_Position = vec4(pos.xyz, 1.0);
}
