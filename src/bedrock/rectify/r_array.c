#include "r_internal.h"

typedef struct {
  uintmax_t chunk_size;
  uintmax_t capacity;
  uintmax_t size;
  uintmax_t element_size;
} ArrayMetadata;

void *rectify_array_alloc(uintmax_t chunk_size, uintmax_t element_size) {
  ArrayMetadata meta = {
    .chunk_size = chunk_size,
    .capacity = chunk_size,
    .size = 0,
    .element_size = element_size,
  };

  uintptr_t metadata_size = (uintptr_t)sizeof(ArrayMetadata);

  void *ptr = calloc(1, (chunk_size * element_size) + metadata_size);
  memcpy(ptr, &meta, metadata_size);
  return (void *)((uintptr_t)ptr + metadata_size);
}

void rectify_array_free(void *ptr) {
  assert(ptr);

  void *unfenced_ptr = (void *)((uintptr_t)ptr - sizeof(ArrayMetadata));
  free(unfenced_ptr);
}

void *rectify_array_push(void *ptr, const void *data) {
  assert(ptr);
  assert(data);

  uintptr_t metadata_size = (uintptr_t)sizeof(ArrayMetadata);

  void *unfenced_ptr = (void *)((uintptr_t)ptr - metadata_size);
  ArrayMetadata *meta = (ArrayMetadata *)unfenced_ptr;

  void *element_ptr = (void *)((uintptr_t)ptr + (uintptr_t)(meta->element_size * meta->size));
  memcpy(element_ptr, data, meta->element_size);

  meta->size++;
  if (meta->size >= meta->capacity) {
    meta->capacity += meta->chunk_size;
    unfenced_ptr = realloc(unfenced_ptr, (meta->capacity * meta->element_size) + metadata_size);
  }

  return (void *)((uintptr_t)unfenced_ptr + metadata_size);
}

uintmax_t rectify_array_cap(void *ptr) {
  assert(ptr);

  void *unfenced_ptr = (void *)((uintptr_t)ptr - sizeof(ArrayMetadata));
  ArrayMetadata *meta = (ArrayMetadata *)unfenced_ptr;
  return meta->capacity;
}

uintmax_t rectify_array_size(void *ptr) {
  assert(ptr);

  void *unfenced_ptr = (void *)((uintptr_t)ptr - sizeof(ArrayMetadata));
  ArrayMetadata *meta = (ArrayMetadata *)unfenced_ptr;
  return meta->size;
}
