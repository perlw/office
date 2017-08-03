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
void system_lua_bridge_internal_register_lua_module(const char *name, LuaBridgeModuleLoadFunc load_func);
int system_lua_bridge_internal_lua_module_load(lua_State *state);
int system_lua_bridge_internal_lua_module_on_message(lua_State *state);
int system_lua_bridge_internal_lua_module_post_message(lua_State *state);
int system_lua_bridge_internal_lua_module_emit_message(lua_State *state);
RectifyMap *system_lua_bridge_internal_table_to_map(lua_State *state, int32_t index);

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

  for (uint32_t t = 0; MSG_NAMES[t]; t++) {
    lua_pushinteger(state, t);
    lua_setglobal(state, MSG_NAMES[t]);
  }

  system_lua_bridge_internal_register_lua_module("lua_bridge", &system_lua_bridge_internal_lua_module_load);

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

void system_lua_bridge_message(uint32_t id, RectifyMap *const map) {
  if (!system_lua_bridge_internal) {
    return;
  }

#ifdef LUA_BRIDGE_DEBUG
  printf("LuaBridge: Pushing to lua, %s%s\n", MSG_NAMES[id], (!map ? ", no data" : ""));
  rectify_map_print(map);
#endif // LUA_BRIDGE_DEBUG

  for (uint32_t t = 0; t < rectify_array_size(system_lua_bridge_internal->listeners); t++) {
    int32_t func_ref = system_lua_bridge_internal->listeners[t];
    lua_rawgeti(system_lua_bridge_internal->state, LUA_REGISTRYINDEX, func_ref);

    lua_pushinteger(system_lua_bridge_internal->state, (lua_Integer)id);
    lua_newtable(system_lua_bridge_internal->state);

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

void system_lua_bridge_internal_register_lua_module(const char *name, LuaBridgeModuleLoadFunc load_func) {
  lua_getglobal(system_lua_bridge_internal->state, "package");
  lua_pushstring(system_lua_bridge_internal->state, "preload");
  lua_gettable(system_lua_bridge_internal->state, -2);
  lua_pushlightuserdata(system_lua_bridge_internal->state, system_lua_bridge_internal);
  lua_pushcclosure(system_lua_bridge_internal->state, load_func, 1);
  lua_setfield(system_lua_bridge_internal->state, -2, name);
  lua_settop(system_lua_bridge_internal->state, 0);
}

int system_lua_bridge_internal_lua_module_load(lua_State *state) {
  lua_newtable(state);

  lua_pushcclosure(state, &system_lua_bridge_internal_lua_module_on_message, 0);
  lua_setfield(state, -2, "on_message");
  lua_pushcclosure(state, &system_lua_bridge_internal_lua_module_post_message, 0);
  lua_setfield(state, -2, "post_message");
  lua_pushcclosure(state, &system_lua_bridge_internal_lua_module_emit_message, 0);
  lua_setfield(state, -2, "emit_message");

  return 1;
}

int system_lua_bridge_internal_lua_module_on_message(lua_State *state) {
  if (lua_gettop(state) < 1) {
    printf("LuaBridge: Too few arguments to function \"lua_bridge.on_message\".\n");
    return 0;
  }

  if (lua_isfunction(state, 1) != 1) {
    printf("LuaBridge: Incorrect argument type, expected function, \"lua_bridge.on_message\".\n");
    return 0;
  }

  int32_t func_ref = (int32_t)luaL_ref(state, LUA_REGISTRYINDEX);
  system_lua_bridge_internal->listeners = rectify_array_push(system_lua_bridge_internal->listeners, &func_ref);

  return 0;
}

int system_lua_bridge_internal_lua_module_post_message(lua_State *state) {
  if (lua_gettop(state) < 2) {
    printf("LuaBridge: Too few arguments to function \"lua_bridge.post_message\".\n");
    return 0;
  }

  if (lua_isstring(state, 1) != 1) {
    printf("LuaBridge: Incorrect type for argument 1, expected string, \"lua_bridge.post_message\".\n");
    return 0;
  }
  if (lua_isinteger(state, 2) != 1) {
    printf("LuaBridge: Incorrect type for argument 2, expected integer, \"lua_bridge.post_message\".\n");
    return 0;
  }
  if (lua_istable(state, 3) != 1 && lua_isnoneornil(state, 3) != 1) {
    printf("LuaBridge: Incorrect type for argument 3, expected table, \"lua_bridge.post_message\".\n");
    return 0;
  }

  const char *system = lua_tostring(state, 1);
  uint32_t id = lua_tointeger(state, 2);
  RectifyMap *map = NULL;

  if (lua_isnoneornil(state, 3) != 1) {
    map = system_lua_bridge_internal_table_to_map(state, 3);
  }

  gossip_post(system, id, map);

  return 0;
}

int system_lua_bridge_internal_lua_module_emit_message(lua_State *state) {
  if (lua_gettop(state) < 1) {
    printf("LuaBridge: Too few arguments to function \"lua_bridge.emit_message\".\n");
    return 0;
  }

  if (lua_isinteger(state, 1) != 1) {
    printf("LuaBridge: Incorrect type for argument 1, expected integer, \"lua_bridge.emit_message\".\n");
    return 0;
  }
  if (lua_istable(state, 2) != 1 && lua_isnoneornil(state, 2) != 1) {
    printf("LuaBridge: Incorrect type for argument 2, expected table, \"lua_bridge.emit_message\".\n");
    return 0;
  }

  uint32_t id = lua_tointeger(state, 1);
  RectifyMap *map = NULL;

  if (lua_isnoneornil(state, 2) != 1) {
    map = system_lua_bridge_internal_table_to_map(state, 2);
  }

  gossip_emit(id, map);

  return 0;
}

RectifyMap *system_lua_bridge_internal_table_to_map(lua_State *state, int32_t index) {
  RectifyMap *map = rectify_map_create();

  lua_pushvalue(state, index);
  lua_pushnil(state);
  while (lua_next(state, -2)) {
    lua_pushvalue(state, -2);

    const char *key = lua_tostring(state, -1);
    if (lua_isboolean(state, -2) == 1) {
      rectify_map_set_bool(map, key, (bool)lua_toboolean(state, -2));
    } else if (lua_isinteger(state, -2) == 1) {
      rectify_map_set_int(map, key, (int32_t)lua_tointeger(state, -2));
    } else if (lua_isnumber(state, -2) == 1) {
      rectify_map_set_double(map, key, (double)lua_tonumber(state, -2));
    } else if (lua_isstring(state, -2) == 1) {
      size_t len = 0;
      const char *val = lua_tolstring(state, -2, &len);
      if (len == 1) {
        rectify_map_set_byte(map, key, (uint8_t)val[0]);
      } else {
        rectify_map_set_string(map, key, (char *const)val);
      }
    } else if (lua_istable(state, -2) == 1) {
      RectifyMap *submap = system_lua_bridge_internal_table_to_map(state, -2);
      rectify_map_set_map(map, key, submap);
    }

    lua_pop(state, 2);
  }
  lua_pop(state, 1);

  return map;
}
