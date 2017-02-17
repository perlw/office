#include "p_internal.h"

Shader *shader_create(GLenum type) {
	Shader *shader = calloc(1, sizeof(Shader));
	*shader = (Shader) {
		.id = glCreateShader(type),
		.type = type,
	};
	return shader;
}

void shader_destroy(Shader *shader) {
	assert(shader);

	glDeleteShader(shader->id);
	free(shader);
}

PicassoShaderResult shader_compile(Shader* shader, const uint8_t *source, uintmax_t length) {
	assert(shader);

	glShaderSource(shader->id, 1, (const GLchar* const*)&source, (const GLint*)&length);
	glCompileShader(shader->id);

	GLint log_length;
	glGetShaderiv(shader->id, GL_INFO_LOG_LENGTH, &log_length);
	if (length > 1) {
		GLchar log[1024];
		glGetShaderInfoLog(shader->id, log_length, 0, &log[0]);
		log[log_length + 1] = '\0';
		printf("--------------->\n%s\n<-------------\n", log);
		return PICASSO_SHADER_COMPILE_FAILED;
	}

	return PICASSO_SHADER_OK;
}

// Vertex interface
PicassoVertexShader *picasso_shader_vertex_create(void) {
	return shader_create(GL_VERTEX_SHADER);
}

void picasso_shader_vertex_destroy(PicassoVertexShader *shader) {
	shader_destroy(shader);
}

PicassoShaderResult picasso_shader_vertex_compile(PicassoVertexShader* shader, const uint8_t *source, uintmax_t length) {
	return shader_compile(shader, source, length);
}

// Fragment interface
PicassoFragmentShader *picasso_shader_fragment_create(void) {
	return shader_create(GL_FRAGMENT_SHADER);
}

void picasso_shader_fragment_destroy(PicassoFragmentShader *shader) {
	shader_destroy(shader);
}

PicassoShaderResult picasso_shader_fragment_compile(PicassoFragmentShader* shader, const uint8_t *source, uintmax_t length) {
	return shader_compile(shader, source, length);
}
