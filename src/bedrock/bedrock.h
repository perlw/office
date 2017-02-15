#ifndef __BEDROCK_H__
#define __BEDROCK_H__

#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#ifdef BEDROCK_INTERNAL
#include "b_internal.h"
#endif


int bedrock_init();
void bedrock_kill();

void bedrock_swap();
void bedrock_poll();
int bedrock_should_close();


// +Gossip
typedef void (*BGossipCallback)(void);

typedef enum {
  B_GOSSIP_ID_CLOSE = 0x0001,

  B_GOSSIP_ID_MAX,
} BGossipID;

void b_gossip_subscribe(BGossipID id, BGossipCallback callback);
void b_gossip_emit(BGossipID id);
// -Gossip


// +Kronos
double b_kronos_time();
// -Kronos


// +Occulus
#ifndef OCCULUS_IMPLEMENTATION
void *b_occulus_malloc(size_t size, const char *file, uint64_t line);
void *b_occulus_calloc(size_t num, size_t size, const char *file, uint64_t line);
void *b_occulus_realloc(void *old_ptr, size_t size, const char *file, uint64_t line);
void b_occulus_free(void *ptr, const char *file, uint64_t line);
void b_occulus_print();

#ifdef MEM_DEBUG
#define malloc(n) b_occulus_malloc(n, __FILE__, __LINE__)
#define calloc(n, s) b_occulus_calloc(n, s, __FILE__, __LINE__)
#define realloc(n, s) b_occulus_realloc(n, s, __FILE__, __LINE__)
#define free(n) b_occulus_free(n, __FILE__, __LINE__)
#endif // MEM_DEBUG
#endif // OCCULUS_IMPLEMENTATION
// -Occulus


// +Picasso
#ifndef BEDROCK_INTERNAL
typedef void BPicassoProgram;
#endif

BPicassoProgram *b_picasso_program_create(const uint8_t *vert_source, size_t vert_length, const uint8_t *frag_source, size_t frag_length);
void b_picasso_program_destroy(BPicassoProgram *program);
void b_picasso_program_use(const BPicassoProgram *program);
int32_t b_picasso_program_attrib_location(const BPicassoProgram *program, const char *name);
int32_t b_picasso_program_uniform_location(const BPicassoProgram *program, const char *name);
void b_picasso_program_mat4_set(const BPicassoProgram *program, int32_t uniform, float *mat);
// -Picasso


// +Archivist
bool b_archivist_read_file(const char *filepath, uint8_t **data, size_t *num_bytes);
// -Archivist


#endif // __BEDROCK_H__
