#include <inttypes.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILEPATH_LENGTH 128
#define FUNCTION_LENGTH 128

typedef struct {
  uintmax_t size;
} PtrMetadata;
#define FENCE 3
#define FENCE2 6
const uint8_t fence[FENCE] = { 'O', 'C', 'C' };

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

void *occulus_internal_fence_ptr(void *unfenced_ptr) {
  uintptr_t metadata_size = (uintptr_t)sizeof(PtrMetadata);
  return (void *)((uintptr_t)unfenced_ptr + metadata_size + FENCE);
}

void *occulus_internal_unfence_ptr(void *fenced_ptr) {
  uintptr_t metadata_size = (uintptr_t)sizeof(PtrMetadata);
  return (void *)((uintptr_t)fenced_ptr - metadata_size - FENCE);
}

void occulus_internal_add_fence_check(void *unfenced_ptr) {
  uintptr_t metadata_size = (uintptr_t)sizeof(PtrMetadata);
  size_t size = ((PtrMetadata *)unfenced_ptr)->size;
  memcpy((void *)((uintptr_t)unfenced_ptr + metadata_size), fence, FENCE);
  memcpy((void *)((uintptr_t)unfenced_ptr + metadata_size + FENCE + size), fence, FENCE);
}

void occulus_assert(void *ptr, const char *filepath, uintmax_t line, const char *function) {
  if (!ptr) {
    printf("%s:%" PRIuMAX "/%s> assert failed on %p.\n", filepath, line, function, ptr);
    exit(-1);
  }
}

void occulus_assert_fence(void *ptr, const char *filepath, uintmax_t line, const char *function) {
  void *unfenced_ptr = occulus_internal_unfence_ptr(ptr);

  uintptr_t metadata_size = (uintptr_t)sizeof(PtrMetadata);
  size_t size = ((PtrMetadata *)unfenced_ptr)->size;
  char *fence_a = (void *)((uintptr_t)unfenced_ptr + metadata_size);
  char *fence_b = (void *)((uintptr_t)unfenced_ptr + metadata_size + FENCE + size);

  if (memcmp(fence_a, fence, 3) != 0 || memcmp(fence_b, fence, 3) != 0) {
    printf("%s:%" PRIuMAX "/%s> fence failed on %p.\n", filepath, line, function, ptr);
  }
}

uintmax_t max_mem = 0;
intmax_t currently_allocated = 0;

#ifdef OCCULUS_LOG_MEM
FILE *mem_log_file = NULL;
#endif // OCCULUS_LOG_MEM

void occulus_log_action(MemoryActionType action, uintptr_t ptr, size_t size, const char *filepath, uintmax_t line, const char *function) {
#ifdef OCCULUS_LOG_MEM
  if (!mem_log_file) {
    mem_log_file = fopen("mem.dbg", "a+b");
  }

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

  if (fwrite(&mem_action, sizeof(MemoryAction), 1, mem_log_file) != 1) {
    printf("%s:%" PRIuMAX "/%s> Couldn't log allocation.\n", filepath, line, function);
  }
#endif // OCCULUS_LOG_MEM

  switch (action) {
    case MEM_ACTION_ALLOC:
      currently_allocated += size;
      max_mem = ((uintmax_t)currently_allocated > max_mem ? (uintmax_t)currently_allocated : max_mem);
      break;

    case MEM_ACTION_FREE:
      if (size > (uintmax_t)currently_allocated) {
        printf("%s:%" PRIuMAX "/%s> Freeing more memory than allocated (%" PRIuMAX " > %" PRIuMAX ").\n", filepath, line, function, size, currently_allocated);
      }
      currently_allocated -= size;
      break;
  }
}

void *occulus_malloc(size_t size, const char *filepath, uintmax_t line, const char *function) {
  uintptr_t metadata_size = (uintptr_t)sizeof(PtrMetadata);
  void *ptr = malloc(size + metadata_size + FENCE2);
  ((PtrMetadata *)ptr)->size = size;

  occulus_internal_add_fence_check(ptr);

  void *fenced_ptr = occulus_internal_fence_ptr(ptr);
  occulus_assert(fenced_ptr, filepath, line, function);
  occulus_log_action(MEM_ACTION_ALLOC, (uintptr_t)fenced_ptr, size, filepath, line, function);

  return fenced_ptr;
}

void *occulus_calloc(size_t num, size_t size, const char *filepath, uintmax_t line, const char *function) {
  uintptr_t metadata_size = (uintptr_t)sizeof(PtrMetadata);
  size_t total = num * size;
  void *ptr = calloc(1, total + metadata_size + FENCE2);
  ((PtrMetadata *)ptr)->size = total;

  occulus_internal_add_fence_check(ptr);

  void *fenced_ptr = occulus_internal_fence_ptr(ptr);
  occulus_assert(fenced_ptr, filepath, line, function);
  occulus_log_action(MEM_ACTION_ALLOC, (uintptr_t)fenced_ptr, total, filepath, line, function);

  return fenced_ptr;
}

void *occulus_realloc(void *old_ptr, size_t size, const char *filepath, uintmax_t line, const char *function) {
  occulus_assert_fence(old_ptr, filepath, line, function);

  void *unfenced_ptr = occulus_internal_unfence_ptr(old_ptr);

  size_t old_size = ((PtrMetadata *)unfenced_ptr)->size;
  uintptr_t metadata_size = (uintptr_t)sizeof(PtrMetadata);
  void *ptr = realloc(unfenced_ptr, size + metadata_size + FENCE2);
  ((PtrMetadata *)ptr)->size = size;

  occulus_internal_add_fence_check(ptr);

  void *fenced_ptr = occulus_internal_fence_ptr(ptr);
  occulus_assert(fenced_ptr, filepath, line, function);
  occulus_log_action(MEM_ACTION_FREE, (uintptr_t)old_ptr, old_size, filepath, line, function);
  occulus_log_action(MEM_ACTION_ALLOC, (uintptr_t)fenced_ptr, size, filepath, line, function);

  return fenced_ptr;
}

void occulus_free(void *ptr, const char *filepath, uintmax_t line, const char *function) {
  void *unfenced_ptr = occulus_internal_unfence_ptr(ptr);

  occulus_assert(ptr, filepath, line, function);
  occulus_assert_fence(ptr, filepath, line, function);
  occulus_log_action(MEM_ACTION_FREE, (uintptr_t)ptr, ((PtrMetadata *)unfenced_ptr)->size, filepath, line, function);

  free(unfenced_ptr);
}

void occulus_init(void) {
#ifdef OCCULUS_LOG_MEM
  remove("mem.dbg");
#endif // OCCULUS_LOG_MEM
}

void occulus_print(void) {
  printf("\n-=-=[OCCULUS]=-=-\nMax: %.2fkb\tLeaked: %.2fkb\n-=-===========-=-\n\n", (double)max_mem / 1024.0, (double)currently_allocated / 1024.0);

#ifdef OCCULUS_LOG_MEM
  if (mem_log_file) {
    fclose(mem_log_file);
  }

  mem_log_file = fopen("mem.dbg", "rb");
  MemoryAction mem_action;
  for (;;) {
    if (fread(&mem_action, sizeof(MemoryAction), 1, mem_log_file) != 1) {
      break;
    }

    if (mem_action.action == MEM_ACTION_ALLOC) {
      uintmax_t orig_pos = ftell(mem_log_file);

      bool found = false;
      for (;;) {
        MemoryAction check_action;
        if (fread(&check_action, sizeof(MemoryAction), 1, mem_log_file) != 1) {
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

      fseek(mem_log_file, orig_pos, SEEK_SET);
    }
  }
  fclose(mem_log_file);
#endif // OCCULUS_LOG_MEM
}

intmax_t occulus_current_usage(void) {
  return currently_allocated;
}
