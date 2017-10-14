#include <inttypes.h>
#include <stdio.h>

#include "r_internal.h"

void *rectify_memory_alloc_copy(const void *ptr, uintmax_t size) {
  assert(ptr);

  void *cptr = malloc(size);
  memcpy(cptr, ptr, size);
  return cptr;
}

void *rectify_memory_alloc_copy_debug(const void *ptr, uintmax_t size, const char *filepath, uintmax_t line, const char *function) {
  assert(ptr);

  void *cptr = malloc(size);
  memcpy(cptr, ptr, size);
  printf("%s:%" PRIuMAX "/%s> copy alloc %p -> %p.\n", filepath, line, function, ptr, cptr);
  return cptr;
}
