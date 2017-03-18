#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "occulus/occulus.h"

bool archivist_read_file(const char *filepath, size_t *num_bytes, uint8_t **data) {
  FILE *file = fopen(filepath, "rb");
  if (!file) {
    *data = NULL;
    *num_bytes = 0;
    return false;
  }

  fseek(file, 0, SEEK_END);
  *num_bytes = ftell(file);
  fseek(file, 0, SEEK_SET);
  *data = calloc(1, *num_bytes);
  fread(*data, *num_bytes, 1, file);
  fclose(file);

  return true;
}
