#pragma once

#define USE_KRONOS
#define USE_PICASSO
#include "bedrock/bedrock.h"

// Temporary, need to find solution
bool system_game_should_kill(void);
void system_input_bind_input(PicassoWindow *window);

extern KronosSystem system_debug;
extern KronosSystem system_game;
extern KronosSystem system_input;
extern KronosSystem system_lua_bridge;
extern KronosSystem system_sound;
extern KronosSystem system_ui;

extern KronosSystem systems;
