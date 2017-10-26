#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "occulus/occulus.h"

#include "archivist.h"

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

bool archivist_read_ini_file(const char *filename, IniCallback callback, void *const userdata) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("Archivist: INI - Failed to open file\n");
    return false;
  }

  uint32_t line = 0;
  bool has_section = false;
  char section[1024] = { 0 };
  while (!feof(file)) {
    char buffer[1024] = { 0 };
    if (!fgets(buffer, 1024, file)) {
      break;
    }

    line++;
    if (buffer[0] == ';') {
      continue;
    } else if (buffer[0] == '[') {
      uint32_t start = 1;
      uint32_t end = 1;
      for (uint32_t t = 0; t < 1024; t++, end++) {
        if (buffer[t] == '\0' || buffer[t] == '\n') {
          printf("Archivist: INI - parse error, broken section title on line %d\n", line);
          break;
        }
        if (buffer[t] == ']') {
          end = t;
          break;
        }
      }

      if (start != end) {
        memset(section, 0, 1024);
        strncpy(section, &buffer[1], end - start);
        has_section = true;
      }
    } else if (!has_section) {
      printf("Archivist: INI - parse error, missing initial section before line %d\n", line);
      break;
    } else {
      int32_t key_start = -1;
      int32_t key_end = -1;
      int32_t value_start = -1;
      int32_t value_end = -1;
      bool has_key = false;
      bool has_value = false;
      for (uint32_t t = 0; t < 1024; t++) {
        if (buffer[t] > 32 && buffer[t] < 127 && buffer[t] != '=') {
          if (!has_key && key_start == -1) {
            key_start = t;
          } else if (has_key && !has_value && value_start == -1) {
            value_start = t;
          }
        } else if (buffer[t] == '=') {
          if (has_key) {
            printf("Archivist: INI - parse error, already parsed key on line %d\n", line);
            break;
          }
          key_end = t;
          has_key = true;
        } else {
          if (buffer[t] == '\0' || buffer[t] == '\n') {
            if (!has_key) {
              if (key_start < 0) {
                continue;
              }

              printf("Archivist: INI - parse error, broken keyval on line %d\n", line);
              break;
            }

            has_value = true;
            value_end = t;
            break;
          }
        }
      }

      if (has_key && has_value) {
        char key[1024] = { 0 };
        char value[1024] = { 0 };
        memset(key, 0, 1024);
        strncpy(key, &buffer[key_start], key_end - key_start);
        memset(value, 0, 1024);
        strncpy(value, &buffer[value_start], value_end - value_start);

        callback(userdata, section, key, value);
      }
    }
  }
  fclose(file);

  return true;
}
