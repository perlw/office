#include <assert.h>
#include <stdio.h>

//#include "lauxlib.h"
#include "lua.h"
//#include "lualib.h"

#include "bedrock/bedrock.h"

#include "messages.h"

typedef struct {
  lua_State *state;
  GossipHandle gossip_handle;
} LuaBridge;

void lua_bridge_internal_gossip(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata);

LuaBridge *lua_bridge_create(lua_State *state) {
  assert(state);

  LuaBridge *lua_bridge = calloc(1, sizeof(LuaBridge));

  *lua_bridge = (LuaBridge){
    .state = state,
  };

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
