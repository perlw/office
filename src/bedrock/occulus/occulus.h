#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

void *occulus_malloc(size_t size, const char *file, uintmax_t line);
void *occulus_calloc(size_t num, size_t size, const char *file, uintmax_t line);
void *occulus_realloc(void *old_ptr, size_t size, const char *file, uintmax_t line);
void occulus_free(void *ptr, const char *file, uintmax_t line);
void occulus_print(bool detailed);

#ifdef MEM_DEBUG
#define malloc(n) occulus_malloc(n, __FILE__, __LINE__)
#define calloc(n, s) occulus_calloc(n, s, __FILE__, __LINE__)
#define realloc(n, s) occulus_realloc(n, s, __FILE__, __LINE__)
#define free(n) occulus_free(n, __FILE__, __LINE__)
#endif // MEM_DEBUG
