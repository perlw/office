#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint32_t res_width;
  uint32_t res_height;
  bool gl_debug;
} Config;

Config read_config(void);
