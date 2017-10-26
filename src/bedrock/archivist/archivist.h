/**
 * Archivist - reading/writing files
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

bool archivist_read_file(const char *filepath, size_t *num_bytes, uint8_t **data);

typedef void (*IniCallback)(void *const userdata, const char *section, const char *key, const char *value);
bool archivist_read_ini_file(const char *filename, IniCallback callback, void *const userdata);
