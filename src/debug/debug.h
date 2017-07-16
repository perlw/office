#pragma once

#include <ascii/ascii.h>

typedef struct DebugOverlay DebugOverlay;

DebugOverlay *debugoverlay_create(void);
void debugoverlay_destroy(DebugOverlay *overlay);

void debugoverlay_update(DebugOverlay *overlay, double delta);
void debugoverlay_draw(DebugOverlay *overlay, AsciiBuffer *const screen);
