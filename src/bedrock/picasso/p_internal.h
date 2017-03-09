#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "glad/glad.h"

#include "occulus/occulus.h"

#include "picasso.h"

struct PicassoShader {
  uint32_t id;
  GLenum type;
};

struct PicassoProgram {
  uint32_t id;
};

typedef enum {
  PICASSO_STATE_PROGRAM = 1,

  PICASSO_STATE_END,
} PicassoState;

uint32_t get_state(PicassoState state);
void set_state(PicassoState state, uint32_t value);
