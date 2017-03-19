#version 330 core

in vec2 texCoord;
out vec4 fragment;

uniform sampler2D font_texture;
uniform sampler2D asciimap_texture;

void main() {
  float s = texture(asciimap_texture, texCoord).r;
	fragment = vec4(s, s, s, 1.0);
}
