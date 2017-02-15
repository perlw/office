#include "bedrock.h"

uint32_t compile_shader(const uint8_t* source, size_t length, GLenum type) {
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

BPicassoProgram* bedrock_picasso_program_create(const uint8_t* vert_source, size_t vert_length, const uint8_t* frag_source, size_t frag_length) {
  BPicassoProgram* p_program = calloc(1, sizeof(BPicassoProgram));

  uint32_t vertex_shader, fragment_shader;

  vertex_shader = compile_shader(vert_source, vert_length, GL_VERTEX_SHADER);
  fragment_shader = compile_shader(frag_source, frag_length, GL_FRAGMENT_SHADER);

  uint32_t program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  glUseProgram(0);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  p_program->program_id = program;
  return p_program;
}

// TODO: Clean up program
void bedrock_picasso_program_destroy(BPicassoProgram* program) {
  free(program);
}
