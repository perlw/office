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
  MSG_GAME_INIT = GOSSIP_ID_LAST + 1,
  MSG_KEY_PRESS,
} Messages;

Config read_config(void);
