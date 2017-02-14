#version 330 core

in vec2 vertex;
in vec2 coord;

uniform mat4 mvMatrix;
uniform mat4 pMatrix;

out vec2 texCoord;

void main() {
    texCoord = coord;
    gl_Position = pMatrix * mvMatrix * vec4(vertex, 0.0f, 1.0f);
}
