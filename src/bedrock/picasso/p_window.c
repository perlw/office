#include <string.h>

#include "p_internal.h"

#include "GLFW/glfw3.h"

GLFWwindow *current_context = NULL;

void dummy_keyboard_callback(const PicassoWindowKeyboardEvent *event) {
  printf("No keyboard callback..\n");
};
void dummy_mouse_move_callback(const PicassoWindowMouseEvent *event){
  //printf("Mouse move: %.2f, %.2f\n", event->x, event->y);
};
void dummy_mouse_button_callback(const PicassoWindowMouseEvent *event){
  //printf("Mouse button: %d %.2f, %.2f %d %d\n", event->button, event->x, event->y, event->pressed, event->released);
};
void dummy_mouse_scroll_callback(const PicassoWindowMouseScrollEvent *event){
  //printf("Mouse scroll: %.2f, %.2f\n", event->offset_x, event->offset_y);
};

void keyboard_callback(GLFWwindow *raw_window, int key, int scancode, int action, int mods) {
  if (action == GLFW_REPEAT) {
    return;
  }

  PicassoWindow *window = glfwGetWindowUserPointer(raw_window);
  window->keyboard_callback(&(PicassoWindowKeyboardEvent){
    .key = key,
    .scancode = scancode,
    .pressed = (action == GLFW_PRESS),
    .released = (action == GLFW_RELEASE),
    .shift = (mods & GLFW_MOD_SHIFT),
  });
}

void mouse_move_callback(GLFWwindow *raw_window, double xpos, double ypos) {
  PicassoWindow *window = glfwGetWindowUserPointer(raw_window);
  window->mouse_move_callback(&(PicassoWindowMouseEvent){
    .button = -1,
    .x = xpos,
    .y = ypos,
    .pressed = 0,
    .released = 0,
  });
}

void mouse_button_callback(GLFWwindow *raw_window, int button, int action, int mods) {
  PicassoWindow *window = glfwGetWindowUserPointer(raw_window);
  double x, y;
  glfwGetCursorPos(raw_window, &x, &y);
  window->mouse_button_callback(&(PicassoWindowMouseEvent){
    .button = button,
    .x = x,
    .y = y,
    .pressed = (action == GLFW_PRESS),
    .released = (action == GLFW_RELEASE),
  });
}

void mouse_scroll_callback(GLFWwindow *raw_window, double xoffset, double yoffset) {
  PicassoWindow *window = glfwGetWindowUserPointer(raw_window);
  double x, y;
  glfwGetCursorPos(raw_window, &x, &y);
  window->mouse_scroll_callback(&(PicassoWindowMouseScrollEvent){
    .offset_x = xoffset,
    .offset_y = yoffset,
    .x = x,
    .y = y,
  });
}

// TODO: Settable callback
void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *user_param) {
  printf("PICASSO: GL %s\n", message);
}

PicassoWindowResult picasso_window_init(void) {
  if (!glfwInit()) {
    return PICASSO_WINDOW_INIT_FAIL;
  }

  return PICASSO_WINDOW_OK;
}

void picasso_window_kill(void) {
  glfwTerminate();
}

void picasso_window_update(void) {
  glfwPollEvents();
}

PicassoWindow *picasso_window_create(const char *title, PicassoWindowInit *const window_init) {
  assert(window_init);

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, (window_init->gl_debug ? GL_TRUE : GL_FALSE));

  if (window_init->fullscreen) {
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *vid_mode = glfwGetVideoMode(monitor);
    window_init->width = vid_mode->width;
    window_init->height = vid_mode->height;
  }

  GLFWwindow *raw_window = glfwCreateWindow(window_init->width, window_init->height, title, NULL, NULL);
  if (!raw_window) {
    return NULL;
  }

  glfwSetKeyCallback(raw_window, keyboard_callback);
  glfwSetCursorPosCallback(raw_window, mouse_move_callback);
  glfwSetMouseButtonCallback(raw_window, mouse_button_callback);
  glfwSetScrollCallback(raw_window, mouse_scroll_callback);

  PicassoWindow *window = calloc(1, sizeof(PicassoWindow));
  *window = (PicassoWindow){
    .raw_ptr = raw_window,
    .keyboard_callback = &dummy_keyboard_callback,
    .mouse_move_callback = &dummy_mouse_move_callback,
    .mouse_button_callback = &dummy_mouse_button_callback,
    .mouse_scroll_callback = &dummy_mouse_scroll_callback,
  };

  glfwSetWindowUserPointer(raw_window, window);
  glfwMakeContextCurrent(raw_window);
  glfwSwapInterval(0);
  current_context = raw_window;

  /* OpenGL */
  if (!gladLoadGL()) {
    return NULL;
  }

  int major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  printf("GL %d.%d\n", major, minor);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glClearDepth(1);
  glDepthFunc(GL_LESS);
  glViewport(0, 0, window_init->width, window_init->height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  if (window_init->gl_debug) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    glDebugMessageCallback((GLDEBUGPROC)debug_callback, NULL);
  }

  return window;
}

void picasso_window_destroy(PicassoWindow **window) {
  assert(window && *window);
  glfwDestroyWindow((*window)->raw_ptr);
  free(*window);
  *window = NULL;
}

void picasso_window_clear(PicassoWindow *window) {
  assert(window);
  picasso_window_make_context_current(window);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void picasso_window_clearcolor(PicassoWindow *window, float r, float g, float b, float a) {
  assert(window);
  picasso_window_make_context_current(window);

  glClearColor(r, g, b, a);
}

void picasso_window_swap(PicassoWindow *window) {
  assert(window);
  glfwSwapBuffers(window->raw_ptr);
}

bool picasso_window_should_close(PicassoWindow *window) {
  assert(window);
  return glfwWindowShouldClose(window->raw_ptr);
}

void picasso_window_make_context_current(PicassoWindow *window) {
  assert(window);
  if (window->raw_ptr != current_context) {
    glfwMakeContextCurrent(window->raw_ptr);
    current_context = window->raw_ptr;
  }
}

void picasso_window_keyboard_callback(PicassoWindow *window, PicassoWindowKeyboardCallback callback) {
  assert(callback);
  assert(window);

  window->keyboard_callback = callback;
}

void picasso_window_mouse_move_callback(PicassoWindow *window, PicassoWindowMouseCallback callback) {
  assert(callback);
  assert(window);

  window->mouse_move_callback = callback;
}

void picasso_window_mouse_button_callback(PicassoWindow *window, PicassoWindowMouseCallback callback) {
  assert(callback);
  assert(window);

  window->mouse_button_callback = callback;
}

void picasso_window_mouse_scroll_callback(PicassoWindow *window, PicassoWindowMouseScrollCallback callback) {
  assert(callback);
  assert(window);

  window->mouse_scroll_callback = callback;
}

void picasso_window_viewport(PicassoWindow *window, int32_t x, int32_t y, int32_t width, int32_t height) {
  assert(window);
  picasso_window_make_context_current(window);

  glViewport(x, y, width, height);
}
