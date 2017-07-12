#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
//#include "lualib.h"

#include "bedrock/bedrock.h"

#include "messages.h"

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
  lua_State *state;
  GossipHandle gossip_handle;
} LuaBridge;

void lua_bridge_internal_gossip(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata);
int lua_bridge_internal_load(lua_State *state);
int lua_bridge_internal_gossip_subscribe(lua_State *state);
int lua_bridge_internal_gossip_emit(lua_State *state);

LuaBridge *lua_bridge_create(lua_State *state) {
  assert(state);

  LuaBridge *lua_bridge = calloc(1, sizeof(LuaBridge));

  *lua_bridge = (LuaBridge){
    .state = state,
  };

  lua_getglobal(state, "package");
  lua_pushstring(state, "preload");
  lua_gettable(state, -2);
  lua_pushcclosure(state, lua_bridge_internal_load, 0);
  lua_setfield(state, -2, "gossip");
  lua_settop(state, 0);

  lua_bridge->gossip_handle = gossip_subscribe(GOSSIP_GROUP_ALL, GOSSIP_ID_ALL, &lua_bridge_internal_gossip, lua_bridge, NULL);

  return lua_bridge;
}

void lua_bridge_destroy(LuaBridge *const lua_bridge) {
  assert(lua_bridge);

  gossip_unsubscribe(lua_bridge->gossip_handle);

  free(lua_bridge);
}

void lua_bridge_internal_gossip(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata) {
  LuaBridge *lua_bridge = (LuaBridge *)subscriberdata;

  //printf("group %d, id %d\n", group_id, id);
}
/*void internal_lua_gossip_call(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata) {
  GossipLuaPackage *pkg = (GossipLuaPackage *)subscriberdata;

  lua_rawgeti(pkg->state, LUA_REGISTRYINDEX, pkg->func_ref);

  uint32_t num_args = 0;
  switch (group_id) {
    case MSG_SCENE:
      switch (id) {
        case MSG_SCENE_CHANGED: {
          Scene *scene = (Scene *)userdata;
          lua_pushstring(pkg->state, scene->name);
          num_args++;
        }
      }
  }

  int result = lua_pcall(pkg->state, num_args, 0, 0);
  if (result != LUA_OK) {
    const char *message = lua_tostring(pkg->state, -1);
    printf("LUA: %s: %s\n", __func__, message);
    lua_pop(pkg->state, 1);
  }
}*/

int lua_bridge_internal_load(lua_State *state) {
  lua_newtable(state);

  lua_pushcfunction(state, &lua_bridge_internal_gossip_subscribe);
  lua_setfield(state, -2, "subscribe");
  lua_pushcfunction(state, &lua_bridge_internal_gossip_emit);
  lua_setfield(state, -2, "emit");

  return 1;
}

typedef struct {
  int32_t group_id;
  int32_t id;
} GossipKeys;

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

  const char *msg_name = lua_tolstring(state, 1, NULL);
  int32_t func_ref = (int32_t)luaL_ref(state, LUA_REGISTRYINDEX);

  GossipKeys keys = lua_bridge_internal_find_gossip_keys(msg_name);
  if (keys.group_id < 0 || keys.id < 0) {
    return 0;
  }

  printf("%s -> %d:%d\n", msg_name, keys.group_id, keys.id);

  /*GossipLuaPackage *pkg = calloc(1, sizeof(GossipLuaPackage));
  *pkg = (GossipLuaPackage){
    .state = state,
    .func_ref = func_ref,
  };
  gossip_subscribe(group, id, &internal_lua_gossip_call, pkg, NULL);*/

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
