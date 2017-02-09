#include <stdio.h>
#include <stdint.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"

#include "occulus.h"
#include "kronos.h"

int quit = 0;

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_REPEAT) {
    return;
  }

	if (key == GLFW_KEY_ESCAPE) {
		quit = 1;
	}
}

void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *user_param) {
  printf("GL debug %s\n", message);
}

int main() {
  if (!glfwInit()) {
    printf("glfw fail\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow* window = glfwCreateWindow(640, 480, "Office", NULL, NULL);
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
  glViewport(0, 0, 640, 480);

  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
  glDebugMessageCallback((GLDEBUGPROC)debug_callback, NULL);

  double last_tick = kronos_time();
  double current_second = 0;

  uint32_t frames = 0;
  glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
  while (!glfwWindowShouldClose(window) && !quit) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double tick = kronos_time();
    double diff = tick - last_tick;
    last_tick = tick;

    current_second += diff;

    frames++;
    if (current_second >= 1) {
      current_second = 0;
			printf("FPS: %d\n", frames);
      frames = 0;
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();

#ifdef MEM_DEBUG
  occulus_print();
#endif

  return 0;
}
