#include <string.h>

#include "p_internal.h"

#include "GLFW/glfw3.h"

bool quit = false;
GLFWwindow *window = NULL;

uintmax_t num_bindings = 0;
PicassoWindowInputBinding *input_bindings = NULL;
PicassoWindowInputCallback main_callback = NULL;
void *main_callback_userdata = NULL;

void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (action == GLFW_REPEAT) {
    return;
  }

  gossip_emit(GOSSIP_ID_INPUT_KEY, &(PicassoWindowInputEvent){
    .key = key,
    .scancode = scancode,
    .pressed = (action == GLFW_PRESS),
    .released = (action == GLFW_RELEASE),
    .shift = (mods & GLFW_MOD_SHIFT),
  });

  if (action == GLFW_RELEASE) {
    return;
  }

  for (uintmax_t t = 0; t < num_bindings; t++) {
    if (input_bindings[t].key == key) {
      if (input_bindings[t].callback) {
        input_bindings[t].callback(&input_bindings[t], input_bindings[t].userdata);
      }

      main_callback(&input_bindings[t], main_callback_userdata);
    }
  }
}

void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *user_param) {
  printf("PICASSO: GL %s\n", message);
}

void should_close_callback(void *subscriberdata, void *userdata) {
  quit = true;
}

PicassoWindowResult picasso_window_init(const char *title, uint32_t res_width, uint32_t res_height, bool gl_debug) {
  if (!glfwInit()) {
    return PICASSO_WINDOW_INIT_FAIL;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, (gl_debug ? GL_TRUE : GL_FALSE));

  window = glfwCreateWindow(res_width, res_height, title, NULL, NULL);
  if (!window) {
    glfwTerminate();
    return PICASSO_WINDOW_CREATION_FAIL;
  }

  glfwSetKeyCallback(window, keyboard_callback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);

  /* OpenGL */
  if(!gladLoadGL()) {
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

  gossip_subscribe(GOSSIP_ID_CLOSE, &should_close_callback, NULL);

  return PICASSO_WINDOW_OK;
}

void picasso_window_kill(void) {
  for (uintmax_t t = 0; t < num_bindings; t++) {
    free(input_bindings[t].action);
  }
  free(input_bindings);

  glfwTerminate();
  gossip_cleanup();
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
  return glfwWindowShouldClose(window) || quit;
}

void picasso_window_action_callback(PicassoWindowInputCallback callback, void *userdata) {
  assert(callback);

  main_callback = callback;
  main_callback_userdata = userdata;
}

void picasso_window_add_binding(PicassoWindowInputBinding *binding) {
  assert(binding);
  if (!input_bindings) {
    input_bindings = calloc(1, sizeof(PicassoWindowInputBinding));
  }

  uintmax_t length = strlen(binding->action) + 1;
  num_bindings++;
  input_bindings = realloc(input_bindings, num_bindings * sizeof(PicassoWindowInputBinding));
  input_bindings[num_bindings - 1] = (PicassoWindowInputBinding){
    .action = rectify_memory_alloc_copy(binding->action, length),
    .key = binding->key,
    .callback = binding->callback,
    .userdata = binding->userdata,
  };
}
