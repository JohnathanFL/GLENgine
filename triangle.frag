#version 430 core
layout(location=0) out vec4 FragColor;

varying vec4 vertColor;

void main() {
        FragColor = vertColor;

}
