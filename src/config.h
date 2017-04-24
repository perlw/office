#pragma once

#include <stdbool.h>
#include <stdint.h>

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
  MSG_GAME_INIT = 0x001,
  MSG_GAME_KILL,

  MSG_SOUND_PLAY_TAP,

  MSG_INPUT_KEYBOARD,
} Messages;

Config read_config(void);
