#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "bedrock/bedrock.h"

typedef struct {
  uint32_t res_width;
  uint32_t res_height;
  bool gl_debug;
  uint32_t frame_lock;
  uint32_t ascii_width;
  uint32_t ascii_height;
} Config;

typedef enum {
  INPUT_ACTION_CLOSE = GOSSIP_ID_LAST + 1,
  INPUT_ACTION_TEST,
} InputAction;

Config read_config(void);
