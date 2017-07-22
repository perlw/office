#pragma once

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "ui/ui.h"

typedef struct LuaBridge LuaBridge;

LuaBridge *lua_bridge_create(void);
void lua_bridge_destroy(LuaBridge *const lua_bridge);

void lua_bridge_update(LuaBridge *const lua_bridge, double delta);
