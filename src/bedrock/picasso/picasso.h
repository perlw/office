#ifndef __PICASSO_H__
#define __PICASSO_H__

#ifndef PICASSO_INTERNAL
typedef void PicassoProgram;
#endif

PicassoProgram *picasso_program_create(const uint8_t *vert_source, size_t vert_length, const uint8_t *frag_source, size_t frag_length);
void picasso_program_destroy(PicassoProgram *program);
void picasso_program_use(const PicassoProgram *program);
int32_t picasso_program_attrib_location(const PicassoProgram *program, const char *name);
int32_t picasso_program_uniform_location(const PicassoProgram *program, const char *name);
void picasso_program_mat4_set(const PicassoProgram *program, int32_t uniform, float *mat);

#endif // __PICASSO_H__
