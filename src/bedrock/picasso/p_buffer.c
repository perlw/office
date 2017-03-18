#include "p_internal.h"

#include "rectify/rectify.h"

GLenum BufferTypeToGL[] = {
  0,
  GL_INT,
  GL_FLOAT,
};

PicassoBufferGroup *picasso_buffergroup_create(void) {
  PicassoBufferGroup *buffergroup = calloc(1, sizeof(PicassoBufferGroup));

  glCreateVertexArrays(1, &buffergroup->id);
  buffergroup->buffers = rectify_array_alloc(1, sizeof(PicassoBuffer*));

  return buffergroup;
}

void picasso_buffergroup_destroy(PicassoBufferGroup *buffergroup) {
  assert(buffergroup);

  for (uintmax_t t = 0; t < rectify_array_size(buffergroup->buffers); t++) {
    buffer_destroy(buffergroup->buffers[t]);
  }
  rectify_array_free(buffergroup->buffers);

  glDeleteVertexArrays(1, &buffergroup->id);
  free(buffergroup);
}

void picasso_buffergroup_draw(PicassoBufferGroup *buffergroup, uintmax_t num_vertices) {
  assert(buffergroup);

  glBindVertexArray(buffergroup->id);
  glDrawArrays(GL_TRIANGLES, 0, num_vertices);
  glBindVertexArray(0);
}

void buffergroup_bind(PicassoBufferGroup *buffergroup) {
  uint32_t id = (buffergroup ? buffergroup->id : 0);
  if (get_state(PICASSO_STATE_VAO) != id) {
    glBindVertexArray(id);
    set_state(PICASSO_STATE_VAO, id);
  }
}


PicassoBuffer *picasso_buffer_create(PicassoBufferGroup *buffergroup) {
  assert(buffergroup);

  PicassoBuffer *buffer = calloc(1, sizeof(PicassoBuffer));

  buffergroup->buffers = rectify_array_push(buffergroup->buffers, &buffer);
  buffer->group = buffergroup;
  buffergroup_bind(buffer->group);

  glCreateBuffers(1, &buffer->id);

  buffergroup_bind(NULL);

  return buffer;
}

void buffer_destroy(PicassoBuffer *buffer) {
  assert(buffer);

  buffergroup_bind(buffer->group);

  glDeleteBuffers(1, &buffer->id);
  free(buffer);

  buffergroup_bind(NULL);
}

void picasso_buffer_set_data(PicassoBuffer *buffer, uintmax_t size, void *data) {
  assert(buffer);

  buffergroup_bind(buffer->group);
  buffer_bind(buffer);

  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

  buffer_bind(NULL);
  buffergroup_bind(NULL);
}

void picasso_buffer_shader_attrib(PicassoBuffer *buffer, int32_t attr_pos, uintmax_t num_fields, int type) {
  assert(buffer);

  buffergroup_bind(buffer->group);
  buffer_bind(buffer);

  glEnableVertexAttribArray(attr_pos);
  glVertexAttribPointer(attr_pos, num_fields, BufferTypeToGL[type], GL_FALSE, 0, NULL);

  buffer_bind(NULL);
  buffergroup_bind(NULL);
}

void buffer_bind(PicassoBuffer *buffer) {
  uint32_t id = (buffer ? buffer->id : 0);

  if (get_state(PICASSO_STATE_BUFFER) != id) {
    glBindBuffer(GL_ARRAY_BUFFER, id);
    set_state(PICASSO_STATE_BUFFER, id);
  }
}
