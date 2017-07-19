#version 330 core

in vec2 texCoord;
out vec4 fragment;

uniform sampler2D fbo_texture;
uniform ivec2 resolution;

vec4 sharpen(in sampler2D tex, in vec2 coords, in vec2 renderSize) {
  float dx = 1.0 / renderSize.x;
  float dy = 1.0 / renderSize.y;
  vec4 sum = vec4(0.0);
  sum += -1. * texture2D(tex, coords + vec2( -1.0 * dx , 0.0 * dy));
  sum += -1. * texture2D(tex, coords + vec2( 0.0 * dx , -1.0 * dy));
  sum += 5. * texture2D(tex, coords + vec2( 0.0 * dx , 0.0 * dy));
  sum += -1. * texture2D(tex, coords + vec2( 0.0 * dx , 1.0 * dy));
  sum += -1. * texture2D(tex, coords + vec2( 1.0 * dx , 0.0 * dy));
  return sum;
}

void main() {
  //if (texCoord.s < 0.33) {
    fragment = texture(fbo_texture, texCoord);
  /*} else if (texCoord.s < 0.50) {
    fragment = sharpen(fbo_texture, texCoord, vec2(1920, 1080));
  } else if (texCoord.s < 0.66) {
    fragment = sharpen(fbo_texture, texCoord, vec2(res_width, res_height));
  } else {
    fragment = sharpen(fbo_texture, texCoord, resolution);
  }*/
}
