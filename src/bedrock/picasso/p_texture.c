#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "p_internal.h"

GLenum TextureTargetToGL[] = {
  0,
  GL_TEXTURE_1D,
  GL_TEXTURE_2D,
};

GLenum TextureChannelToGL[] = {
  0,
  GL_RED,
  0,
  GL_RGB,
  GL_RGBA,
};

GLenum TextureChannelToFormatGL[] = {
  0,
  GL_R8,
  0,
  GL_RGB8,
  GL_RGBA8,
};

PicassoTexture *picasso_texture_create(PicassoTextureTarget target) {
  PicassoTexture *texture = calloc(1, sizeof(PicassoTexture));

  texture->gl.target = TextureTargetToGL[target];

  glCreateTextures(texture->gl.target, 1, &texture->id);
  glTextureParameterf(texture->id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameterf(texture->id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  return texture;
}

void picasso_texture_destroy(PicassoTexture *texture) {
  assert(texture);

  glDeleteTextures(1, &texture->id);
  free(texture);
}

PicassoTextureResult picasso_texture_load(PicassoTexture *texture, PicassoTextureChannels channels, uintmax_t size, const uint8_t *data) {
  assert(texture);

  int w, h;
  uint8_t *imagedata = stbi_load_from_memory(data, size, &w, &h, 0, channels);

  glTextureStorage2D(texture->id, 1, TextureChannelToFormatGL[channels], w, h);
  glTextureSubImage2D(texture->id, 0, 0, 0, w, h, TextureChannelToGL[channels], GL_UNSIGNED_BYTE, imagedata);

  stbi_image_free(imagedata);

  return PICASSO_TEXTURE_OK;
}

void picasso_texture_set_data(PicassoTexture *texture, uintmax_t width, uintmax_t height, PicassoTextureChannels channels, const void *data) {
  assert(texture);

  glTextureStorage2D(texture->id, 1, TextureChannelToFormatGL[channels], width, height);
  glTextureSubImage2D(texture->id, 0, 0, 0, width, height, TextureChannelToGL[channels], GL_UNSIGNED_BYTE, data);
}

void picasso_texture_bind_to(PicassoTexture *texture, uint32_t index) {
  assert(texture);

  glBindTextureUnit(index, texture->id);
}
