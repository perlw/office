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

  glCreateTextures(texture->gl.target, 1, &texture->id);
  if (filtered) {
    glTextureParameteri(texture->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTextureParameteri(texture->id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(texture->id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  glTextureParameteri(texture->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTextureParameteri(texture->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glTextureStorage2D(texture->id, 1, TextureChannelToFormatGL[channels], (GLsizei)texture->width, (GLsizei)texture->height);

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

void picasso_texture_destroy(PicassoTexture *const texture) {
  assert(texture);

  glDeleteTextures(1, &texture->id);
  free(texture);
}

PicassoTextureResult picasso_texture_set_data(PicassoTexture *const texture, uintmax_t offset_x, uintmax_t offset_y, uintmax_t width, uintmax_t height, const void *data) {
  assert(texture);
  if (offset_x + width > texture->width || offset_y + height > texture->height) {
    return PICASSO_TEXTURE_OUT_OF_BOUNDS;
  }

  glTextureSubImage2D(texture->id, 0, (GLint)offset_x, (GLint)offset_y, (GLsizei)width, (GLsizei)height, texture->gl.channels, GL_UNSIGNED_BYTE, data);

  return PICASSO_TEXTURE_OK;
}

void picasso_texture_bind_to(PicassoTexture *const texture, uint32_t index) {
  assert(texture);

  glBindTextureUnit(index, texture->id);
}
