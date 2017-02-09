#ifndef __OCCULUS_H__
#define __OCCULUS_H__

#include <stdint.h>

void* occulus_malloc(size_t, const char*, uint64_t);
void* occulus_realloc(void*, size_t, const char*, uint64_t);
void occulus_free(void*, const char*, uint64_t);

void occulus_print();

#ifdef MEM_DEBUG
#define malloc(n) occulus_malloc(n, __FILE__, __LINE__)
#define realloc(n, s) occulus_realloc(n, s, __FILE__, __LINE__)
#define free(n) occulus_free(n, __FILE__, __LINE__)
#endif

#endif // __OCCULUS_H__
