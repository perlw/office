#version 330 core

in vec2 texCoord;
out vec4 fragment;

uniform sampler2D fbo_texture;

void main() {
	fragment = texture(fbo_texture, texCoord);
}
