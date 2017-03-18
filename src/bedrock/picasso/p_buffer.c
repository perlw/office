#include "p_internal.h"

PicassoBuffer *picasso_buffer_create(void) {
  PicassoBuffer *buffer = calloc(1, sizeof(PicassoBuffer));

  glCreateBuffers(1, &buffer->id);

  return buffer;
}

void picasso_buffer_destroy(PicassoBuffer *buffer) {
  assert(buffer);

  glDeleteBuffers(1, &buffer->id);
  free(buffer);
}

void picasso_buffer_bind(const PicassoBuffer *buffer) {
  uint32_t id = (buffer ? buffer->id : 0);
  if (get_state(PICASSO_STATE_BUFFER) != id) {
    glBindBuffer(GL_ARRAY_BUFFER, id);
    set_state(PICASSO_STATE_BUFFER, id);
  }
}
