#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "bedrock.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

bool quit = false;
GLFWwindow *window = NULL;

void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (action == GLFW_REPEAT) {
    return;
  }

  gossip_emit(GOSSIP_ID_INPUT_KEY, (void*)&(BedrockKeyboardEvent){
    .key = key,
    .scancode = scancode,
    .press = (action == GLFW_PRESS),
    .release = (action == GLFW_RELEASE),
  });
}

void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *user_param) {
  printf("BEDROCK: GL %s\n", message);
}

void should_close_callback(void *userdata) {
  quit = true;
}

int bedrock_init(const char *title, uint32_t res_width, uint32_t res_height, bool gl_debug) {
  if (!glfwInit()) {
    printf("glfw fail\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, (gl_debug ? GL_TRUE : GL_FALSE));

  window = glfwCreateWindow(res_width, res_height, title, NULL, NULL);
  if (!window) {
    printf("glfw window fail\n");
    glfwTerminate();
    return -1;
  }

  glfwSetKeyCallback(window, keyboard_callback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);

  /* OpenGL */
  if(!gladLoadGL()) {
    printf("GLAD fail\n");
    return -1;
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

  if (gl_debug) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    glDebugMessageCallback((GLDEBUGPROC)debug_callback, NULL);
  }

  gossip_subscribe(GOSSIP_ID_CLOSE, &should_close_callback);

  return 1;
}

void bedrock_kill(void) {
  glfwTerminate();
  gossip_cleanup();
}

void bedrock_clear_color(float r, float g, float b, float a) {
  glClearColor(r, g, b, a);
}

void bedrock_clear(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bedrock_swap(void) {
  glfwSwapBuffers(window);
}

void bedrock_poll(void) {
  glfwPollEvents();
}

int bedrock_should_close(void) {
  return glfwWindowShouldClose(window) || quit;
}

#ifdef WIN32
double bedrock_time(void) {
  static int initialized = 0;
  static uint64_t freq, start;
  uint64_t curr;

  if (!initialized) {
    initialized = 1;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&start);
  }
  QueryPerformanceCounter((LARGE_INTEGER*)&curr);

  return (double)(curr - start) / freq;
}
#else
#include <time.h>
double bedrock_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (float)ts.tv_sec + ((float)ts.tv_nsec / 1000000000.0f);
}
#endif
