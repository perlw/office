/**
 * Rectify - datastructures and memory
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

// +array
void *rectify_array_alloc(uintmax_t chunk_size, uintmax_t element_size);
void rectify_array_free(void **ptr);

void *rectify_array_push(void *ptr, const void *data);
void *rectify_array_delete(void *ptr, uintmax_t index);

uintmax_t rectify_array_cap(const void *const ptr);
uintmax_t rectify_array_size(const void *const ptr);
// -array

// +map
typedef enum {
  RECTIFY_MAP_TYPE_BYTE = 1,
  RECTIFY_MAP_TYPE_BOOL,
  RECTIFY_MAP_TYPE_UINT,
  RECTIFY_MAP_TYPE_INT,
  RECTIFY_MAP_TYPE_FLOAT,
  RECTIFY_MAP_TYPE_DOUBLE,
  RECTIFY_MAP_TYPE_STRING,
  RECTIFY_MAP_TYPE_MAP,
  RECTIFY_MAP_TYPE_PTR,
} RectifyMapType;

typedef struct RectifyMap RectifyMap;

typedef struct {
  char *key;
  void *val;
  size_t size;
  RectifyMapType type;
} RectifyMapItem;

typedef struct {
  RectifyMap *map;
  uint32_t index;
} RectifyMapIter;

RectifyMap *rectify_map_create(void);
void rectify_map_destroy(RectifyMap **map);

void rectify_map_set(RectifyMap *const map, const char *key, RectifyMapType type, size_t value_size, void *const value);
void rectify_map_set_byte(RectifyMap *const map, const char *key, uint8_t value);
void rectify_map_set_bool(RectifyMap *const map, const char *key, bool value);
void rectify_map_set_uint(RectifyMap *const map, const char *key, uint32_t value);
void rectify_map_set_int(RectifyMap *const map, const char *key, int32_t value);
void rectify_map_set_float(RectifyMap *const map, const char *key, float value);
void rectify_map_set_double(RectifyMap *const map, const char *key, double value);
void rectify_map_set_string(RectifyMap *const map, const char *key, char *const value);
void rectify_map_set_map(RectifyMap *const map, const char *key, RectifyMap *const value);

void *const rectify_map_get(RectifyMap *const map, const char *key);
uint8_t rectify_map_get_byte(RectifyMap *const map, const char *key);
bool rectify_map_get_bool(RectifyMap *const map, const char *key);
uint32_t rectify_map_get_uint(RectifyMap *const map, const char *key);
int32_t rectify_map_get_int(RectifyMap *const map, const char *key);
float rectify_map_get_float(RectifyMap *const map, const char *key);
double rectify_map_get_double(RectifyMap *const map, const char *key);
char *const rectify_map_get_string(RectifyMap *const map, const char *key);
RectifyMap *const rectify_map_get_map(RectifyMap *const map, const char *key);

RectifyMapIter rectify_map_iter(RectifyMap *const map);
bool rectify_map_iter_next(RectifyMapIter *const iter, RectifyMapItem *item);

void rectify_map_print(RectifyMap *const map);
// -map

// +memory
#ifndef RECTIFY_DEBUG
void *rectify_memory_alloc_copy(const void *ptr, uintmax_t size);
#else
void *rectify_memory_alloc_copy_debug(const void *ptr, uintmax_t size, const char *filepath, uintmax_t line, const char *function);
#define rectify_memory_alloc_copy(a, b) rectify_memory_alloc_copy_debug(a, b, __FILE__, __LINE__, __func__)
#endif
// +memory
