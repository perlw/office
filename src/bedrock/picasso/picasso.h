#pragma once

// +Shaders
typedef enum {
  PICASSO_SHADER_VERTEX = 1,
  PICASSO_SHADER_FRAGMENT,
} PicassoShaderType;

typedef enum {
  PICASSO_SHADER_OK = 1,
  PICASSO_SHADER_COMPILE_FAILED,
} PicassoShaderResult;

typedef struct PicassoShader PicassoShader;
typedef struct PicassoProgram PicassoProgram;

PicassoShader *picasso_shader_create(PicassoShaderType type);
void picasso_shader_destroy(PicassoShader *shader);
PicassoShaderResult picasso_shader_compile(PicassoShader* shader, const uint8_t *source, uintmax_t length);

PicassoProgram *picasso_program_create(void);
void picasso_program_destroy(PicassoProgram *program);
void picasso_program_link_shaders(PicassoProgram *program, uintmax_t num_shaders, const PicassoShader **shaders);
void picasso_program_use(PicassoProgram *program);
int32_t picasso_program_attrib_location(PicassoProgram *program, const char *name);
int32_t picasso_program_uniform_location(PicassoProgram *program, const char *name);
void picasso_program_mat4_set(PicassoProgram *program, int32_t uniform, float *mat);
// -Shaders

// +Buffers
typedef enum {
  PICASSO_BUFFER_TYPE_INT = 1,
  PICASSO_BUFFER_TYPE_FLOAT,
} PicassoBufferDataType;

typedef struct PicassoBufferGroup PicassoBufferGroup;
typedef struct PicassoBuffer PicassoBuffer;

PicassoBufferGroup *picasso_buffergroup_create(void);
void picasso_buffergroup_destroy(PicassoBufferGroup *buffergroup);
void picasso_buffergroup_draw(PicassoBufferGroup *buffergroup, uintmax_t num_vertices);

PicassoBuffer *picasso_buffer_create(PicassoBufferGroup *buffergroup);
void picasso_buffer_set_data(PicassoBuffer *buffer, uintmax_t size, void *data);
void picasso_buffer_shader_attrib(PicassoBuffer *buffer, int32_t attr_pos, uintmax_t num_fields, PicassoBufferDataType type);
// -Buffers

