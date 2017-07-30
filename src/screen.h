#pragma once

#include "ascii/ascii.h"

typedef void (*ScreenRender)(AsciiBuffer *const screen, void *const userdata);

void screen_init(void);
void screen_kill(void);

void screen_hook_render(ScreenRender render_func, void *const userdata, uint32_t layer);
void screen_unhook_render(ScreenRender render_func, void *const userdata);

void screen_render(void);
