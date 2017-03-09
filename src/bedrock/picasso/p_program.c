#include "p_internal.h"

PicassoProgram *picasso_program_create(void) {
	PicassoProgram *program = calloc(1, sizeof(PicassoProgram));

	*program = (PicassoProgram) {
		.id = glCreateProgram(),
	};

	return program;
}

void picasso_program_link_shaders(const PicassoProgram *program, uintmax_t num_shaders, const PicassoShader **shaders) {
	assert(program);
	assert(num_shaders > 0);
	assert(shaders);

	for (uintmax_t t = 0; t < num_shaders; t++) {
		glAttachShader(program->id, shaders[t]->id);
	}
	glLinkProgram(program->id);

	picasso_program_use(NULL);
}

void picasso_program_destroy(PicassoProgram *program) {
	assert(program);

	glDeleteProgram(program->id);
	free(program);
}

void picasso_program_use(const PicassoProgram *program) {
	if (program && get_state(PICASSO_STATE_PROGRAM) != program->id) {
		glUseProgram(program->id);
    set_state(PICASSO_STATE_PROGRAM, program->id);
	} else {
		glUseProgram(0);
    set_state(PICASSO_STATE_PROGRAM, 0);
	}
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
