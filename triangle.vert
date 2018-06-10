#version 430 core
layout (location=0) in vec3 pos;
layout(location = 1) in vec4 color;

layout(location = 0) uniform vec3 uni;

varying vec4 vertColor;

void main() {
    vertColor = color;
        gl_Position = vec4(pos.xyz + uni, 1.0);
}
