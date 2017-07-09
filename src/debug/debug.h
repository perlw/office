#pragma once

#include "config.h"

typedef struct DebugOverlay DebugOverlay;

DebugOverlay *debugoverlay_create(const Config *config);
void debugoverlay_destroy(DebugOverlay *overlay);
