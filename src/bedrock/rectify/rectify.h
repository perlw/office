/**
 * Rectify - datastructures and memory
 */

#pragma once

#include <stdint.h>

// +array
void *rectify_array_alloc(uintmax_t chunk_size, uintmax_t element_size);
void rectify_array_free(void *const ptr);

void *rectify_array_push(void *ptr, const void *data);
void *rectify_array_delete(void *ptr, uintmax_t index);

uintmax_t rectify_array_cap(const void *const ptr);
uintmax_t rectify_array_size(const void *const ptr);
// -array

// +memory
void *rectify_memory_alloc_copy(const void *ptr, uintmax_t size);
//void *rectify_memory_alloc_copy_debug(const void *ptr, uintmax_t size, const char *filepath, uintmax_t line, const char *function);
//#define rectify_memory_alloc_copy(a, b) rectify_memory_alloc_copy_debug(a, b, __FILE__, __LINE__, __func__)
// +memory
