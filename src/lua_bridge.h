#pragma once

typedef struct LuaBridge LuaBridge;

LuaBridge *lua_bridge_create(lua_State *state);
void lua_bridge_destroy(LuaBridge *const lua_bridge);
