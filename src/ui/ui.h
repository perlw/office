#pragma once

#include <stdint.h>

#include "ascii/ascii.h"

typedef enum {
  UI_WINDOW_EVENT_MOUSEMOVE = GOSSIP_ID_ALL + 1,
  UI_WINDOW_EVENT_CLICK,
  UI_WINDOW_EVENT_PAINT,
} UIWindowEvent;

typedef struct {
  uint32_t x;
  uint32_t y;
} UIEventMouseMove;

typedef struct {
  uint32_t x;
  uint32_t y;
} UIEventClick;

// +UIWindow
typedef struct UIWindow UIWindow;
typedef void (*UIWindowEventCallback)(UIWindow *const window, UIWindowEvent event, void *const eventdata, void *const userdata);

UIWindow *ui_window_create(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void ui_window_destroy(UIWindow *const window);

void ui_window_glyph(UIWindow *const window, uint32_t x, uint32_t y, Glyph glyph);

void ui_window_update(UIWindow *const window, double delta);
void ui_window_draw(UIWindow *const window, AsciiBuffer *const ascii);
// -UIWindow
