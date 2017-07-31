#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include "bedrock/bedrock.h"

#include "messages.h"

bool system_lua_bridge_start(void);
void system_lua_bridge_stop(void);
void system_lua_bridge_message(uint32_t id, RectifyMap *const map);

KronosSystem system_lua_bridge = {
  .name = "lua_bridge",
  .frames = 1,
  .autostart = true,
  .start = &system_lua_bridge_start,
  .stop = &system_lua_bridge_stop,
  .update = NULL,
  .message = &system_lua_bridge_message,
};

typedef struct {
  lua_State *state;
  int32_t *listeners;
} SystemLuaBridge;

typedef int (*LuaBridgeModuleLoadFunc)(lua_State *);
void lua_bridge_internal_register_lua_module(const char *name, LuaBridgeModuleLoadFunc load_func);
int lua_bridge_internal_lua_module_load(lua_State *state);
int lua_bridge_internal_lua_module_message(lua_State *state);

SystemLuaBridge *system_lua_bridge_internal = NULL;
bool system_lua_bridge_start(void) {
  if (system_lua_bridge_internal) {
    return false;
  }

  system_lua_bridge_internal = calloc(1, sizeof(SystemLuaBridge));

  lua_State *state = luaL_newstate();
  luaL_openlibs(state);
  lua_getglobal(state, "package");
  lua_getfield(state, -1, "path");
  lua_pop(state, 1);
  lua_pushstring(state, "./lua/?.lua");
  lua_setfield(state, -2, "path");
  lua_pop(state, 1);

  *system_lua_bridge_internal = (SystemLuaBridge){
    .state = state,
    .listeners = rectify_array_alloc(4, sizeof(int32_t)),
  };

  lua_bridge_internal_register_lua_module("lua_bridge", &lua_bridge_internal_lua_module_load);

  {
    luaL_loadfile(state, "./lua/main.lua");
    {
      int result = lua_pcall(state, 0, LUA_MULTRET, 0);
      if (result != LUA_OK) {
        const char *message = lua_tostring(state, -1);
        printf("LUA: %s: %s\n", __func__, message);
        lua_pop(state, 1);
      }
    }
  }

  return true;
}

void system_lua_bridge_stop(void) {
  if (!system_lua_bridge_internal) {
    return;
  }

  lua_close(system_lua_bridge_internal->state);
  rectify_array_free(&system_lua_bridge_internal->listeners);

  free(system_lua_bridge_internal);
  system_lua_bridge_internal = NULL;
}

// TODO: Queue up for next iteration instead of instant
void system_lua_bridge_message(uint32_t id, RectifyMap *const map) {
  if (!system_lua_bridge_internal) {
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(system_lua_bridge_internal->listeners); t++) {
    int32_t func_ref = system_lua_bridge_internal->listeners[t];
    lua_rawgeti(system_lua_bridge_internal->state, LUA_REGISTRYINDEX, func_ref);

    lua_pushinteger(system_lua_bridge_internal->state, (lua_Number)id);
    lua_newtable(system_lua_bridge_internal->state);

    rectify_map_print(map);

    RectifyMapIter iter = rectify_map_iter(map);
    for (RectifyMapItem item; rectify_map_iter_next(&iter, &item);) {
      switch (item.type) {
        case RECTIFY_MAP_TYPE_BYTE: {
          uint8_t val = *(uint8_t *)item.val;
          lua_pushinteger(system_lua_bridge_internal->state, val);
          break;
        }

        case RECTIFY_MAP_TYPE_BOOL: {
          bool val = *(bool *)item.val;
          lua_pushboolean(system_lua_bridge_internal->state, val);
          break;
        }

        case RECTIFY_MAP_TYPE_UINT: {
          uint32_t val = *(uint32_t *)item.val;
          lua_pushinteger(system_lua_bridge_internal->state, val);
          break;
        }

        case RECTIFY_MAP_TYPE_INT: {
          int32_t val = *(int32_t *)item.val;
          lua_pushinteger(system_lua_bridge_internal->state, val);
          break;
        }

        case RECTIFY_MAP_TYPE_FLOAT: {
          float val = *(float *)item.val;
          lua_pushnumber(system_lua_bridge_internal->state, val);
          break;
        }

        case RECTIFY_MAP_TYPE_DOUBLE: {
          double val = *(double *)item.val;
          lua_pushnumber(system_lua_bridge_internal->state, val);
          break;
        }

        case RECTIFY_MAP_TYPE_STRING: {
          char *val = (char *)item.val;
          lua_pushstring(system_lua_bridge_internal->state, val);
          break;
        }

        case RECTIFY_MAP_TYPE_PTR:
        default: {
          uintptr_t ptr = (uintptr_t)item.val;
          lua_pushinteger(system_lua_bridge_internal->state, ptr);
          break;
        }
      }
      lua_setfield(system_lua_bridge_internal->state, -2, item.key);
    }

    int result = lua_pcall(system_lua_bridge_internal->state, 2, 0, 0);
    if (result != LUA_OK) {
      const char *message = lua_tostring(system_lua_bridge_internal->state, -1);
      printf("LuaBridge: %s: %s\n", __func__, message);
      lua_pop(system_lua_bridge_internal->state, 1);
    }
  }
}

void lua_bridge_internal_register_lua_module(const char *name, LuaBridgeModuleLoadFunc load_func) {
  lua_getglobal(system_lua_bridge_internal->state, "package");
  lua_pushstring(system_lua_bridge_internal->state, "preload");
  lua_gettable(system_lua_bridge_internal->state, -2);
  lua_pushlightuserdata(system_lua_bridge_internal->state, system_lua_bridge_internal);
  lua_pushcclosure(system_lua_bridge_internal->state, load_func, 1);
  lua_setfield(system_lua_bridge_internal->state, -2, name);
  lua_settop(system_lua_bridge_internal->state, 0);
}

int lua_bridge_internal_lua_module_load(lua_State *state) {
  lua_newtable(state);

  lua_pushcclosure(state, &lua_bridge_internal_lua_module_message, 0);
  lua_setfield(state, -2, "message");

  return 1;
}

int lua_bridge_internal_lua_module_message(lua_State *state) {
  if (lua_gettop(state) < 1) {
    printf("LuaBridge: Too few arguments to function \"lua_bridge.message\".\n");
    return 0;
  }

  if (lua_isfunction(state, 1) != 1) {
    printf("LuaBridge: Incorrect argument type, expected function, \"lua_bridge.message\".\n");
    return 0;
  }

  int32_t func_ref = (int32_t)luaL_ref(state, LUA_REGISTRYINDEX);
  system_lua_bridge_internal->listeners = rectify_array_push(system_lua_bridge_internal->listeners, &func_ref);

  return 1;
}
