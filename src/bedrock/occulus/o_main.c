#include <inttypes.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILEPATH_LENGTH 128
#define FUNCTION_LENGTH 128

typedef enum {
  MEM_ACTION_ALLOC = 0x1,
  MEM_ACTION_FREE,
} MemoryActionType;

typedef struct {
  uintptr_t ptr;
  uintmax_t size;
  uint32_t action;

  char filepath[FILEPATH_LENGTH];
  char function[FUNCTION_LENGTH];
  uintmax_t line;
} MemoryAction;

void occulus_assert(void *ptr, const char *filepath, uintmax_t line, const char *function) {
  if (!ptr) {
    printf("%s:%" PRIuMAX "/%s> assert failed on %p.\n", filepath, line, function, ptr);
    exit(-1);
  }
}

void occulus_log_action(MemoryActionType action, uintptr_t ptr, size_t size, const char *filepath, uintmax_t line, const char *function) {
  FILE *log_file = fopen("mem.dbg", "a+b");

  MemoryAction mem_action = (MemoryAction){
    .ptr = ptr,
    .size = size,
    .action = action,
    .filepath = { 0 },
    .function = { 0 },
    .line = line,
  };
  strncpy(mem_action.filepath, filepath, FILEPATH_LENGTH);
  strncpy(mem_action.function, function, FUNCTION_LENGTH);

  if (fwrite(&mem_action, sizeof(MemoryAction), 1, log_file) != 1) {
    printf("%s:%" PRIuMAX "/%s> Couldn't log allocation.\n", filepath, line, function);
  }

  fclose(log_file);
}

void *occulus_malloc(size_t size, const char *filepath, uintmax_t line, const char *function) {
  void *ptr = malloc(size);

  occulus_assert(ptr, filepath, line, function);
  occulus_log_action(MEM_ACTION_ALLOC, (uintptr_t)ptr, size, filepath, line, function);

  return ptr;
}

void *occulus_calloc(size_t num, size_t size, const char *filepath, uintmax_t line, const char *function) {
  size_t total = num * size;
  void *ptr = calloc(num, total);

  occulus_assert(ptr, filepath, line, function);
  occulus_log_action(MEM_ACTION_ALLOC, (uintptr_t)ptr, size, filepath, line, function);

  return ptr;
}

void *occulus_realloc(void *old_ptr, size_t size, const char *filepath, uintmax_t line, const char *function) {
  void *ptr = realloc(old_ptr, size);

  occulus_assert(ptr, filepath, line, function);
  occulus_log_action(MEM_ACTION_FREE, (uintptr_t)old_ptr, 0, filepath, line, function);
  occulus_log_action(MEM_ACTION_ALLOC, (uintptr_t)ptr, size, filepath, line, function);

  return ptr;
}

void occulus_free(void *ptr, const char *filepath, uintmax_t line, const char *function) {
  occulus_assert(ptr, filepath, line, function);
  occulus_log_action(MEM_ACTION_FREE, (uintptr_t)ptr, 0, filepath, line, function);
  free(ptr);
}

void occulus_print(void) {
  FILE *log_file = fopen("mem.dbg", "rb");
  MemoryAction mem_action;
  for (;;) {
    if (fread(&mem_action, sizeof(MemoryAction), 1, log_file) != 1) {
      break;
    }

    if (mem_action.action == MEM_ACTION_ALLOC) {
      uintmax_t orig_pos = ftell(log_file);

      bool found = false;
      for (;;) {
        MemoryAction check_action;
        if (fread(&check_action, sizeof(MemoryAction), 1, log_file) != 1) {
          break;
        }

        if (mem_action.ptr == check_action.ptr) {
          switch (check_action.action) {
            case MEM_ACTION_ALLOC:
              printf("%s:%" PRIuMAX "/%s> alloc on used memory %p.\n", check_action.filepath, check_action.line, check_action.function, (void *)mem_action.ptr);
              break;

            case MEM_ACTION_FREE:
              found = true;
              break;
          }
          break;
        }
      }

      if (!found) {
        printf("%s:%" PRIuMAX "/%s> never freed %p.\n", mem_action.filepath, mem_action.line, mem_action.function, (void *)mem_action.ptr);
      }

      fseek(log_file, orig_pos, SEEK_SET);
    }
  }
  fclose(log_file);
}
