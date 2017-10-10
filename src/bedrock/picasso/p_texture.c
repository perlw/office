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

PicassoTexture *picasso_texture_create(PicassoTextureTarget target, uintmax_t width, uintmax_t height, PicassoTextureChannels channels, bool filtered) {
  PicassoTexture *texture = calloc(1, sizeof(PicassoTexture));

  texture->width = width;
  texture->height = height;
  texture->gl.target = TextureTargetToGL[target];
  texture->gl.channels = TextureChannelToGL[channels];

  glGenTextures(1, &texture->id);
  texture_bind(texture);
  if (filtered) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glTexImage2D(texture->gl.target, 0, TextureChannelToFormatGL[channels], (GLsizei)texture->width, (GLsizei)texture->height, 0, texture->gl.channels, GL_UNSIGNED_BYTE, NULL);

  return texture;
}

PicassoTexture *picasso_texture_load(PicassoTextureTarget target, PicassoTextureChannels channels, uintmax_t size, const uint8_t *data) {
  int w, h;
  uint8_t *imagedata = stbi_load_from_memory(data, (int)size, &w, &h, 0, channels);
  if (!imagedata) {
    return NULL;
  }

  PicassoTexture *texture = picasso_texture_create(target, w, h, channels, false);
  picasso_texture_set_data(texture, 0, 0, w, h, imagedata);

  stbi_image_free(imagedata);

  return texture;
}

void picasso_texture_destroy(PicassoTexture **texture) {
  assert(texture && *texture);
  glDeleteTextures(1, &(*texture)->id);
  free(*texture);
  *texture = NULL;
}

PicassoTextureResult picasso_texture_set_data(PicassoTexture *const texture, uintmax_t offset_x, uintmax_t offset_y, uintmax_t width, uintmax_t height, const void *data) {
  assert(texture);
  if (offset_x + width > texture->width || offset_y + height > texture->height) {
    return PICASSO_TEXTURE_OUT_OF_BOUNDS;
  }

  texture_bind(texture);
  glTexSubImage2D(texture->gl.target, 0, (GLint)offset_x, (GLint)offset_y, (GLsizei)width, (GLsizei)height, texture->gl.channels, GL_UNSIGNED_BYTE, data);

  return PICASSO_TEXTURE_OK;
}

void picasso_texture_bind_to(PicassoTexture *const texture, uint32_t index) {
  assert(texture);

  glActiveTexture(GL_TEXTURE0 + index);
  texture_bind(texture);
}

void texture_bind(PicassoTexture *const texture) {
  uint32_t id = (texture ? texture->id : 0);

  if (get_state(PICASSO_STATE_TEXTURE) != id) {
    glBindTexture(GL_TEXTURE_2D, id);
    set_state(PICASSO_STATE_TEXTURE, id);
  }
}
