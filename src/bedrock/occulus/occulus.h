#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

void *occulus_malloc(size_t size, const char *filepath, uintmax_t line, const char *function);
void *occulus_calloc(size_t num, size_t size, const char *filepath, uintmax_t line, const char *function);
void *occulus_realloc(void *old_ptr, size_t size, const char *filepath, uintmax_t line, const char *function);
void occulus_free(void *ptr, const char *filepath, uintmax_t line, const char *function);
void occulus_print(bool detailed);
uintmax_t occulus_current_allocated(void);

#ifdef MEM_DEBUG
#define malloc(n) occulus_malloc(n, __FILE__, __LINE__, __func__)
#define calloc(n, s) occulus_calloc(n, s, __FILE__, __LINE__, __func__)
#define realloc(n, s) occulus_realloc(n, s, __FILE__, __LINE__, __func__)
#define free(n) occulus_free(n, __FILE__, __LINE__, __func__)
#endif // MEM_DEBUG
