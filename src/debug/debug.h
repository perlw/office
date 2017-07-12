#pragma once

typedef struct DebugOverlay DebugOverlay;

DebugOverlay *debugoverlay_create(void);
void debugoverlay_destroy(DebugOverlay *overlay);
