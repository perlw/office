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

PicassoTexture *picasso_texture_create(PicassoTextureTarget target) {
  PicassoTexture *texture = calloc(1, sizeof(PicassoTexture));

  texture->gl.target = TextureTargetToGL[target];

  glCreateTextures(texture->gl.target, 1, &texture->id);
  glBindTexture(texture->gl.target, texture->id);
  glTexParameterf(texture->gl.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(texture->gl.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glBindTexture(texture->gl.target, 0);

  return texture;
}

void picasso_texture_destroy(PicassoTexture *texture) {
  assert(texture);

  glDeleteTextures(1, &texture->id);
  free(texture);
}

PicassoTextureResult picasso_texture_load(PicassoTexture *texture, const uint8_t *data, uintmax_t size, PicassoTextureChannels channels) {
  int w, h;
  uint8_t *imagedata = stbi_load_from_memory(data, size, &w, &h, 0, channels);

  texture_bind(texture);
  glTexImage2D(texture->gl.target, 0, TextureChannelToGL[channels], w, h, 0, TextureChannelToGL[channels], GL_UNSIGNED_BYTE, imagedata);

  stbi_image_free(imagedata);

  return PICASSO_TEXTURE_OK;
}

void picasso_texture_bind_to(PicassoTexture *texture, uint32_t index) {
  assert(texture);

  texture->gl.active_texture = index;
  active_texture_bind(index);
  texture_bind(texture);
}

void active_texture_bind(uint32_t id) {
  if (get_state(PICASSO_STATE_TEXTURE) != id) {
    glActiveTexture(GL_TEXTURE0 + id);
    set_state(PICASSO_STATE_TEXTURE, id);
  }
}

// TODO: Should separate bind check between targets, bug prone?
void texture_bind(PicassoTexture *texture) {
  uint32_t id = (texture ? texture->id : 0);

  if (get_state(PICASSO_STATE_TEXTURE) != id) {
    glBindTexture(texture->gl.target, id);
    set_state(PICASSO_STATE_TEXTURE, id);
  }
}
