#include "r_internal.h"

void *rectify_memory_alloc_copy(const void *ptr, uintmax_t size) {
  assert(ptr);

  void *cptr = malloc(size);
  memcpy(cptr, ptr, size);
  return cptr;
}
