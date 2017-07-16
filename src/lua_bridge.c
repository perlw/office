#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include "lua_bridge.h"

#include "input.h"
#include "scenes.h"
#include "ui/ui.h"

typedef struct {
  uint32_t func_ref;
  //GossipKeys keys;
} LuaBridgeHandle;

struct LuaBridge {
  lua_State *state;
  GossipHandle action_handle;
  GossipHandle gossip_handle;
  LuaBridgeHandle *handles;
};

int lua_bridge_internal_input_action(lua_State *state);
void lua_bridge_internal_action_event(const char *message, void *const subscriberdata, void *const userdata);
void lua_bridge_internal_gossip_event(const char *message, void *const subscriberdata, void *const userdata);

int lua_bridge_internal_gossip_load(lua_State *state);
int lua_bridge_internal_gossip_subscribe(lua_State *state);
int lua_bridge_internal_gossip_unsubscribe(lua_State *state);
int lua_bridge_internal_gossip_emit(lua_State *state);
void lua_bridge_internal_gossip_ids_to_string(uint32_t group_id, uint32_t id, uintmax_t length, char *buffer);

int lua_bridge_internal_ui_window_load(lua_State *state);
int lua_bridge_internal_ui_window_create(lua_State *state);
int lua_bridge_internal_ui_window_destroy(lua_State *state);
int lua_bridge_internal_ui_window_glyph(lua_State *state);

void lua_bridge_internal_register_lua_module(LuaBridge *lua_bridge, const char *name, int (*load_func)(lua_State *)) {
  lua_getglobal(lua_bridge->state, "package");
  lua_pushstring(lua_bridge->state, "preload");
  lua_gettable(lua_bridge->state, -2);
  lua_pushlightuserdata(lua_bridge->state, lua_bridge);
  lua_pushcclosure(lua_bridge->state, load_func, 1);
  lua_setfield(lua_bridge->state, -2, name);
  lua_settop(lua_bridge->state, 0);
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

  lua_bridge_internal_register_lua_module(lua_bridge, "lua_bridge/gossip", &lua_bridge_internal_gossip_load);
  lua_bridge_internal_register_lua_module(lua_bridge, "lua_bridge/ui", &lua_bridge_internal_ui_window_load);

  lua_bridge->action_handle = gossip_subscribe("lua_bridge:action", &lua_bridge_internal_action_event, lua_bridge);
  lua_bridge->gossip_handle = gossip_subscribe("*:*", &lua_bridge_internal_gossip_event, lua_bridge);

  {
    lua_pushcclosure(state, &lua_bridge_internal_input_action, 0);
    lua_setglobal(state, "action");

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

  lua_close(lua_bridge->state);

  gossip_unsubscribe(lua_bridge->gossip_handle);
  gossip_unsubscribe(lua_bridge->action_handle);

  rectify_array_free(lua_bridge->handles);

  free(lua_bridge);
}

void lua_bridge_internal_action_event(const char *message, void *const subscriberdata, void *const userdata) {
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

void lua_bridge_internal_gossip_event(const char *message, void *const subscriberdata, void *const userdata) {
  LuaBridge *lua_bridge = (LuaBridge *)subscriberdata;

  /*for (uint32_t t = 0; t < rectify_array_size(lua_bridge->handles); t++) {
    LuaBridgeHandle *handle = &lua_bridge->handles[t];

    if ((handle->keys.group_id == group_id || handle->keys.group_id == GOSSIP_GROUP_ALL)
        && (handle->keys.id == id || handle->keys.id == GOSSIP_ID_ALL)) {
      lua_rawgeti(lua_bridge->state, LUA_REGISTRYINDEX, handle->func_ref);

      uint32_t num_args = 0;

      char buffer[128] = { 0 };
      lua_bridge_internal_gossip_ids_to_string(group_id, id, 128, buffer);
      lua_pushstring(lua_bridge->state, buffer);
      num_args++;

      switch (group_id) {
        case MSG_SCENE:
          switch (id) {
            case MSG_SCENE_SETUP:
            case MSG_SCENE_TEARDOWN:
            case MSG_SCENE_CHANGED: {
              Scene *scene = (Scene *)userdata;
              lua_pushstring(lua_bridge->state, scene->name);
              num_args++;
              break;
            }
          }
          break;

        case MSG_UI_WINDOW:
          switch (id) {
            case UI_WINDOW_EVENT_MOUSEMOVE: {
              UIEventClick *event = (UIEventClick *)userdata;

              lua_newtable(lua_bridge->state);

              lua_pushnumber(lua_bridge->state, (lua_Number)(uintptr_t)event->target);
              lua_setfield(lua_bridge->state, -2, "target");
              lua_pushnumber(lua_bridge->state, event->x);
              lua_setfield(lua_bridge->state, -2, "x");
              lua_pushnumber(lua_bridge->state, event->y);
              lua_setfield(lua_bridge->state, -2, "y");

              num_args++;
              break;
            }

            case UI_WINDOW_EVENT_CLICK: {
              UIEventClick *event = (UIEventClick *)userdata;

              lua_newtable(lua_bridge->state);

              lua_pushnumber(lua_bridge->state, (lua_Number)(uintptr_t)event->target);
              lua_setfield(lua_bridge->state, -2, "target");
              lua_pushnumber(lua_bridge->state, event->x);
              lua_setfield(lua_bridge->state, -2, "x");
              lua_pushnumber(lua_bridge->state, event->y);
              lua_setfield(lua_bridge->state, -2, "y");

              num_args++;
              break;
            }
          }
          break;

        case MSG_UI_WIDGET:
          switch (id) {
            case UI_WIDGET_RUNE_SELECTOR_SELECTED: {
              uint32_t rune = *(uint32_t *)userdata;
              lua_pushnumber(lua_bridge->state, rune);
              num_args++;
              break;
            }

            case UI_WIDGET_COLOR_SELECTOR_SELECTED: {
              uint32_t color = *(uint32_t *)userdata;
              lua_pushnumber(lua_bridge->state, color);
              num_args++;
              break;
            }

            case UI_WIDGET_EVENT_PAINT: {
              UIWindow *window = (UIWindow *)userdata;
              lua_newtable(lua_bridge->state);

              lua_pushnumber(lua_bridge->state, (lua_Number)(uintptr_t)window);
              lua_setfield(lua_bridge->state, -2, "target");

              num_args++;
              break;
            }
          }
          break;
      }

      int result = lua_pcall(lua_bridge->state, num_args, 0, 0);
      if (result != LUA_OK) {
        const char *message = lua_tostring(lua_bridge->state, -1);
        printf("LUA: %s: %s\n", __func__, message);
        lua_pop(lua_bridge->state, 1);
      }
    }
  }*/
}

int lua_bridge_internal_input_action(lua_State *state) {
  InputActionRef action_ref = (InputActionRef){
    .action = (char *)lua_tostring(state, 1),
    .ref = (int32_t)luaL_ref(state, LUA_REGISTRYINDEX),
  };
  input_action_add_action(&action_ref);
  return 0;
}

int lua_bridge_internal_ui_window_load(lua_State *state) {
  lua_newtable(state);

  lua_pushcfunction(state, &lua_bridge_internal_ui_window_create);
  lua_setfield(state, -2, "window_create");
  lua_pushcfunction(state, &lua_bridge_internal_ui_window_destroy);
  lua_setfield(state, -2, "window_destroy");
  lua_pushcfunction(state, &lua_bridge_internal_ui_window_glyph);
  lua_setfield(state, -2, "window_glyph");

  return 1;
}

int lua_bridge_internal_ui_window_create(lua_State *state) {
  if (lua_gettop(state) < 5) {
    printf("Main: Too few arguments to function \"ui.window_create\".\n");
    lua_pushnil(state);
    return 1;
  }

  const char *title = lua_tostring(state, 1);
  uint32_t x = (uint32_t)lua_tonumber(state, 2);
  uint32_t y = (uint32_t)lua_tonumber(state, 3);
  uint32_t width = (uint32_t)lua_tonumber(state, 4);
  uint32_t height = (uint32_t)lua_tonumber(state, 5);

  UIWindow *window = ui_window_create(title, x, y, width, height);
  lua_pushnumber(state, (lua_Number)(uintptr_t)window);

  return 1;
}

int lua_bridge_internal_ui_window_destroy(lua_State *state) {
  if (lua_gettop(state) < 1) {
    printf("Main: Too few arguments to function \"ui.window_destroy\".\n");
    return 0;
  }

  UIWindow *window = (UIWindow *)(uintptr_t)lua_tonumber(state, 1);
  if (!window) {
    return 0;
  }

  ui_window_destroy(window);

  return 0;
}

int lua_bridge_internal_ui_window_glyph(lua_State *state) {
  if (lua_gettop(state) < 5) {
    printf("Main: Too few arguments to function \"ui.window_glyph\".\n");
    return 0;
  }

  UIWindow *window = (UIWindow *)(uintptr_t)lua_tonumber(state, 1);
  if (!window) {
    return 0;
  }

  uint8_t rune = (uint8_t)lua_tonumber(state, 2);
  int32_t x = (int32_t)lua_tonumber(state, 3);
  int32_t y = (int32_t)lua_tonumber(state, 4);
  uint32_t fore_color = (uint32_t)lua_tonumber(state, 5);
  uint32_t back_color = (uint32_t)lua_tonumber(state, 6);

  ui_window_glyph(window, x, y, (Glyph){
                                  .rune = rune,
                                  .fore = glyphcolor_from_int(fore_color),
                                  .back = glyphcolor_from_int(back_color),
                                });

  return 0;
}

int lua_bridge_internal_gossip_load(lua_State *state) {
  LuaBridge *lua_bridge = (LuaBridge *)lua_topointer(state, lua_upvalueindex(1));

  lua_newtable(state);

  lua_pushlightuserdata(state, lua_bridge);
  lua_pushcclosure(state, &lua_bridge_internal_gossip_subscribe, 1);
  lua_setfield(state, -2, "subscribe");
  lua_pushlightuserdata(state, lua_bridge);
  lua_pushcclosure(state, &lua_bridge_internal_gossip_unsubscribe, 1);
  lua_setfield(state, -2, "unsubscribe");
  lua_pushlightuserdata(state, lua_bridge);
  lua_pushcclosure(state, &lua_bridge_internal_gossip_emit, 1);
  lua_setfield(state, -2, "emit");

  return 1;
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
  const char *msg_name = lua_tostring(state, 1);
  int32_t func_ref = (int32_t)luaL_ref(state, LUA_REGISTRYINDEX);

  /*GossipKeys keys = lua_bridge_internal_find_gossip_keys(msg_name);
  if (keys.group_id < 0 || keys.id < 0) {
    return 0;
  }*/

  lua_bridge->handles = rectify_array_push(lua_bridge->handles, &(LuaBridgeHandle){
                                                                  .func_ref = func_ref,
                                                                });
  lua_pushnumber(lua_bridge->state, func_ref);

  return 1;
}

int lua_bridge_internal_gossip_unsubscribe(lua_State *state) {
  if (lua_gettop(state) < 1) {
    printf("Main: Too few arguments to function \"gossip.unsubscribe\".\n");
    return 0;
  }

  if (!lua_isnumber(state, 1)) {
    printf("Main: Incorrect argumenttypes, expected number.\n");
    return 0;
  }

  LuaBridge *lua_bridge = (LuaBridge *)lua_topointer(state, lua_upvalueindex(1));
  int32_t func_ref = (int32_t)lua_tonumber(state, 1);

  for (uint32_t t = 0; t < rectify_array_size(lua_bridge->handles); t++) {
    LuaBridgeHandle *handle = &lua_bridge->handles[t];

    if (handle->func_ref == func_ref) {
      lua_bridge->handles = rectify_array_delete(lua_bridge->handles, t);
    }
  }

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

  const char *message = lua_tostring(state, 1);

  char *message_tokens = rectify_memory_alloc_copy(message, strlen(message) + 1);
  char *message_token_group = strtok(message_tokens, ":");
  char *message_token_id = strtok(NULL, ":");
  bool skip_group_check = (strncmp(message_token_group, "*", 2) == 0);
  bool skip_id_check = (strncmp(message_token_id, "*", 2) == 0);

  if (strncmp(message_token_group, "widget", 128) == 0) {
    if (strncmp(message_token_id, "rune_selected", 128) == 0) {
      uint32_t rune = (uint32_t)lua_tonumber(state, 2);
      gossip_emit(message, &rune);
    } else if (strncmp(message_token_id, "color_selected", 128) == 0) {
      uint32_t color = (uint32_t)lua_tonumber(state, 2);
      gossip_emit(message, &color);
    } else {
      gossip_emit(message, NULL);
    }
  } else {
    gossip_emit(message, NULL);
  }

  free(message_tokens);

  return 0;
}
