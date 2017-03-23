#version 330 core

in vec2 texCoord;

uniform sampler2D font_texture;
uniform sampler2D asciimap_texture;

out vec4 fragment;

void main() {
  // Which tile to render
  vec2 coord = vec2(texCoord.s, texCoord.t);
  vec4 tile = texture(asciimap_texture, coord);
  int tileIndex = int(floor(tile.r * 255.0));
  if (tileIndex == 0) {
    discard;
  }
  float tileFore = tile.g;
  float tileBack = tile.b;

  // Actual tile texture coords
  vec2 index = vec2(mod(tileIndex, 16), tileIndex / 16);
  vec2 origin = floor(index) / 16.0;
  vec2 tileCoord = mod(vec2(texCoord.s * 80, texCoord.t * 60), 1) / 16.0;

  vec4 sample = texture(font_texture, origin + tileCoord);
  if (sample == vec4(1, 1, 1, 1)) {
    fragment = vec4(tileFore, tileFore, tileFore, 1);
  } else {
    fragment = vec4(tileBack, tileBack, tileBack, 1);
  }
}
