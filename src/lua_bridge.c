#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include "lua_bridge.h"

#include "input.h"
#include "messages.h"
#include "scenes.h"
#include "ui/ui.h"

struct {
  char *name;
  uint32_t key;
  struct {
    char *name;
    uint32_t key;
  } children[10];
} lua_bridge_keys[] = {
  {
    .name = "game",
    .key = MSG_GAME,
    .children = {
      {
        .name = "init",
        .key = MSG_GAME_INIT,
      },
      {
        .name = "kill",
        .key = MSG_GAME_KILL,
      },
      {
        .name = NULL,
        .key = 0,
      },
    },
  },
  {
    .name = "scene",
    .key = MSG_SCENE,
    .children = {
      {
        .name = "prev",
        .key = MSG_SCENE_PREV,
      },
      {
        .name = "next",
        .key = MSG_SCENE_NEXT,
      },
      {
        .name = "setup",
        .key = MSG_SCENE_SETUP,
      },
      {
        .name = "teardown",
        .key = MSG_SCENE_TEARDOWN,
      },
      {
        .name = "changed",
        .key = MSG_SCENE_CHANGED,
      },
      {
        .name = NULL,
        .key = 0,
      },
    },
  },
  {
    .name = NULL,
    .key = 0,
  },
};

typedef struct {
  int32_t group_id;
  int32_t id;
} GossipKeys;

typedef struct {
  uint32_t func_ref;
  GossipKeys keys;
} LuaBridgeHandle;

struct LuaBridge {
  lua_State *state;
  GossipHandle action_handle;
  GossipHandle gossip_handle;
  LuaBridgeHandle *handles;
};

void lua_bridge_internal_action_event(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata);
void lua_bridge_internal_gossip_event(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata);
int lua_bridge_internal_load(lua_State *state);
int lua_bridge_internal_gossip_subscribe(lua_State *state);
int lua_bridge_internal_gossip_emit(lua_State *state);

int internal_action(lua_State *state) {
  InputActionRef action_ref = (InputActionRef){
    .action = (char *)lua_tolstring(state, 1, NULL),
    .ref = (int32_t)luaL_ref(state, LUA_REGISTRYINDEX),
  };
  input_action_add_action(&action_ref);
  return 0;
}

int internal_mod_func1(lua_State *state) {
  printf("LUA FUNC 1\n");
  return 0;
}

int internal_mod_func2(lua_State *state) {
  printf("LUA FUNC 2\n");
  return 0;
}

int internal_lua_ui_create_window(lua_State *state) {
  if (lua_gettop(state) < 4) {
    printf("Main: Too few arguments to function \"ui.create_window\".\n");
    return 0;
  }

  uint32_t x = (uint32_t)lua_tonumber(state, 1);
  uint32_t y = (uint32_t)lua_tonumber(state, 2);
  uint32_t width = (uint32_t)lua_tonumber(state, 3);
  uint32_t height = (uint32_t)lua_tonumber(state, 4);

  UIWindow *window = ui_window_create(x, y, width, height);
  lua_pushnumber(state, (lua_Number)(uintptr_t)window);

  return 1;
}

int internal_lua_ui_destroy_window(lua_State *state) {
  if (lua_gettop(state) < 1) {
    printf("Main: Too few arguments to function \"ui.destroy_window\".\n");
    return 0;
  }

  UIWindow *window = (UIWindow *)(uintptr_t)lua_tonumber(state, 1);
  ui_window_destroy(window);

  return 0;
}

void register_lua_module(lua_State *state, const char *name, int (*load_func)(lua_State *)) {
  lua_getglobal(state, "package");
  lua_pushstring(state, "preload");
  lua_gettable(state, -2);
  lua_pushcclosure(state, load_func, 0);
  lua_setfield(state, -2, name);
  lua_settop(state, 0);
}

int internal_lua_testlib(lua_State *state) {
  lua_newtable(state);

  lua_pushcfunction(state, &internal_mod_func1);
  lua_setfield(state, -2, "func1");
  lua_pushcfunction(state, &internal_mod_func2);
  lua_setfield(state, -2, "func2");

  return 1;
}

int internal_lua_ui(lua_State *state) {
  lua_newtable(state);

  lua_pushcfunction(state, &internal_lua_ui_create_window);
  lua_setfield(state, -2, "window_create");
  lua_pushcfunction(state, &internal_lua_ui_destroy_window);
  lua_setfield(state, -2, "window_destroy");

  return 1;
}

LuaBridge *lua_bridge_create(void) {
  LuaBridge *lua_bridge = calloc(1, sizeof(LuaBridge));

  lua_State *state = luaL_newstate();
  luaL_openlibs(state);
  lua_getglobal(state, "package");
  lua_getfield(state, -1, "path");
  lua_pop(state, 1);
  lua_pushstring(state, "./lua/?.lua");
  lua_setfield(state, -2, "path");
  lua_pop(state, 1);

  *lua_bridge = (LuaBridge){
    .state = state,
    .handles = rectify_array_alloc(10, sizeof(LuaBridgeHandle)),
  };

  lua_getglobal(state, "package");
  lua_pushstring(state, "preload");
  lua_gettable(state, -2);
  lua_pushlightuserdata(state, lua_bridge);
  lua_pushcclosure(state, lua_bridge_internal_load, 1);
  lua_setfield(state, -2, "gossip");
  lua_settop(state, 0);

  lua_bridge->action_handle = gossip_subscribe(MSG_LUA_BRIDGE, LUA_ACTION, &lua_bridge_internal_action_event, lua_bridge, NULL);
  lua_bridge->gossip_handle = gossip_subscribe(GOSSIP_GROUP_ALL, GOSSIP_ID_ALL, &lua_bridge_internal_gossip_event, lua_bridge, NULL);

  {
    lua_pushcclosure(state, &internal_action, 0);
    lua_setglobal(state, "action");

    register_lua_module(state, "testlib", internal_lua_testlib);
    register_lua_module(state, "ui", internal_lua_ui);

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

  return lua_bridge;
}

void lua_bridge_destroy(LuaBridge *const lua_bridge) {
  assert(lua_bridge);

  gossip_unsubscribe(lua_bridge->gossip_handle);
  gossip_unsubscribe(lua_bridge->action_handle);

  rectify_array_free(lua_bridge->handles);
  lua_close(lua_bridge->state);

  free(lua_bridge);
}

void lua_bridge_internal_action_event(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata) {
  LuaBridge *lua_bridge = (LuaBridge *)subscriberdata;
  InputActionRef *action_ref = (InputActionRef *)userdata;

  lua_rawgeti(lua_bridge->state, LUA_REGISTRYINDEX, action_ref->ref);
  int result = lua_pcall(lua_bridge->state, 0, 0, 0);
  if (result != LUA_OK) {
    const char *message = lua_tostring(lua_bridge->state, -1);
    printf("LUA: %s: %s\n", __func__, message);
    lua_pop(lua_bridge->state, 1);
  }
}

void lua_bridge_internal_gossip_event(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata) {
  LuaBridge *lua_bridge = (LuaBridge *)subscriberdata;

  for (uint32_t t = 0; t < rectify_array_size(lua_bridge->handles); t++) {
    LuaBridgeHandle *handle = &lua_bridge->handles[t];

    if (handle->keys.group_id == group_id && handle->keys.id == id) {
      lua_rawgeti(lua_bridge->state, LUA_REGISTRYINDEX, handle->func_ref);

      uint32_t num_args = 0;
      switch (group_id) {
        case MSG_SCENE:
          switch (id) {
            case MSG_SCENE_SETUP:
            case MSG_SCENE_TEARDOWN:
            case MSG_SCENE_CHANGED: {
              Scene *scene = (Scene *)userdata;
              lua_pushstring(lua_bridge->state, scene->name);
              num_args++;
            }
          }
      }

      int result = lua_pcall(lua_bridge->state, num_args, 0, 0);
      if (result != LUA_OK) {
        const char *message = lua_tostring(lua_bridge->state, -1);
        printf("LUA: %s: %s\n", __func__, message);
        lua_pop(lua_bridge->state, 1);
      }
    }
  }
}

int lua_bridge_internal_load(lua_State *state) {
  LuaBridge *lua_bridge = (LuaBridge *)lua_topointer(state, lua_upvalueindex(1));

  lua_newtable(state);

  lua_pushlightuserdata(state, lua_bridge);
  lua_pushcclosure(state, &lua_bridge_internal_gossip_subscribe, 1);
  lua_setfield(state, -2, "subscribe");
  lua_pushlightuserdata(state, lua_bridge);
  lua_pushcclosure(state, &lua_bridge_internal_gossip_emit, 1);
  lua_setfield(state, -2, "emit");

  return 1;
}

GossipKeys lua_bridge_internal_find_gossip_keys(const char *lua_keys) {
  int32_t group_id = -1;
  int32_t id = -1;

  char *tokens = rectify_memory_alloc_copy(lua_keys, strlen(lua_keys) + 1);
  char *token_group = strtok(tokens, ":");
  char *token_id = strtok(NULL, ":");
  for (uint32_t t = 0; lua_bridge_keys[t].name; t++) {
    if (strcmp(token_group, lua_bridge_keys[t].name) == 0) {
      group_id = lua_bridge_keys[t].key;
      for (uint32_t u = 0; lua_bridge_keys[t].children[u].name; u++) {
        if (strcmp(token_id, lua_bridge_keys[t].children[u].name) == 0) {
          id = lua_bridge_keys[t].children[u].key;
          break;
        }
      }
      break;
    }
  }
  free(tokens);

  return (GossipKeys){
    .group_id = group_id,
    .id = id,
  };
}

int lua_bridge_internal_gossip_subscribe(lua_State *state) {
  if (lua_gettop(state) < 2) {
    printf("Main: Too few arguments to function \"gossip.subscribe\".\n");
    return 0;
  }

  if (!lua_isstring(state, 1)) {
    printf("Main: Incorrect argumenttypes, expected string, function.\n");
    return 0;
  }

  LuaBridge *lua_bridge = (LuaBridge *)lua_topointer(state, lua_upvalueindex(1));
  const char *msg_name = lua_tolstring(state, 1, NULL);
  int32_t func_ref = (int32_t)luaL_ref(state, LUA_REGISTRYINDEX);

  GossipKeys keys = lua_bridge_internal_find_gossip_keys(msg_name);
  if (keys.group_id < 0 || keys.id < 0) {
    return 0;
  }

  lua_bridge->handles = rectify_array_push(lua_bridge->handles, &(LuaBridgeHandle){
                                                                  .func_ref = func_ref, .keys = keys,
                                                                });
  return 0;
}

int lua_bridge_internal_gossip_emit(lua_State *state) {
  if (lua_gettop(state) < 1) {
    printf("Main: Too few arguments to function \"gossip.emit\".\n");
    return 0;
  }

  if (!lua_isstring(state, 1)) {
    printf("Main: Incorrect argumenttypes, expected string, function.\n");
    return 0;
  }

  const char *msg_name = lua_tolstring(state, 1, NULL);

  GossipKeys keys = lua_bridge_internal_find_gossip_keys(msg_name);
  if (keys.group_id < 0 || keys.id < 0) {
    return 0;
  }

  gossip_emit(keys.group_id, keys.id, NULL, NULL);

  return 0;
}
