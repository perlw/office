#pragma once

#include <stdbool.h>

bool archivist_read_file(const char *filepath, size_t *num_bytes, uint8_t **data);
