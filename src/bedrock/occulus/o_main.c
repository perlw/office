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

const char fence[3] = { 'O', 'C', 'C' };

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

void log_allocation(const void *restrict ptr, size_t size, const char *restrict filepath, uintmax_t line, const char *restrict function) {
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
  void *ptr = malloc(size + 6);
  assert(ptr);

  for (uintmax_t t = 0; t < 3; t++) {
    ((uint8_t*)ptr)[t] = fence[t];
    ((uint8_t*)ptr)[size + t + 3] = fence[t];
  }

  log_allocation(ptr, size, filepath, line, function);
  return (ptr + 3);
}

void *occulus_calloc(size_t num, size_t size, const char *restrict filepath, uintmax_t line, const char *restrict function) {
  size_t total = num * size;
  void *ptr = malloc(total + 6);
  assert(ptr);

  memset((ptr + 3), 0, total);
  for (uintmax_t t = 0; t < 3; t++) {
    ((uint8_t*)ptr)[t] = fence[t];
    ((uint8_t*)ptr)[total + t + 3] = fence[t];
  }

  log_allocation(ptr, total, filepath, line, function);
  return (ptr + 3);
}

// TODO: Fencing
void *occulus_realloc(void *restrict old_ptr, size_t size, const char *restrict filepath, uintmax_t line, const char *restrict function) {
  void *ptr = realloc(old_ptr, size);
  assert(ptr);
  for (uintmax_t t = 0; t < num_allocations; t++) {
    if (allocations[t].ptr == old_ptr && !allocations[t].freed) {
      mem_leaked -= allocations[t].size;
      allocations[t].freed = true;
    }
  }

  log_allocation(ptr, size, filepath, line, function);
  return ptr;
}

void occulus_free(void *restrict ptr, const char *restrict filepath, uintmax_t line, const char *restrict function) {
  assert(ptr);
  void *fenced_ptr = (ptr - 3);

  bool found = false;
  for (uintmax_t t = 0; t < num_allocations; t++) {
    if (allocations[t].ptr == fenced_ptr && !allocations[t].freed) {
      mem_leaked -= allocations[t].size;
      allocations[t].freed = true;
      found = true;

      for (uintmax_t u = 0; u < 3; u++) {
        if (((uint8_t*)fenced_ptr)[u] != fence[u] ||
            ((uint8_t*)fenced_ptr)[allocations[t].size + u + 3] != fence[u]) {
          printf("%s:%" PRIuMAX "/%s> fence failed on memory originally allocated at %s:%" PRIuMAX"/%s\n", filepath, line, function, allocations[t].filepath, allocations[t].line, allocations[t].function);
          break;
        }
      }

      break;
    }
  }

  if (!found) {
    printf("%s:%" PRIuMAX "/%s> free on unrecognized memory, 0x%" PRIuPTR "x\n", filepath, line, function, (uintptr_t)ptr);
  }

  free(fenced_ptr);
}

typedef struct {
  char filepath[FILEPATH_LENGTH];
  uintmax_t num_allocations;
  uintmax_t allocations_size;
  Allocation **allocations;
} OutputFileStats;
// FIXME: Prettify and merge stats
// Filepath NumAlloc MaxSize Leaked
// ...
// AppMaxSize AppLeaked
void occulus_print(bool detailed) {
  printf("MEM_DEBUG>\nMax: %.2fkb\tLeaked: %.2fkb\n", (double)max_mem / 1024.0, (double)mem_leaked / 1024.0);
  if (detailed) {
    const uintmax_t chunk = 10;
    uintmax_t stats_size = 10;
    uintmax_t num_stats = 0;
    OutputFileStats *stats = calloc(chunk, sizeof(OutputFileStats) * stats_size);

    for (uintmax_t t = 0; t < num_allocations; t++) {
      uintmax_t index = 0;
      bool found = false;
      for (uintmax_t u = 0; u < num_stats; u++) {
        if (strcmp(stats[u].filepath, allocations[t].filepath) == 0) {
          found = true;
          index = u;
          break;
        }
      }

      if (!found) {
        index = num_stats;
        stats[index] = (OutputFileStats){
          .filepath = { 0 },
          .num_allocations = 0,
          .allocations_size = chunk,
          .allocations = calloc(chunk, sizeof(Allocation*)),
        };
        strncpy(stats[index].filepath, allocations[t].filepath, FILEPATH_LENGTH);

        num_stats++;
        if (num_stats >= stats_size) {
          stats_size += chunk;
          stats = realloc(stats, sizeof(OutputFileStats) * stats_size);
        }
      }

      stats[index].allocations[stats[index].num_allocations] = &allocations[t];
      stats[index].num_allocations++;
      if (stats[index].num_allocations >= stats[index].allocations_size) {
        stats[index].allocations_size += chunk;
        stats[index].allocations = realloc(stats[index].allocations, sizeof(Allocation*) * stats[index].allocations_size);
      }
    }

    for (uintmax_t t = 0; t < num_stats; t++) {
      printf("%s\n", stats[t].filepath);

      for (uintmax_t u = 0; u < stats[t].num_allocations; u++) {
        printf("\t%s:%" PRIuMAX " #%.2fkb\n", stats[t].allocations[u]->function, stats[t].allocations[u]->line, (double)stats[t].allocations[u]->size / 1024.0);
      }
    }

    for (uintmax_t t = 0; t < num_stats; t++) {
      free(stats[t].allocations);
    }
    free(stats);
  }

  for (uintmax_t t = 0; t < num_allocations; t++) {
    if (!allocations[t].freed) {
      printf("Memory at %s:%" PRIuMAX "/%s never freed and leaked %.2fkb!\n", allocations[t].filepath, allocations[t].line, allocations[t].function, (double)allocations[t].size / 1024.0);
    }
  }
  printf("<MEM_DEBUG\n");
}
