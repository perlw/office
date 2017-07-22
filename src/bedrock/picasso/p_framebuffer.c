#include "p_internal.h"

PicassoFramebuffer *picasso_framebuffer_create(void) {
  PicassoFramebuffer *buffer = calloc(1, sizeof(PicassoFramebuffer));
  glGenFramebuffers(1, &buffer->id);
  return buffer;
}

void picasso_framebuffer_destroy(PicassoFramebuffer *const buffer) {
  assert(buffer);
  glDeleteFramebuffers(1, &buffer->id);
  free(buffer);
}

PicassoFramebufferResult picasso_framebuffer_attach_texture(PicassoFramebuffer *const buffer, PicassoTexture *const texture) {
  assert(buffer);
  assert(texture);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->id, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    return PICASSO_FRAMEBUFFER_INCOMPLETE;
  }

  return PICASSO_FRAMEBUFFER_OK;
}

void picasso_framebuffer_bind(PicassoFramebuffer *const buffer) {
  assert(buffer);
  if (get_state(PICASSO_STATE_FRAMEBUFFER) != buffer->id) {
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->id);
    set_state(PICASSO_STATE_FRAMEBUFFER, buffer->id);
  }
}

void picasso_framebuffer_unbind(PicassoFramebuffer *const buffer) {
  assert(buffer);
  if (get_state(PICASSO_STATE_FRAMEBUFFER) != 0) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    set_state(PICASSO_STATE_FRAMEBUFFER, 0);
  }
}
