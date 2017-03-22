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

PicassoTexture *picasso_texture_create(PicassoTextureTarget target, uintmax_t width, uintmax_t height, PicassoTextureChannels channels) {
  PicassoTexture *texture = calloc(1, sizeof(PicassoTexture));

  texture->width = width;
  texture->height = height;
  texture->gl.target = TextureTargetToGL[target];
  texture->gl.channels = TextureChannelToGL[channels];

  glCreateTextures(texture->gl.target, 1, &texture->id);
  glTextureParameterf(texture->id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameterf(texture->id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTextureStorage2D(texture->id, 1, TextureChannelToFormatGL[channels], texture->width, texture->height);

  return texture;
}

PicassoTexture *picasso_texture_load(PicassoTextureTarget target, PicassoTextureChannels channels, uintmax_t size, const uint8_t *data) {
  int w, h;
  uint8_t *imagedata = stbi_load_from_memory(data, size, &w, &h, 0, channels);
  if (!imagedata) {
    return NULL;
  }

  PicassoTexture *texture = picasso_texture_create(target, w, h, channels);
  picasso_texture_set_data(texture, 0, 0, w, h, imagedata);

  stbi_image_free(imagedata);

  return texture;
}

void picasso_texture_destroy(PicassoTexture *texture) {
  assert(texture);

  glDeleteTextures(1, &texture->id);
  free(texture);
}

PicassoTextureResult picasso_texture_set_data(PicassoTexture *texture, uintmax_t offset_x, uintmax_t offset_y, uintmax_t width, uintmax_t height, const void *data) {
  assert(texture);
  if (offset_x + width > texture->width || offset_y + height > texture->height) {
    return PICASSO_TEXTURE_OUT_OF_BOUNDS;
  }

  glTextureSubImage2D(texture->id, 0, offset_x, offset_y, width, height, texture->gl.channels, GL_UNSIGNED_BYTE, data);

  return PICASSO_TEXTURE_OK;
}

void picasso_texture_bind_to(PicassoTexture *texture, uint32_t index) {
  assert(texture);

  glBindTextureUnit(index, texture->id);
}