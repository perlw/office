#pragma once

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "messages.h"

typedef struct UIWindow UIWindow;
typedef struct UIWidgetRuneSelector UIWidgetRuneSelector;
typedef struct UIDialogRuneSelector UIDialogRuneSelector;

struct UIWindow {
  double timing;
  double since_update;

  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;

  Surface *surface;

  GossipHandle system_handle;
  GossipHandle mouse_handle;
};

struct UIWidgetRuneSelector {
  UIWindow *parent;

  uint8_t chosen_rune;
  GossipHandle event_handle;
  GossipHandle mouse_event_handle;
};

struct UIDialogRuneSelector {
  UIWindow *window;
  UIWidgetRuneSelector *rune_selector;
};
