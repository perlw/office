#version 330 core

in vec2 texCoord;

uniform sampler2D font_texture;
uniform sampler2D asciimap_texture;
uniform sampler2D forecolors_texture;
uniform sampler2D backcolors_texture;
uniform int ascii_res_width;
uniform int ascii_res_height;

out vec4 fragment;

void main() {
  // Which tile to render
  vec2 coord = vec2(texCoord.s, texCoord.t);
  float tile = texture(asciimap_texture, coord).r;
  int tileIndex = int(floor(tile * 255.0));
  if (tileIndex == 0) {
    discard;
  }
  vec4 tileFore = texture(forecolors_texture, coord);
  vec4 tileBack = texture(backcolors_texture, coord);

  // Actual tile texture coords
  vec2 index = vec2(mod(tileIndex, 16), tileIndex / 16);
  vec2 origin = floor(index) / 16.0;
  vec2 tileCoord = mod(vec2(texCoord.s * ascii_res_width, texCoord.t * ascii_res_height), 1) / 15.5;

  vec4 sample = texture(font_texture, origin + tileCoord);
  if (sample == vec4(1, 1, 1, 1)) {
    fragment = tileFore;
  } else {
    fragment = tileBack;
  }
}
