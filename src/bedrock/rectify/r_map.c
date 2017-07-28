#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "rectify.h"

typedef struct {
  char *key;
  void *val;
} RectifyMapKeyVal;

struct RectifyMap {
  RectifyMapKeyVal *key_vals;
};

RectifyMap *rectify_map_create(void) {
  RectifyMap *map = calloc(1, sizeof(RectifyMap));
  map->key_vals = rectify_array_alloc(10, sizeof(RectifyMapKeyVal));
  return map;
}

void rectify_map_destroy(RectifyMap **map) {
  if (!map || !*map) {
    return;
  }

  RectifyMap *dereffed = *map;
  for (uint32_t t = 0; t < rectify_array_size(dereffed->key_vals); t++) {
    free(dereffed->key_vals[t].key);
    free(dereffed->key_vals[t].val);
  }
  rectify_array_free(&dereffed->key_vals);
  free(*map);
  *map = NULL;
}

void rectify_map_set(RectifyMap *const map, const char *key, size_t value_size, void *const value) {
  if (!map) {
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(map->key_vals); t++) {
    if (strncmp(map->key_vals[t].key, key, 128) == 0) {
      free(map->key_vals[t].val);
      map->key_vals[t].val = rectify_memory_alloc_copy(value, value_size);
      return;
    }
  }
  RectifyMapKeyVal key_val = {
    .key = calloc(1, sizeof(char) * (strnlen(key, 128) + 1)),
    .val = calloc(1, value_size),
  };
  memcpy(key_val.key, key, sizeof(char) * (strnlen(key, 128) + 1));
  memcpy(key_val.val, value, value_size);
  map->key_vals = rectify_array_push(map->key_vals, &key_val);
}

void *const rectify_map_get(RectifyMap *const map, const char *key) {
  if (!map) {
    return NULL;
  }

  for (uint32_t t = 0; t < rectify_array_size(map->key_vals); t++) {
    if (strncmp(map->key_vals[t].key, key, 128) == 0) {
      return map->key_vals[t].val;
    }
  }

  return NULL;
}

void rectify_map_print(RectifyMap *const map) {
  if (!map) {
    return;
  }

  printf("RectifyMap: Debug\n");
  for (uint32_t t = 0; t < rectify_array_size(map->key_vals); t++) {
    printf("[%s], %.12s\n", map->key_vals[t].key, (char *)map->key_vals[t].val);
  }
  printf("/End\n");
}
