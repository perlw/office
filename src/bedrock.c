#include <stdio.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "gossip.h"

int quit = 0;
GLFWwindow* window = NULL;

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_REPEAT) {
    return;
  }

  if (key == GLFW_KEY_ESCAPE) {
		gossip_emit(GOSSIP_BEDROCK_CLOSE);
  }
}

void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *user_param) {
  printf("GL debug %s\n", message);
}

void should_close_callback(void) {
	quit = 1;
}

int bedrock_init() {
	if (!glfwInit()) {
    printf("glfw fail\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  window = glfwCreateWindow(640, 480, "Office", NULL, NULL);
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

	gossip_subscribe(GOSSIP_BEDROCK_CLOSE, should_close_callback);

	return 1;
}

void bedrock_kill() {
	glfwTerminate();
}

void bedrock_swap() {
	glfwSwapBuffers(window);
}

void bedrock_poll() {
	glfwPollEvents();
}

int bedrock_should_close() {
	return glfwWindowShouldClose(window) || quit;
}
