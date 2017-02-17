#ifndef __PICASSO_H__
#define __PICASSO_H__

typedef enum {
	PICASSO_SHADER_VERTEX = 1,
	PICASSO_SHADER_FRAGMENT,
} PicassoShaderType;

typedef enum {
	PICASSO_SHADER_OK = 1,
	PICASSO_SHADER_COMPILE_FAILED,
} PicassoShaderResult;

#ifndef PICASSO_INTERNAL
typedef void PicassoShader;
typedef void PicassoProgram;
#else
#include "p_types.h"
#endif

PicassoShader *picasso_shader_create(PicassoShaderType type);
void picasso_shader_destroy(PicassoShader *shader);
PicassoShaderResult picasso_shader_compile(PicassoShader* shader, const uint8_t *source, uintmax_t length);

PicassoProgram *picasso_program_create(const uint8_t *vert_source, size_t vert_length, const uint8_t *frag_source, size_t frag_length);
void picasso_program_destroy(PicassoProgram *program);
void picasso_program_use(const PicassoProgram *program);
int32_t picasso_program_attrib_location(const PicassoProgram *program, const char *name);
int32_t picasso_program_uniform_location(const PicassoProgram *program, const char *name);
void picasso_program_mat4_set(const PicassoProgram *program, int32_t uniform, float *mat);

#endif // __PICASSO_H__
