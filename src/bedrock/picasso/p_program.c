#include "p_internal.h"

PicassoProgram *picasso_program_create(void) {
  PicassoProgram *program = calloc(1, sizeof(PicassoProgram));

  *program = (PicassoProgram){
    .id = glCreateProgram(),
  };

  return program;
}

PicassoResult picasso_program_link_shaders(PicassoProgram *const program, uintmax_t num_shaders, const PicassoShader **shaders) {
  assert(program);
  assert(num_shaders > 0);
  assert(shaders);

  for (uintmax_t t = 0; t < num_shaders; t++) {
    glAttachShader(program->id, shaders[t]->id);
  }
  glLinkProgram(program->id);

  picasso_program_use(NULL);

  return (PicassoResult){
    .result = PICASSO_SHADER_OK,
    .length = 0,
    .detail = { 0 },
  };
}

void picasso_program_destroy(PicassoProgram *const program) {
  assert(program);

  glDeleteProgram(program->id);
  free(program);
}

void picasso_program_use(PicassoProgram *const program) {
  uint32_t id = (program ? program->id : 0);
  if (get_state(PICASSO_STATE_PROGRAM) != id) {
    glUseProgram(id);
    set_state(PICASSO_STATE_PROGRAM, id);
  }
}

int32_t picasso_program_attrib_location(PicassoProgram *const program, const char *name) {
  assert(program);

  return glGetAttribLocation(program->id, name);
}

int32_t picasso_program_uniform_location(PicassoProgram *const program, const char *name) {
  assert(program);

  return glGetUniformLocation(program->id, name);
}

void picasso_program_uniform_int(PicassoProgram *const program, int32_t uniform, int32_t val) {
  assert(program);

  glProgramUniform1i(program->id, uniform, val);
}

void picasso_program_uniform_float(PicassoProgram *const program, int32_t uniform, float val) {
  assert(program);

  glProgramUniform1f(program->id, uniform, val);
}

void picasso_program_uniform_mat4(PicassoProgram *const program, int32_t uniform, const float *mat) {
  assert(program);

  glProgramUniformMatrix4fv(program->id, uniform, 1, GL_FALSE, (const GLfloat *)mat);
}
