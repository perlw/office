#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "glad/glad.h"

#include "GLFW/glfw3.h"

#include "occulus/occulus.h"
#include "rectify/rectify.h"

#include "picasso.h"

struct PicassoWindow {
  GLFWwindow *raw_ptr;
  PicassoWindowKeyboardCallback keyboard_callback;
  PicassoWindowMouseCallback mouse_move_callback;
  PicassoWindowMouseCallback mouse_button_callback;
  PicassoWindowMouseScrollCallback mouse_scroll_callback;
};

struct PicassoShader {
  uint32_t id;
  GLenum type;
};

struct PicassoProgram {
  uint32_t id;
};

struct PicassoBuffer {
  uint32_t id;
  uintmax_t num_fields;

  PicassoBufferGroup *group;

  struct {
    GLenum type;
    GLenum usage;
    GLenum data_type;
  } gl;
};

struct PicassoBufferGroup {
  uint32_t id;

  PicassoBuffer **buffers;
};

struct PicassoFramebuffer {
  uint32_t id;
};

struct PicassoTexture {
  uint32_t id;
  uintmax_t width;
  uintmax_t height;

  struct {
    GLenum target;
    GLenum channels;
    uint32_t active_texture;
  } gl;
};

typedef enum {
  PICASSO_STATE_PROGRAM = 1,
  PICASSO_STATE_BUFFER,
  PICASSO_STATE_FRAMEBUFFER,
  PICASSO_STATE_VAO,
  PICASSO_STATE_ACTIVE_TEXTURE,
  PICASSO_STATE_TEXTURE,

  PICASSO_STATE_END,
} PicassoState;

uint32_t get_state(PicassoState state);
void set_state(PicassoState state, uint32_t value);

void buffergroup_bind(PicassoBufferGroup *const buffergroup);
void buffer_destroy(PicassoBuffer **buffer);
void buffer_bind(PicassoBuffer *const buffer);
void texture_bind(PicassoTexture *const texture);
