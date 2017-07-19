#include "p_internal.h"

PicassoFramebuffer *picasso_framebuffer_create(void) {
  PicassoFramebuffer *buffer = calloc(1, sizeof(PicassoFramebuffer));
  glGenFramebuffers(1, &buffer->id);
  return buffer;
}

void picasso_framebuffer_destroy(PicassoFramebuffer *const buffer) {
  assert(buffer);
  glDeleteFramebuffers(1, &buffer->id);
}

void picasso_framebuffer_bind(PicassoFramebuffer *const buffer) {
  assert(buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, buffer->id);
}

void picasso_framebuffer_unbind(PicassoFramebuffer *const buffer) {
  assert(buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
