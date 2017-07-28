#pragma once

#include <stdint.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"

extern KronosSystem system_ui;

typedef uintptr_t UIWindowHandle;

typedef struct {
  char *title;
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;

  int32_t scroll_x;
  int32_t scroll_y;

  Surface *surface;

  GossipHandle mouse_handle;

  UIWindowHandle handle;
} UIWindow;

typedef struct {
  char *title;
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
} UIEventCreateWindow;

typedef struct {
  UIWindow *window;
  int32_t x;
  int32_t y;
  Glyph glyph;
} UIEventWindowGlyph;

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

typedef struct {
  UIWindow *target;
  int32_t scroll_x;
  int32_t scroll_y;
  uint32_t x;
  uint32_t y;
} UIEventScroll;
