#version 330 core

in vec2 texCoord;
out vec4 fragment;

uniform sampler2D image;

void main() {
	fragment = texture(image, texCoord);
}
