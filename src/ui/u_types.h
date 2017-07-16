#pragma once

#include "bedrock/bedrock.h"

#ifndef UI_INTERNAL
typedef struct UIWindow UIWindow;
typedef struct UIWidgetRuneSelector UIWidgetRuneSelector;
typedef struct UIDialogRuneSelector UIDialogRuneSelector;
#else
#include "u_internal.h"
#endif

typedef struct {
  UIWindow *target;
  uint32_t x;
  uint32_t y;
} UIEventMouseMove;

typedef struct {
  UIWindow *target;
  uint32_t x;
  uint32_t y;
} UIEventClick;
