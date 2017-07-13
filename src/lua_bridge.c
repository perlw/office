#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
//#include "lualib.h"

#include "bedrock/bedrock.h"

#include "messages.h"
#include "scenes.h"

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

typedef struct {
  lua_State *state;
  GossipHandle gossip_handle;
  LuaBridgeHandle *handles;
} LuaBridge;

void lua_bridge_internal_load_event(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata);
void lua_bridge_internal_gossip_event(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata);
int lua_bridge_internal_load(lua_State *state);
int lua_bridge_internal_gossip_subscribe(lua_State *state);
int lua_bridge_internal_gossip_emit(lua_State *state);

LuaBridge *lua_bridge_create(lua_State *state) {
  assert(state);

  LuaBridge *lua_bridge = calloc(1, sizeof(LuaBridge));

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

  lua_bridge->gossip_handle = gossip_subscribe(GOSSIP_GROUP_ALL, GOSSIP_ID_ALL, &lua_bridge_internal_gossip_event, lua_bridge, NULL);

  return lua_bridge;
}

void lua_bridge_destroy(LuaBridge *const lua_bridge) {
  assert(lua_bridge);

  gossip_unsubscribe(lua_bridge->gossip_handle);

  rectify_array_free(lua_bridge->handles);

  free(lua_bridge);
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
