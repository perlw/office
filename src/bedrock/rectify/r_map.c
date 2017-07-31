#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "rectify.h"

struct RectifyMap {
  RectifyMapItem *items;
};

RectifyMap *rectify_map_create(void) {
  RectifyMap *map = calloc(1, sizeof(RectifyMap));
  map->items = rectify_array_alloc(10, sizeof(RectifyMapItem));
  return map;
}

void rectify_map_destroy(RectifyMap **map) {
  if (!map || !*map) {
    return;
  }

  RectifyMap *dereffed = *map;
  for (uint32_t t = 0; t < rectify_array_size(dereffed->items); t++) {
    free(dereffed->items[t].key);
    free(dereffed->items[t].val);
  }
  rectify_array_free(&dereffed->items);
  free(*map);
  *map = NULL;
}

void rectify_map_set(RectifyMap *const map, const char *key, RectifyMapType type, size_t value_size, void *const value) {
  if (!map) {
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(map->items); t++) {
    if (strncmp(map->items[t].key, key, 128) == 0) {
      free(map->items[t].val);
      map->items[t].val = rectify_memory_alloc_copy(value, value_size);
      return;
    }
  }
  RectifyMapItem item = {
    .key = calloc(1, sizeof(char) * (strnlen(key, 128) + 1)),
    .val = calloc(1, value_size),
    .size = value_size,
    .type = type,
  };
  memcpy(item.key, key, sizeof(char) * (strnlen(key, 128) + 1));
  memcpy(item.val, value, value_size);
  map->items = rectify_array_push(map->items, &item);
}

void *const rectify_map_get(RectifyMap *const map, const char *key) {
  if (!map) {
    return NULL;
  }

  for (uint32_t t = 0; t < rectify_array_size(map->items); t++) {
    if (strncmp(map->items[t].key, key, 128) == 0) {
      return map->items[t].val;
    }
  }

  return NULL;
}

RectifyMapIter rectify_map_iter(RectifyMap *const map) {
  return (RectifyMapIter){
    .map = map,
    .index = 0,
  };
}

bool rectify_map_iter_next(RectifyMapIter *const iter, RectifyMapItem *item) {
  if (!iter || !iter->map || !item) {
    return false;
  }

  if (iter->index >= rectify_array_size(iter->map->items)) {
    return false;
  }

  RectifyMapItem *map_item = &iter->map->items[iter->index];
  memcpy(item, map_item, sizeof(RectifyMapItem));
  iter->index++;

  return true;
}

void rectify_map_print(RectifyMap *const map) {
  if (!map) {
    return;
  }

  printf("RectifyMap {\n");
  for (uint32_t t = 0; t < rectify_array_size(map->items); t++) {
    RectifyMapItem *const item = &map->items[t];
    printf("\t[%s] = ", item->key);
    switch (item->type) {
      case RECTIFY_MAP_TYPE_BYTE: {
        uint8_t val = *(uint8_t *)item->val;
        printf("%d|%c", val, val);
        break;
      }

      case RECTIFY_MAP_TYPE_BOOL: {
        bool val = *(bool *)item->val;
        printf("%s", (val ? "true" : "false"));
        break;
      }

      case RECTIFY_MAP_TYPE_UINT: {
        uint32_t val = *(uint32_t *)item->val;
        printf("%d", val);
        break;
      }

      case RECTIFY_MAP_TYPE_INT: {
        int32_t val = *(int32_t *)item->val;
        printf("%d", val);
        break;
      }

      case RECTIFY_MAP_TYPE_FLOAT: {
        float val = *(float *)item->val;
        printf("%f", val);
        break;
      }

      case RECTIFY_MAP_TYPE_DOUBLE: {
        double val = *(double *)item->val;
        printf("%f", val);
        break;
      }

      case RECTIFY_MAP_TYPE_STRING: {
        char *val = (char *)item->val;
        printf("%s", val);
        break;
      }

      case RECTIFY_MAP_TYPE_PTR:
      default: {
        void *val = item->val;
        printf("%p", val);
        break;
      }
    }
    printf("\n");
  }
  printf("}\n");
}
