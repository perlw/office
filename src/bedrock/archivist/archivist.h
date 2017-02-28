#pragma once

#include <stdbool.h>

bool archivist_read_file(const char *filepath, uint8_t **data, size_t *num_bytes);
