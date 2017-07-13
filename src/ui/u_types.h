#pragma once

#include "bedrock/bedrock.h"

#ifndef UI_INTERNAL
typedef struct UIWindow UIWindow;
typedef struct UIWidgetRuneSelector UIWidgetRuneSelector;
typedef struct UIDialogRuneSelector UIDialogRuneSelector;
#else
#include "u_internal.h"
#endif

typedef enum {
  UI_WINDOW_EVENT_MOUSEMOVE = GOSSIP_ID_ALL + 1,
  UI_WINDOW_EVENT_CLICK,
} UIWindowEvent;

typedef struct {
  uint32_t x;
  uint32_t y;
} UIEventMouseMove;

typedef struct {
  uint32_t x;
  uint32_t y;
} UIEventClick;

typedef enum {
  UI_WIDGET_EVENT_PAINT = GOSSIP_ID_ALL + 1,

  UI_WIDGET_RUNE_SELECTOR_SELECTED,
} UIWidgetEvent;
