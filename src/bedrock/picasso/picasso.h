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
PicassoShaderResult picasso_shader_compile(PicassoShader* shader, uintmax_t length, const uint8_t *source);

PicassoProgram *picasso_program_create(void);
void picasso_program_destroy(PicassoProgram *program);
void picasso_program_link_shaders(PicassoProgram *program, uintmax_t num_shaders, const PicassoShader **shaders);
void picasso_program_use(PicassoProgram *program);
int32_t picasso_program_attrib_location(PicassoProgram *program, const char *name);
int32_t picasso_program_uniform_location(PicassoProgram *program, const char *name);
void picasso_program_uniform_int(PicassoProgram *program, int32_t uniform, int32_t val);
void picasso_program_uniform_mat4(PicassoProgram *program, int32_t uniform, float *mat);
// -Shaders

// +Buffers
typedef enum {
  PICASSO_BUFFER_TYPE_ARRAY = 1,
  PICASSO_BUFFER_TYPE_ELEMENT,
} PicassoBufferType;

typedef enum {
  PICASSO_BUFFER_USAGE_STREAM  = 1,
  PICASSO_BUFFER_USAGE_STATIC,
  PICASSO_BUFFER_USAGE_DYNAMIC,
} PicassoBufferUsage;

typedef enum {
  PICASSO_TYPE_INT = 1,
  PICASSO_TYPE_FLOAT,
} PicassoDataType;

typedef enum {
  PICASSO_BUFFER_MODE_LINES = 1,
  PICASSO_BUFFER_MODE_TRIANGLES,
} PicassoBufferMode;

typedef struct PicassoBufferGroup PicassoBufferGroup;
typedef struct PicassoBuffer PicassoBuffer;

PicassoBufferGroup *picasso_buffergroup_create(void);
void picasso_buffergroup_destroy(PicassoBufferGroup *buffergroup);
void picasso_buffergroup_draw(PicassoBufferGroup *buffergroup, PicassoBufferMode mode, uintmax_t num_vertices);

PicassoBuffer *picasso_buffer_create(PicassoBufferGroup *buffergroup, PicassoBufferType type, PicassoBufferUsage usage);
void picasso_buffer_set_data(PicassoBuffer *buffer, uintmax_t num_fields, PicassoDataType type, uintmax_t size, void *data);
void picasso_buffer_shader_attrib(PicassoBuffer *buffer, int32_t attr_pos);
// -Buffers

// +Textures
typedef enum {
  PICASSO_TEXTURE_TARGET_1D = 1,
  PICASSO_TEXTURE_TARGET_2D,
} PicassoTextureTarget;

typedef enum {
  PICASSO_TEXTURE_R = 1,
  PICASSO_TEXTURE_G = 1,
  PICASSO_TEXTURE_B = 1,
  PICASSO_TEXTURE_RGB = 3,
  PICASSO_TEXTURE_RGBA = 4,
} PicassoTextureChannels;

typedef enum {
  PICASSO_TEXTURE_OK = 1,
  PICASSO_TEXTURE_LOAD_FAILED,
} PicassoTextureResult;

typedef struct PicassoTexture PicassoTexture;

PicassoTexture *picasso_texture_create(PicassoTextureTarget target);
void picasso_texture_destroy(PicassoTexture *texture);

PicassoTextureResult picasso_texture_load(PicassoTexture *texture, const uint8_t *data, uintmax_t size, PicassoTextureChannels channels);
void picasso_texture_bind_to(PicassoTexture *texture, uint32_t index);
// -Textures

