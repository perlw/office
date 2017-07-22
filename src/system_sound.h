#pragma once

#include <stdint.h>

#include "bedrock/bedrock.h"

typedef struct {
  uint32_t song_id;
  float left[2048];
  float right[2048];
} Spectrum;

extern KronosSystem system_sound;
