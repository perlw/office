#ifndef __BEDROCK_H__
#define __BEDROCK_H__

#include <stdio.h>
#include <malloc.h>
#include <stdint.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#ifdef BEDROCK_IMPLEMENTATION
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
	BEDROCK_GOSSIP_ID_CLOSE = 0x0001,

	BEDROCK_GOSSIP_ID_MAX,
} BGossipID;

void bedrock_gossip_subscribe(BGossipID id, BGossipCallback callback);
void bedrock_gossip_emit(BGossipID id);
// -Gossip


// +Kronos
double bedrock_kronos_time();
// -Kronos


// +Occulus
void* bedrock_occulus_malloc(size_t size, const char* file, uint64_t line);
void* bedrock_occulus_realloc(void* old_ptr, size_t size, const char* file, uint64_t line);
void bedrock_occulus_free(void* ptr, const char* file, uint64_t line);
void bedrock_occulus_print();

#ifdef MEM_DEBUG
#define malloc(n) bedrock_occulus_malloc(n, __FILE__, __LINE__)
#define realloc(n, s) bedrock_occulus_realloc(n, s, __FILE__, __LINE__)
#define free(n) bedrock_occulus_free(n, __FILE__, __LINE__)
#endif
// -Occulus


// +Picasso
typedef struct {
	uint32_t program_id;
} BPicassoProgram;

BPicassoProgram* bedrock_picasso_program_create(const uint8_t* vert_source, size_t vert_length, const uint8_t* frag_source, size_t frag_length);
void bedrock_picasso_program_destroy(BPicassoProgram* program);
// -Picasso


#endif // __BEDROCK_H__
