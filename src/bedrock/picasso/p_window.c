#include <string.h>

#include "p_internal.h"

#include "GLFW/glfw3.h"

GLFWwindow *window = NULL;

void dummy_keyboard_callback(const PicassoWindowKeyboardEvent *event) {
  printf("No keyboard callback..\n");
};
void dummy_mouse_move_callback(const PicassoWindowMouseEvent *event){
  //printf("Mouse move: %.2f, %.2f\n", event->x, event->y);
};
void dummy_mouse_button_callback(const PicassoWindowMouseEvent *event){
  //printf("Mouse button: %d %.2f, %.2f %d %d\n", event->button, event->x, event->y, event->pressed, event->released);
};
PicassoWindowKeyboardCallback picasso_keyboard_callback = &dummy_keyboard_callback;
PicassoWindowMouseCallback picasso_mouse_move_callback = &dummy_mouse_move_callback;
PicassoWindowMouseCallback picasso_mouse_button_callback = &dummy_mouse_button_callback;

void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (action == GLFW_REPEAT) {
    return;
  }

  picasso_keyboard_callback(&(PicassoWindowKeyboardEvent){
    .key = key,
    .scancode = scancode,
    .pressed = (action == GLFW_PRESS),
    .released = (action == GLFW_RELEASE),
    .shift = (mods & GLFW_MOD_SHIFT),
  });
}

void mouse_move_callback(GLFWwindow *window, double xpos, double ypos) {
  picasso_mouse_move_callback(&(PicassoWindowMouseEvent){
    .button = -1,
    .x = xpos,
    .y = ypos,
    .pressed = 0,
    .released = 0,
  });
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  picasso_mouse_button_callback(&(PicassoWindowMouseEvent){
    .button = button,
    .x = x,
    .y = y,
    .pressed = (action == GLFW_PRESS),
    .released = (action == GLFW_RELEASE),
  });
}

void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *user_param) {
  printf("PICASSO: GL %s\n", message);
}

PicassoWindowResult picasso_window_init(const char *title, uint32_t res_width, uint32_t res_height, bool fullscreen, bool gl_debug) {
  if (!glfwInit()) {
    return PICASSO_WINDOW_INIT_FAIL;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, (gl_debug ? GL_TRUE : GL_FALSE));

  if (fullscreen) {
    // Borderless fullscreen
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    /*GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *vid_mode = glfwGetVideoMode(monitor);*/
    // Borderless fullscreen
  }

  window = glfwCreateWindow(res_width, res_height, title, NULL, NULL);
  if (!window) {
    glfwTerminate();
    return PICASSO_WINDOW_CREATION_FAIL;
  }

  glfwSetKeyCallback(window, keyboard_callback);
  glfwSetCursorPosCallback(window, mouse_move_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);

  /* OpenGL */
  if (!gladLoadGL()) {
    return PICASSO_WINDOW_GL_CONTEXT_FAIL;
  }

  int major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  printf("GL %d.%d\n", major, minor);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glClearDepth(1);
  glDepthFunc(GL_LESS);
  glViewport(0, 0, res_width, res_height);
  glClearColor(0.5f, 0.5f, 1.0f, 1.0f);

  if (gl_debug) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    glDebugMessageCallback((GLDEBUGPROC)debug_callback, NULL);
  }

  return PICASSO_WINDOW_OK;
}

void picasso_window_kill(void) {
  glfwTerminate();
}

void picasso_window_clear(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void picasso_window_swap(void) {
  glfwSwapBuffers(window);
}

void picasso_window_update(void) {
  glfwPollEvents();
}

bool picasso_window_should_close(void) {
  return glfwWindowShouldClose(window);
}

void picasso_window_keyboard_callback(PicassoWindowKeyboardCallback callback) {
  assert(callback);
  picasso_keyboard_callback = callback;
}

void picasso_window_mouse_move_callback(PicassoWindowMouseCallback callback) {
  assert(callback);
  picasso_mouse_move_callback = callback;
}

void picasso_window_mouse_button_callback(PicassoWindowMouseCallback callback) {
  assert(callback);
  picasso_mouse_button_callback = callback;
}
