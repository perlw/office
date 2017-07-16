#pragma once

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"

typedef struct UIWindow UIWindow;

struct UIWindow {
  double timing;
  double since_update;

  char *title;
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;

  Surface *surface;

  GossipHandle mouse_handle;
};
