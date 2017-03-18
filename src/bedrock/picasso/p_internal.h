#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "glad/glad.h"

#include "occulus/occulus.h"

#include "picasso.h"

typedef enum {
  PICASSO_STATE_PROGRAM = 1,
  PICASSO_STATE_BUFFER,
  PICASSO_STATE_VAO,

  PICASSO_STATE_END,
} PicassoState;

struct PicassoShader {
  uint32_t id;
  GLenum type;
};

struct PicassoProgram {
  uint32_t id;
};

struct PicassoBuffer {
  uint32_t id;

  PicassoBufferGroup *group;
};

struct PicassoBufferGroup {
  uint32_t id;

  PicassoBuffer **buffers;
};

uint32_t get_state(PicassoState state);
void set_state(PicassoState state, uint32_t value);

void buffergroup_bind(PicassoBufferGroup *buffergroup);
void buffer_destroy(PicassoBuffer *buffer);
void buffer_bind(PicassoBuffer *buffer);
