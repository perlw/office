#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint32_t res_width;
  uint32_t res_height;
  bool gl_debug;
  uint32_t frame_lock;
  uint32_t ascii_width;
  uint32_t ascii_height;
  double grid_size_width;
  double grid_size_height;
} Config;

const Config *const config_init(void);
const Config *const config_get(void);
