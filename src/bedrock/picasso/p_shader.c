#include "p_internal.h"

GLenum picasso_shader_type_to_gl(PicassoShaderType type) {
  switch (type) {
    case PICASSO_SHADER_VERTEX:
      return GL_VERTEX_SHADER;
    case PICASSO_SHADER_FRAGMENT:
      return GL_FRAGMENT_SHADER;
    default:
      return 0;
  }
}

PicassoShader *picasso_shader_create(PicassoShaderType type) {
  GLenum gl_type = picasso_shader_type_to_gl(type);
  if (!gl_type) {
    return NULL;
  }

  PicassoShader *shader = calloc(1, sizeof(PicassoShader));
  *shader = (PicassoShader){
    .id = glCreateShader(gl_type),
    .type = type,
  };
  return shader;
}

void picasso_shader_destroy(PicassoShader *shader) {
  assert(shader);

  glDeleteShader(shader->id);
  free(shader);
}

PicassoResult picasso_shader_compile(PicassoShader *shader, uintmax_t length, const uint8_t *source) {
  assert(shader);

  glShaderSource(shader->id, 1, (const GLchar *const *)&source, (const GLint *)&length);
  glCompileShader(shader->id);

  GLint log_length;
  glGetShaderiv(shader->id, GL_INFO_LOG_LENGTH, &log_length);
  if (log_length > 1) {
    PicassoResult result;
    result.result = PICASSO_SHADER_COMPILE_FAILED;
    result.length = log_length;

    log_length = (log_length >= 255 ? 255 : log_length);
    glGetShaderInfoLog(shader->id, log_length, 0, result.detail);
    result.detail[log_length + 1] = '\0';

    return result;
  }

  return (PicassoResult){
    .result = PICASSO_SHADER_OK,
    .length = 0,
    .detail = { 0 },
  };
}
