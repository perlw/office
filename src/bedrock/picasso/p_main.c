#include "p_internal.h"

// TODO: Should not do all this in here
PicassoProgram *picasso_program_create(const uint8_t *vert_source, size_t vert_length, const uint8_t *frag_source, size_t frag_length) {
	PicassoVertexShader* vertex_shader = picasso_shader_vertex_create();
	PicassoFragmentShader* fragment_shader = picasso_shader_fragment_create();
	picasso_shader_vertex_compile(vertex_shader, vert_source, vert_length);
	picasso_shader_fragment_compile(fragment_shader, frag_source, frag_length);

	uint32_t program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader->id);
	glAttachShader(program_id, fragment_shader->id);
	glLinkProgram(program_id);
	glUseProgram(0);
	picasso_shader_vertex_destroy(vertex_shader);
	picasso_shader_fragment_destroy(vertex_shader);

	PicassoProgram *program = calloc(1, sizeof(PicassoProgram));

	*program = (PicassoProgram) {
		.id = program_id,
	};

	return program;
}

void picasso_program_destroy(PicassoProgram *program) {
	assert(program);

	glDeleteProgram(program->id);
	free(program);
}

void picasso_program_use(const PicassoProgram *program) {
	assert(program);

	glUseProgram(program->id);
}

int32_t picasso_program_attrib_location(const PicassoProgram *program, const char *name) {
	assert(program);

	return glGetAttribLocation(program->id, name);
}

int32_t picasso_program_uniform_location(const PicassoProgram *program, const char *name) {
	assert(program);

	return glGetUniformLocation(program->id, name);
}

void picasso_program_mat4_set(const PicassoProgram *program, int32_t uniform, float *mat) {
	assert(program);

	glProgramUniformMatrix4fv(program->id, uniform, 1, GL_FALSE, (const GLfloat*)mat);
}
