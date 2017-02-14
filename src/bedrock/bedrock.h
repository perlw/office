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
typedef void (*BedrockGossipCallbackType)(void);

typedef enum {
	BEDROCK_GOSSIP_ID_CLOSE = 0x0001,

	BEDROCK_GOSSIP_ID_MAX,
} BedrockGossipID;

void bedrock_gossip_subscribe(BedrockGossipID id, BedrockGossipCallbackType callback);
void bedrock_gossip_emit(BedrockGossipID id);
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


#endif // __BEDROCK_H__
