#pragma once

#include <stdint.h>

// +array
void *rectify_array_alloc(uintmax_t chunk_size, uintmax_t element_size);
void rectify_array_free(void *ptr);

void *rectify_array_push(void *ptr, const void *data);

uintmax_t rectify_array_cap(void *ptr);
uintmax_t rectify_array_size(void *ptr);
// -array

// +memory
void *rectify_memory_alloc_copy(const void *ptr, uintmax_t size);
// +memory
