#pragma once

#include "config.h"

typedef struct DebugOverlay DebugOverlay;

DebugOverlay *debugoverlay_create(const Config *config);
void debugoverlay_destroy(DebugOverlay *overlay);

void debugoverlay_update(DebugOverlay *overlay, double dt);
void debugoverlay_draw(DebugOverlay *overlay);
