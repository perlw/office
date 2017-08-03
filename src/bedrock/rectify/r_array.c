#include "r_internal.h"

typedef struct {
  uintmax_t chunk_size;
  uintmax_t capacity;
  uintmax_t size;
  uintmax_t element_size;
} ArrayMetadata;

void *fence_ptr(void *unfenced_ptr) {
  uintptr_t metadata_size = (uintptr_t)sizeof(ArrayMetadata);
  return (void *)((uintptr_t)unfenced_ptr + metadata_size);
}

void *unfence_ptr(void *fenced_ptr) {
  uintptr_t metadata_size = (uintptr_t)sizeof(ArrayMetadata);
  return (void *)((uintptr_t)fenced_ptr - metadata_size);
}

ArrayMetadata *get_meta(void *fenced_ptr) {
  return (ArrayMetadata *)unfence_ptr(fenced_ptr);
}

void *get_element_ptr(void *fenced_ptr, uintmax_t index) {
  ArrayMetadata *meta = get_meta(fenced_ptr);
  return (void *)((uintptr_t)fenced_ptr + (uintptr_t)(meta->element_size * index));
}

void *resize_ptr(void *fenced_ptr) {
  uintptr_t metadata_size = (uintptr_t)sizeof(ArrayMetadata);
  ArrayMetadata *meta = get_meta(fenced_ptr);
  void *unfenced_ptr = unfence_ptr(fenced_ptr);
  unfenced_ptr = realloc(unfenced_ptr, (meta->capacity * meta->element_size) + metadata_size);
  return fence_ptr(unfenced_ptr);
}

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
  return fence_ptr(ptr);
}

void rectify_array_free(void **ptr) {
  assert(ptr && *ptr);

  free(unfence_ptr(*ptr));
  *ptr = NULL;
}

void *rectify_array_push(void *ptr, const void *data) {
  assert(ptr);
  assert(data);

  ArrayMetadata *meta = get_meta(ptr);
  memcpy(get_element_ptr(ptr, meta->size), data, meta->element_size);

  meta->size++;
  if (meta->size >= meta->capacity) {
    meta->capacity += meta->chunk_size;
    ptr = resize_ptr(ptr);
  }

  return ptr;
}

void *rectify_array_delete(void *ptr, uintmax_t index) {
  assert(ptr);

  ArrayMetadata *meta = get_meta(ptr);
  assert(index < meta->size);

  if (meta->size > 1 && index < meta->size - 1) {
    void *index_ptr = get_element_ptr(ptr, index);
    void *next_ptr = get_element_ptr(ptr, index + 1);
    memcpy(index_ptr, next_ptr, meta->element_size * (meta->size - index - 1));
  }

  meta->size--;
  return ptr;
}

uintmax_t rectify_array_cap(const void *const ptr) {
  assert(ptr);

  void *unfenced_ptr = (void *)((uintptr_t)ptr - sizeof(ArrayMetadata));
  ArrayMetadata *meta = (ArrayMetadata *)unfenced_ptr;
  return meta->capacity;
}

uintmax_t rectify_array_size(const void *const ptr) {
  assert(ptr);

  void *unfenced_ptr = (void *)((uintptr_t)ptr - sizeof(ArrayMetadata));
  ArrayMetadata *meta = (ArrayMetadata *)unfenced_ptr;
  return meta->size;
}
