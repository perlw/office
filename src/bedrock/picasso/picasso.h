#pragma once

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

// Shaders
PicassoShader *picasso_shader_create(PicassoShaderType type);
void picasso_shader_destroy(PicassoShader *shader);
PicassoShaderResult picasso_shader_compile(const PicassoShader* shader, const uint8_t *source, uintmax_t length);

// Programs
PicassoProgram *picasso_program_create(void);
void picasso_program_destroy(PicassoProgram *program);
void picasso_program_link_shaders(const PicassoProgram *program, uintmax_t num_shaders, const PicassoShader **shaders);
void picasso_program_use(const PicassoProgram *program);
int32_t picasso_program_attrib_location(const PicassoProgram *program, const char *name);
int32_t picasso_program_uniform_location(const PicassoProgram *program, const char *name);
void picasso_program_mat4_set(const PicassoProgram *program, int32_t uniform, float *mat);
