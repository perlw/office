#ifndef __ARCHIVIST_H__
#define __ARCHIVIST_H__

#include <stdbool.h>

bool archivist_read_file(const char *filepath, uint8_t **data, size_t *num_bytes);

#endif // __ARCHIVIST_H__
