#include "p_internal.h"

#include "rectify/rectify.h"

GLenum BufferDataTypeToGL[] = {
  0,
  GL_INT,
  GL_FLOAT,
};

GLenum BufferTypeToGL[] = {
  0,
  GL_ARRAY_BUFFER,
  GL_ELEMENT_ARRAY_BUFFER,
};

GLenum BufferUsageToGL[] = {
  0,
  GL_STREAM_DRAW,
  GL_STATIC_DRAW,
  GL_DYNAMIC_DRAW,
};

GLenum BufferModeToGL[] = {
  0,
  GL_LINES,
  GL_TRIANGLES,
};

PicassoBufferGroup *picasso_buffergroup_create(void) {
  PicassoBufferGroup *buffergroup = calloc(1, sizeof(PicassoBufferGroup));

  glCreateVertexArrays(1, &buffergroup->id);
  buffergroup->buffers = rectify_array_alloc(1, sizeof(PicassoBuffer *));

  return buffergroup;
}

void picasso_buffergroup_destroy(PicassoBufferGroup *buffergroup) {
  assert(buffergroup);

  buffergroup_bind(0);

  for (uintmax_t t = 0; t < rectify_array_size(buffergroup->buffers); t++) {
    buffer_destroy(buffergroup->buffers[t]);
  }
  rectify_array_free(buffergroup->buffers);

  glDeleteVertexArrays(1, &buffergroup->id);
  free(buffergroup);
}

void picasso_buffergroup_draw(PicassoBufferGroup *buffergroup, PicassoBufferMode mode, uintmax_t num_vertices) {
  assert(buffergroup);

  buffergroup_bind(buffergroup);
  glDrawArrays(BufferModeToGL[mode], 0, (GLsizei)num_vertices);
}

void buffergroup_bind(PicassoBufferGroup *buffergroup) {
  uint32_t id = (buffergroup ? buffergroup->id : 0);
  if (get_state(PICASSO_STATE_VAO) != id) {
    glBindVertexArray(id);
    set_state(PICASSO_STATE_VAO, id);
  }
}

PicassoBuffer *picasso_buffer_create(PicassoBufferGroup *buffergroup, PicassoBufferType type, PicassoBufferUsage usage) {
  assert(buffergroup);

  PicassoBuffer *buffer = calloc(1, sizeof(PicassoBuffer));

  buffergroup->buffers = rectify_array_push(buffergroup->buffers, &buffer);
  buffer->group = buffergroup;

  glCreateBuffers(1, &buffer->id);
  glVertexArrayVertexBuffer(buffer->group->id, (GLuint)rectify_array_size(buffer->group->buffers), buffer->id, 0, 0);

  buffer->gl.type = BufferTypeToGL[type];
  buffer->gl.usage = BufferUsageToGL[usage];

  return buffer;
}

void buffer_destroy(PicassoBuffer *buffer) {
  assert(buffer);

  glDeleteBuffers(1, &buffer->id);
  free(buffer);
}

void picasso_buffer_set_data(PicassoBuffer *buffer, uintmax_t num_fields, PicassoDataType type, uintmax_t size, void *data) {
  assert(buffer);

  glNamedBufferData(buffer->id, size, data, buffer->gl.usage);
  buffer->num_fields = num_fields;
  buffer->gl.data_type = BufferDataTypeToGL[type];
}

void picasso_buffer_shader_attrib(PicassoBuffer *buffer, int32_t attr_pos) {
  assert(buffer);

  buffergroup_bind(buffer->group);
  buffer_bind(buffer);

  glEnableVertexAttribArray(attr_pos);
  glVertexAttribPointer(attr_pos, (GLint)buffer->num_fields, buffer->gl.data_type, GL_FALSE, 0, NULL);

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
