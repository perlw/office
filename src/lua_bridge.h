#pragma once

#include "bedrock/bedrock.h"

typedef enum {
  LUA_ACTION = GOSSIP_ID_ALL + 1,
} LuaEvent;

typedef struct LuaBridge LuaBridge;

LuaBridge *lua_bridge_create(void);
void lua_bridge_destroy(LuaBridge *const lua_bridge);
