#pragma once

typedef struct {
  uint32_t res_width;
  uint32_t res_height;
} Config;

Config read_config(void);
