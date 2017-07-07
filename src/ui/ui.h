#pragma once

#include <stdint.h>

#include "ascii/ascii.h"
#include "u_dialogs.h"
#include "u_types.h"
#include "u_widgets.h"

// +UIWindow
UIWindow *ui_window_create(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void ui_window_destroy(UIWindow *const window);

void ui_window_glyph(UIWindow *const window, uint32_t x, uint32_t y, Glyph glyph);

void ui_window_update(UIWindow *const window, double delta);
void ui_window_draw(UIWindow *const window, AsciiBuffer *const ascii);
// -UIWindow
