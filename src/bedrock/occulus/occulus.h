#ifndef __OCCULUS_H__
#define __OCCULUS_H__

#include <stdint.h>
#include <stdlib.h>

void *occulus_malloc(size_t size, const char *file, uint64_t line);
void *occulus_calloc(size_t num, size_t size, const char *file, uint64_t line);
void *occulus_realloc(void *old_ptr, size_t size, const char *file, uint64_t line);
void occulus_free(void *ptr, const char *file, uint64_t line);
void occulus_print();

#ifdef MEM_DEBUG
#define malloc(n) occulus_malloc(n, __FILE__, __LINE__)
#define calloc(n, s) occulus_calloc(n, s, __FILE__, __LINE__)
#define realloc(n, s) occulus_realloc(n, s, __FILE__, __LINE__)
#define free(n) occulus_free(n, __FILE__, __LINE__)
#endif // MEM_DEBUG

#endif // __OCCULUS_H__