#version 330 core

in vec2 pixelCoord;
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
  float tile = texture(asciimap_texture, texCoord).r;
  int tileIndex = int(tile * 256);
  vec4 tileFore = texture(forecolors_texture, texCoord);
  vec4 tileBack = texture(backcolors_texture, texCoord);

  // Actual tile texture coords
  int xPos = tileIndex % 16;
  int yPos = tileIndex / 16;
  vec2 origin = vec2(xPos, yPos) / 16.0;
  float xOffset = fract(texCoord.x * ascii_res_width) / 16.0;
  float yOffset = fract(texCoord.y * ascii_res_height) / 16.0;
  vec2 tileOffset = vec2(xOffset, yOffset);

  vec4 sample = texture(font_texture, origin + tileOffset);
  if (sample.r == sample.g && sample.g == sample.b) {
    fragment = sample * tileFore;
  } else if (tileBack.r == 1.0 && tileBack.g == 0.0 && tileBack.b == 1.0) {
    discard;
  } else {
    fragment = tileBack;
  }
}
