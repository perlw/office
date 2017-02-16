#include "p_internal.h"

uint32_t compile_shader(const uint8_t *source, size_t length, GLenum type) {
  uint32_t shader = glCreateShader(type);
  glShaderSource(shader, 1, (const GLchar* const*)&source, (const GLint*)&length);
  glCompileShader(shader);

  {
    int length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
      GLchar log[1024];
      glGetShaderInfoLog(shader, length, 0, &log[0]);
      log[length + 1] = '\0';
      printf("--------------->\n%s\n<-------------\n", log);
    }
  }
  return shader;
}

// TODO: Error handling
PicassoProgram *picasso_program_create(const uint8_t *vert_source, size_t vert_length, const uint8_t *frag_source, size_t frag_length) {
  PicassoProgram *program = calloc(1, sizeof(PicassoProgram));

  uint32_t vertex_shader, fragment_shader;

  vertex_shader = compile_shader(vert_source, vert_length, GL_VERTEX_SHADER);
  fragment_shader = compile_shader(frag_source, frag_length, GL_FRAGMENT_SHADER);

  *program = glCreateProgram();
  glAttachShader(*program, vertex_shader);
  glAttachShader(*program, fragment_shader);
  glLinkProgram(*program);
  glUseProgram(0);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return program;
}

void picasso_program_destroy(PicassoProgram *program) {
  glDeleteProgram(*program);
  free(program);
}

void picasso_program_use(const PicassoProgram *program) {
  glUseProgram(*program);
}

int32_t picasso_program_attrib_location(const PicassoProgram *program, const char *name) {
  return glGetAttribLocation(*program, name);
}

int32_t picasso_program_uniform_location(const PicassoProgram *program, const char *name) {
  return glGetUniformLocation(*program, name);
}

void picasso_program_mat4_set(const PicassoProgram *program, int32_t uniform, float *mat) {
  glProgramUniformMatrix4fv(*program, uniform, 1, GL_FALSE, (const GLfloat*)mat);
}
