#version 330 core

in vec2 pixelCoord;
in vec2 texCoord;

uniform sampler2D tileset;
uniform sampler2D tilemap;
uniform int num_tiles_x;
uniform int num_tiles_y;
uniform float offset;

out vec4 fragment;

void main() {
  // Which tile to render
  float tile = texture(tilemap, texCoord).r;
  int tileIndex = int(tile * 256);

  // Actual tile texture coords
  int xPos = tileIndex % 16;
  int yPos = tileIndex / 16;
  vec2 origin = vec2(xPos, yPos) / 16.0;
  float xOffset = fract((texCoord.x + offset) * num_tiles_x) / 16.0;
  float yOffset = fract((texCoord.y + offset) * num_tiles_y) / 16.0;
  vec2 tileOffset = vec2(xOffset, yOffset);

  vec4 sample = texture(tileset, origin + tileOffset);
  if (sample.rgb != vec3(1.0, 0.0, 1.0)) {
    fragment = sample;
  } else {
    discard;
  }
}
