#pragma once

#include <stdbool.h>
#include <stdint.h>

extern uint8_t map[40 * 30];

typedef struct {
  uint8_t offset;
  bool neighbors[9];
} AutoTile;
extern AutoTile auto_tiles[];
const uintmax_t num_auto_tiles;
