#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define LOCATION_STRING_LENGTH 64
#define ALLOC_CHUNK 100

typedef struct {
  const void *ptr;
  uintmax_t size;
  bool freed;
  char alloc_location[LOCATION_STRING_LENGTH];
  char free_location[LOCATION_STRING_LENGTH];
} Allocation;

uintmax_t max_mem = 0;
uintmax_t mem_leaked = 0;
uintmax_t num_allocations = 0;
uintmax_t allocations_length = 0;
Allocation *allocations = NULL;

void log_allocation(const void *ptr, size_t size, const char *file, uintmax_t line) {
  if (num_allocations >= allocations_length) {
    allocations_length += ALLOC_CHUNK;
    allocations = realloc(allocations, allocations_length * sizeof(Allocation));
  }

  allocations[num_allocations] = (Allocation){
    .ptr = ptr,
    .size = size,
    .freed = false,
    .alloc_location = { 0 },
    .free_location = { 0 },
  };
  snprintf(allocations[num_allocations].alloc_location, LOCATION_STRING_LENGTH, "%s:%lu", file, line);
  num_allocations += 1;

  mem_leaked += size;
  max_mem = (mem_leaked > max_mem ? mem_leaked : max_mem);
}

void *occulus_malloc(size_t size, const char *file, uintmax_t line) {
  void *ptr = malloc(size);
  assert(ptr);
  log_allocation(ptr, size, file, line);
  return ptr;
}

void *occulus_calloc(size_t num, size_t size, const char *file, uintmax_t line) {
  void *ptr = calloc(num, size);
  assert(ptr);
  log_allocation(ptr, num * size, file, line);
  return ptr;
}

// FIXME: Log allocation
void *occulus_realloc(void *old_ptr, size_t size, const char *file, uintmax_t line) {
  void *ptr = realloc(old_ptr, size);
  assert(ptr);
  printf("%s:%lu> realloc(%lu) 0x%0lx->0x%0lx\n", file, line, size, (uintmax_t)old_ptr, (uintmax_t)ptr);
  return ptr;
}

void occulus_free(void *ptr, const char *file, uintmax_t line) {
  assert(ptr);
  bool found = false;
  for (uintmax_t t = 0; t < num_allocations; t++) {
    if (allocations[t].ptr == ptr && !allocations[t].freed) {
      snprintf(allocations[num_allocations].free_location, LOCATION_STRING_LENGTH, "%s:%lu", file, line);
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

void occulus_print() {
  printf("MEM_DEBUG>\nMax: %lukb\tLeaked: %lukb\n", max_mem / 1024, mem_leaked / 1024);
  for (uintmax_t t = 0; t < num_allocations; t++) {
    if (!allocations[t].freed) {
      printf("Memory at %s never freed!\n", allocations[t].alloc_location);
    }
  }
  printf("<MEM_DEBUG\n");
}
