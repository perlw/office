#pragma once

#include "bedrock/bedrock.h"

typedef struct LuaBridge LuaBridge;

LuaBridge *lua_bridge_create(void);
void lua_bridge_destroy(LuaBridge *const lua_bridge);
