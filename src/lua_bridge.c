#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include "lua_bridge.h"

#include "ascii/ascii.h"
#include "input.h"
#include "scenes.h"
#include "screen.h"
#include "ui/ui.h"

typedef struct {
  uint32_t func_ref;
  char *group_id;
  char *id;
} LuaBridgeHandle;

struct LuaBridge {
  lua_State *state;
  GossipHandle action_handle;
  GossipHandle gossip_handle;

  UIWindow **windows;

  LuaBridgeHandle *handles;
};

int lua_bridge_internal_input_action(lua_State *state);
void lua_bridge_internal_action_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);
void lua_bridge_internal_gossip_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);
void lua_bridge_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

int lua_bridge_internal_gossip_load(lua_State *state);
int lua_bridge_internal_gossip_subscribe(lua_State *state);
int lua_bridge_internal_gossip_unsubscribe(lua_State *state);
int lua_bridge_internal_gossip_emit(lua_State *state);

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
    .windows = rectify_array_alloc(10, sizeof(UIWindow *)),
  };

  lua_bridge_internal_register_lua_module(lua_bridge, "lua_bridge/gossip", &lua_bridge_internal_gossip_load);
  lua_bridge_internal_register_lua_module(lua_bridge, "lua_bridge/ui", &lua_bridge_internal_ui_window_load);

  lua_bridge->action_handle = gossip_subscribe("lua_bridge:action", &lua_bridge_internal_action_event, lua_bridge);
  lua_bridge->gossip_handle = gossip_subscribe("*:*", &lua_bridge_internal_gossip_event, lua_bridge);
  screen_hook_render(&lua_bridge_internal_render_hook, lua_bridge);

  {
    luaL_loadfile(state, "./lua/init.lua");
    {
      int result = lua_pcall(state, 0, LUA_MULTRET, 0);
      if (result != LUA_OK) {
        const char *message = lua_tostring(state, -1);
        printf("LUA: %s: %s\n", __func__, message);
        lua_pop(state, 1);
      }
    }
  }

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

  screen_unhook_render(&lua_bridge_internal_render_hook, lua_bridge);

  for (uint32_t t = 0; t < rectify_array_size(lua_bridge->handles); t++) {
    LuaBridgeHandle *handle = &lua_bridge->handles[t];
    if (handle->group_id) {
      free(handle->group_id);
    }
    if (handle->id) {
      free(handle->id);
    }
  }
  rectify_array_free(lua_bridge->handles);

  rectify_array_free(lua_bridge->windows);

  free(lua_bridge);
}

void lua_bridge_update(LuaBridge *const lua_bridge, double delta) {
  assert(lua_bridge);

  for (uint32_t t = 0; t < rectify_array_size(lua_bridge->windows); t++) {
    ui_window_update(lua_bridge->windows[t], delta);
  }
}

void lua_bridge_internal_action_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
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

void lua_bridge_internal_gossip_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  LuaBridge *lua_bridge = (LuaBridge *)subscriberdata;

  bool skip_group_check = (strncmp(group_id, "*", 2) == 0);
  bool skip_id_check = (strncmp(id, "*", 2) == 0);

  for (uint32_t t = 0; t < rectify_array_size(lua_bridge->handles); t++) {
    LuaBridgeHandle *handle = &lua_bridge->handles[t];

    if ((skip_group_check || strncmp(handle->group_id, "*", 2) == 0 || strncmp(handle->group_id, group_id, 128) == 0)
        && (skip_id_check || strncmp(handle->id, "*", 2) == 0 || strncmp(handle->id, id, 128) == 0)) {
      lua_rawgeti(lua_bridge->state, LUA_REGISTRYINDEX, handle->func_ref);

      uint32_t num_args = 0;

      lua_pushstring(lua_bridge->state, group_id);
      lua_pushstring(lua_bridge->state, id);
      num_args += 2;

      if (strncmp(group_id, "scene", 128) == 0) {
        Scene *scene = (Scene *)userdata;
        lua_pushstring(lua_bridge->state, scene->name);
        num_args++;
      } else if (strncmp(group_id, "window", 128) == 0) {
        if (strncmp(id, "mousemove", 128) == 0) {
          UIEventClick *event = (UIEventClick *)userdata;

          lua_newtable(lua_bridge->state);

          lua_pushnumber(lua_bridge->state, (lua_Number)(uintptr_t)event->target);
          lua_setfield(lua_bridge->state, -2, "target");
          lua_pushnumber(lua_bridge->state, event->x);
          lua_setfield(lua_bridge->state, -2, "x");
          lua_pushnumber(lua_bridge->state, event->y);
          lua_setfield(lua_bridge->state, -2, "y");

          num_args++;
        } else if (strncmp(id, "click", 128) == 0) {
          UIEventClick *event = (UIEventClick *)userdata;

          lua_newtable(lua_bridge->state);

          lua_pushnumber(lua_bridge->state, (lua_Number)(uintptr_t)event->target);
          lua_setfield(lua_bridge->state, -2, "target");
          lua_pushnumber(lua_bridge->state, event->x);
          lua_setfield(lua_bridge->state, -2, "x");
          lua_pushnumber(lua_bridge->state, event->y);
          lua_setfield(lua_bridge->state, -2, "y");

          num_args++;
        }
      } else if (strncmp(group_id, "widget", 128) == 0) {
        if (strncmp(id, "rune_selected", 128) == 0) {
          uint32_t rune = *(uint32_t *)userdata;
          lua_pushnumber(lua_bridge->state, rune);
          num_args++;
        } else if (strncmp(id, "color_selected", 128) == 0) {
          uint32_t color = *(uint32_t *)userdata;
          lua_pushnumber(lua_bridge->state, color);
          num_args++;
        } else if (strncmp(id, "paint", 128) == 0) {
          uintptr_t window = *(uintptr_t *)userdata;
          lua_newtable(lua_bridge->state);

          lua_pushnumber(lua_bridge->state, (lua_Number)window);
          lua_setfield(lua_bridge->state, -2, "target");

          num_args++;
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

void lua_bridge_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  LuaBridge *lua_bridge = (LuaBridge *)userdata;

  for (uint32_t t = 0; t < rectify_array_size(lua_bridge->windows); t++) {
    ui_window_draw(lua_bridge->windows[t], screen);
  }
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
  LuaBridge *lua_bridge = (LuaBridge *)lua_topointer(state, lua_upvalueindex(1));

  lua_newtable(state);

  lua_pushlightuserdata(state, lua_bridge);
  lua_pushcclosure(state, &lua_bridge_internal_ui_window_create, 1);
  lua_setfield(state, -2, "window_create");
  lua_pushlightuserdata(state, lua_bridge);
  lua_pushcclosure(state, &lua_bridge_internal_ui_window_destroy, 1);
  lua_setfield(state, -2, "window_destroy");
  lua_pushlightuserdata(state, lua_bridge);
  lua_pushcclosure(state, &lua_bridge_internal_ui_window_glyph, 1);
  lua_setfield(state, -2, "window_glyph");

  return 1;
}

int lua_bridge_internal_ui_window_create(lua_State *state) {
  if (lua_gettop(state) < 5) {
    printf("Main: Too few arguments to function \"ui.window_create\".\n");
    lua_pushnil(state);
    return 1;
  }

  LuaBridge *lua_bridge = (LuaBridge *)lua_topointer(state, lua_upvalueindex(1));
  const char *title = lua_tostring(state, 1);
  uint32_t x = (uint32_t)lua_tonumber(state, 2);
  uint32_t y = (uint32_t)lua_tonumber(state, 3);
  uint32_t width = (uint32_t)lua_tonumber(state, 4);
  uint32_t height = (uint32_t)lua_tonumber(state, 5);

  UIWindow *window = ui_window_create(title, x, y, width, height);
  printf("create_window %p\n", (void *)window);
  lua_bridge->windows = rectify_array_push(lua_bridge->windows, (void *)&window);

  lua_pushnumber(state, (lua_Number)(uintptr_t)window);

  return 1;
}

int lua_bridge_internal_ui_window_destroy(lua_State *state) {
  if (lua_gettop(state) < 1) {
    printf("Main: Too few arguments to function \"ui.window_destroy\".\n");
    return 0;
  }

  LuaBridge *lua_bridge = (LuaBridge *)lua_topointer(state, lua_upvalueindex(1));
  UIWindow *window = (UIWindow *)(uintptr_t)lua_tonumber(state, 1);
  printf("destroy_window %p\n", (void *)window);
  if (!window) {
    return 0;
  }
  for (uint32_t t = 0; t < rectify_array_size(lua_bridge->windows); t++) {
    if (lua_bridge->windows[t] == window) {
      lua_bridge->windows = rectify_array_delete(lua_bridge->windows, t);
    }
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
  const char *message = lua_tostring(state, 1);
  int32_t func_ref = (int32_t)luaL_ref(state, LUA_REGISTRYINDEX);

  char *message_tokens = rectify_memory_alloc_copy(message, sizeof(char) * (strlen(message) + 1));
  char *message_token_group = strtok(message_tokens, ":");
  char *message_token_id = strtok(NULL, ":");

  printf("LUA: \"gossip.subscribe\" x <%s>:<%s> #%d\n", message_token_group, message_token_id, func_ref);
  lua_bridge->handles = rectify_array_push(lua_bridge->handles, &(LuaBridgeHandle){
                                                                  .func_ref = func_ref, .group_id = rectify_memory_alloc_copy(message_token_group, sizeof(char) * (strnlen(message_token_group, 128) + 1)), .id = rectify_memory_alloc_copy(message_token_id, sizeof(char) * (strnlen(message_token_id, 128) + 1)),
                                                                });
  free(message_tokens);

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
      free(handle->group_id);
      handle->group_id = NULL;
      free(handle->id);
      handle->id = NULL;
      lua_bridge->handles = rectify_array_delete(lua_bridge->handles, t);

      printf("LUA: \"gossip.unsubscribe\" #%d\n", func_ref);
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

  char *message_tokens = rectify_memory_alloc_copy(message, sizeof(char) * (strlen(message) + 1));
  char *message_token_group = strtok(message_tokens, ":");
  char *message_token_id = strtok(NULL, ":");

  if (strncmp(message_token_group, "widget", 128) == 0) {
    if (strncmp(message_token_id, "rune_selected", 128) == 0) {
      uint32_t rune = (uint32_t)lua_tonumber(state, 2);
      gossip_emit(message, sizeof(uint32_t), &rune);
    } else if (strncmp(message_token_id, "color_selected", 128) == 0) {
      uint32_t color = (uint32_t)lua_tonumber(state, 2);
      gossip_emit(message, sizeof(uint32_t), &color);
    } else {
      gossip_emit(message, 0, NULL);
    }
  } else if (strncmp(message_token_group, "system", 128) == 0) {
    if (strncmp(message_token_id, "start", 128) == 0
        || strncmp(message_token_id, "stop", 128) == 0) {
      size_t length = 0;
      const char *system = lua_tolstring(state, 2, &length);
      gossip_emit(message, length + 1, (void *)system);
    }
  } else {
    gossip_emit(message, 0, NULL);
  }

  free(message_tokens);

  return 0;
}
