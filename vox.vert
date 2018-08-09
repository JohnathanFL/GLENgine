#version 450


layout(location = 0) in vec3 position;
layout(location = 1) in 

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}
