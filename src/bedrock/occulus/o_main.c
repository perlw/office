#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#define FILEPATH_LENGTH 128
#define FUNCTION_LENGTH 128
#define ALLOC_CHUNK 100

typedef struct {
  const void *ptr;
  uintmax_t size;
  bool freed;

  char filepath[FILEPATH_LENGTH];
  char function[FUNCTION_LENGTH];
  uintmax_t line;
} Allocation;

uintmax_t max_mem = 0;
uintmax_t mem_leaked = 0;
uintmax_t num_allocations = 0;
uintmax_t allocations_length = 0;
Allocation *allocations = NULL;

void log_allocation(const void *ptr, size_t size, const char *restrict filepath, uintmax_t line, const char *restrict function) {
  if (num_allocations >= allocations_length) {
    allocations_length += ALLOC_CHUNK;
    allocations = realloc(allocations, allocations_length * sizeof(Allocation));
  }

  allocations[num_allocations] = (Allocation){
    .ptr = ptr,
    .size = size,
    .freed = false,
    .filepath = { 0 },
    .function = { 0 },
    .line = line,
  };
  strncpy(allocations[num_allocations].filepath, filepath, FILEPATH_LENGTH);
  strncpy(allocations[num_allocations].function, function, FUNCTION_LENGTH);
  num_allocations += 1;

  mem_leaked += size;
  max_mem = (mem_leaked > max_mem ? mem_leaked : max_mem);
}

void *occulus_malloc(size_t size, const char *restrict filepath, uintmax_t line, const char *restrict function) {
  void *ptr = malloc(size);
  assert(ptr);
  log_allocation(ptr, size, filepath, line, function);
  return ptr;
}

void *occulus_calloc(size_t num, size_t size, const char *restrict filepath, uintmax_t line, const char *restrict function) {
  void *ptr = calloc(num, size);
  assert(ptr);
  log_allocation(ptr, num * size, filepath, line, function);
  return ptr;
}

// FIXME: Log allocation
void *occulus_realloc(void *old_ptr, size_t size, const char *restrict filepath, uintmax_t line, const char *restrict function) {
  void *ptr = realloc(old_ptr, size);
  assert(ptr);
  printf("%s:%" PRIuMAX "> realloc(%zu) 0x%" PRIuPTR "x->0x%" PRIuPTR "x\n", filepath, line, size, (uintptr_t)old_ptr, (uintptr_t)ptr);
  return ptr;
}

void occulus_free(void *ptr, const char *restrict file, uintmax_t line, const char *restrict function) {
  assert(ptr);
  bool found = false;
  for (uintmax_t t = 0; t < num_allocations; t++) {
    if (allocations[t].ptr == ptr && !allocations[t].freed) {
      mem_leaked -= allocations[t].size;
      allocations[t].freed = true;
      found = true;
      break;
    }
  }

  if (!found) {
    printf("%s:%lu> free on unrecognized memory, 0x%0lx\n", file, line, (uintmax_t)ptr);
  }

  free(ptr);
}

// FIXME: Prettify and merge stats
// Filepath NumAlloc MaxSize Leaked
// ...
// AppMaxSize AppLeaked
void occulus_print(bool detailed) {
  printf("MEM_DEBUG>\nMax: %.2fkb\tLeaked: %.2fkb\n", (double)max_mem / 1024.0, (double)mem_leaked / 1024.0);
  if (detailed) {
    for (uintmax_t t = 0; t < num_allocations; t++) {
      printf("%s:%" PRIuMAX "/%s #%.2fkb\n", allocations[t].filepath, allocations[t].line, allocations[t].function, (double)allocations[t].size / 1024.0);
    }
  }

  for (uintmax_t t = 0; t < num_allocations; t++) {
    if (!allocations[t].freed) {
      printf("Memory at %s:%" PRIuMAX "/%s never freed and leaked %.2fkb!\n", allocations[t].filepath, allocations[t].line, allocations[t].function, (double)allocations[t].size / 1024.0);
    }
  }
  printf("<MEM_DEBUG\n");
}
