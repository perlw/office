#include <stdio.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include "bedrock/bedrock.h"

#include "config.h"
#include "messages.h"

struct {
  char *name;
  uint32_t val;
} key_names[] = {
  {
    .name = "KEY_SPACE",
    .val = PICASSO_KEY_SPACE,
  },
  {
    .name = "KEY_APOSTROPHE",
    .val = PICASSO_KEY_APOSTROPHE,
  },
  {
    .name = "KEY_COMMA",
    .val = PICASSO_KEY_COMMA,
  },
  {
    .name = "KEY_MINUS",
    .val = PICASSO_KEY_MINUS,
  },
  {
    .name = "KEY_PERIOD",
    .val = PICASSO_KEY_PERIOD,
  },
  {
    .name = "KEY_SLASH",
    .val = PICASSO_KEY_SLASH,
  },
  {
    .name = "KEY_0",
    .val = PICASSO_KEY_0,
  },
  {
    .name = "KEY_1",
    .val = PICASSO_KEY_1,
  },
  {
    .name = "KEY_2",
    .val = PICASSO_KEY_2,
  },
  {
    .name = "KEY_3",
    .val = PICASSO_KEY_3,
  },
  {
    .name = "KEY_4",
    .val = PICASSO_KEY_4,
  },
  {
    .name = "KEY_5",
    .val = PICASSO_KEY_5,
  },
  {
    .name = "KEY_6",
    .val = PICASSO_KEY_6,
  },
  {
    .name = "KEY_7",
    .val = PICASSO_KEY_7,
  },
  {
    .name = "KEY_8",
    .val = PICASSO_KEY_8,
  },
  {
    .name = "KEY_9",
    .val = PICASSO_KEY_9,
  },
  {
    .name = "KEY_SEMICOLON",
    .val = PICASSO_KEY_SEMICOLON,
  },
  {
    .name = "KEY_EQUAL",
    .val = PICASSO_KEY_EQUAL,
  },
  {
    .name = "KEY_A",
    .val = PICASSO_KEY_A,
  },
  {
    .name = "KEY_B",
    .val = PICASSO_KEY_B,
  },
  {
    .name = "KEY_C",
    .val = PICASSO_KEY_C,
  },
  {
    .name = "KEY_D",
    .val = PICASSO_KEY_D,
  },
  {
    .name = "KEY_E",
    .val = PICASSO_KEY_E,
  },
  {
    .name = "KEY_F",
    .val = PICASSO_KEY_F,
  },
  {
    .name = "KEY_G",
    .val = PICASSO_KEY_G,
  },
  {
    .name = "KEY_H",
    .val = PICASSO_KEY_H,
  },
  {
    .name = "KEY_I",
    .val = PICASSO_KEY_I,
  },
  {
    .name = "KEY_J",
    .val = PICASSO_KEY_J,
  },
  {
    .name = "KEY_K",
    .val = PICASSO_KEY_K,
  },
  {
    .name = "KEY_L",
    .val = PICASSO_KEY_L,
  },
  {
    .name = "KEY_M",
    .val = PICASSO_KEY_M,
  },
  {
    .name = "KEY_N",
    .val = PICASSO_KEY_N,
  },
  {
    .name = "KEY_O",
    .val = PICASSO_KEY_O,
  },
  {
    .name = "KEY_P",
    .val = PICASSO_KEY_P,
  },
  {
    .name = "KEY_Q",
    .val = PICASSO_KEY_Q,
  },
  {
    .name = "KEY_R",
    .val = PICASSO_KEY_R,
  },
  {
    .name = "KEY_S",
    .val = PICASSO_KEY_S,
  },
  {
    .name = "KEY_T",
    .val = PICASSO_KEY_T,
  },
  {
    .name = "KEY_U",
    .val = PICASSO_KEY_U,
  },
  {
    .name = "KEY_V",
    .val = PICASSO_KEY_V,
  },
  {
    .name = "KEY_W",
    .val = PICASSO_KEY_W,
  },
  {
    .name = "KEY_X",
    .val = PICASSO_KEY_X,
  },
  {
    .name = "KEY_Y",
    .val = PICASSO_KEY_Y,
  },
  {
    .name = "KEY_Z",
    .val = PICASSO_KEY_Z,
  },
  {
    .name = "KEY_LEFT_BRACKET",
    .val = PICASSO_KEY_LEFT_BRACKET,
  },
  {
    .name = "KEY_BACKSLASH",
    .val = PICASSO_KEY_BACKSLASH,
  },
  {
    .name = "KEY_RIGHT_BRACKET",
    .val = PICASSO_KEY_RIGHT_BRACKET,
  },
  {
    .name = "KEY_GRAVE_ACCENT",
    .val = PICASSO_KEY_GRAVE_ACCENT,
  },
  {
    .name = "KEY_WORLD_1",
    .val = PICASSO_KEY_WORLD_1,
  },
  {
    .name = "KEY_WORLD_2",
    .val = PICASSO_KEY_WORLD_2,
  },

  {
    .name = "KEY_ESCAPE",
    .val = PICASSO_KEY_ESCAPE,
  },
  {
    .name = "KEY_ENTER",
    .val = PICASSO_KEY_ENTER,
  },
  {
    .name = "KEY_TAB",
    .val = PICASSO_KEY_TAB,
  },
  {
    .name = "KEY_BACKSPACE",
    .val = PICASSO_KEY_BACKSPACE,
  },
  {
    .name = "KEY_INSERT",
    .val = PICASSO_KEY_INSERT,
  },
  {
    .name = "KEY_DELETE",
    .val = PICASSO_KEY_DELETE,
  },
  {
    .name = "KEY_RIGHT",
    .val = PICASSO_KEY_RIGHT,
  },
  {
    .name = "KEY_LEFT",
    .val = PICASSO_KEY_LEFT,
  },
  {
    .name = "KEY_DOWN",
    .val = PICASSO_KEY_DOWN,
  },
  {
    .name = "KEY_UP",
    .val = PICASSO_KEY_UP,
  },
  {
    .name = "KEY_PAGE_UP",
    .val = PICASSO_KEY_PAGE_UP,
  },
  {
    .name = "KEY_PAGE_DOWN",
    .val = PICASSO_KEY_PAGE_DOWN,
  },
  {
    .name = "KEY_HOME",
    .val = PICASSO_KEY_HOME,
  },
  {
    .name = "KEY_END",
    .val = PICASSO_KEY_END,
  },
  {
    .name = "KEY_CAPS_LOCK",
    .val = PICASSO_KEY_CAPS_LOCK,
  },
  {
    .name = "KEY_SCROLL_LOCK",
    .val = PICASSO_KEY_SCROLL_LOCK,
  },
  {
    .name = "KEY_NUM_LOCK",
    .val = PICASSO_KEY_NUM_LOCK,
  },
  {
    .name = "KEY_PRINT_SCREEN",
    .val = PICASSO_KEY_PRINT_SCREEN,
  },
  {
    .name = "KEY_PAUSE",
    .val = PICASSO_KEY_PAUSE,
  },
  {
    .name = "KEY_F1",
    .val = PICASSO_KEY_F1,
  },
  {
    .name = "KEY_F2",
    .val = PICASSO_KEY_F2,
  },
  {
    .name = "KEY_F3",
    .val = PICASSO_KEY_F3,
  },
  {
    .name = "KEY_F4",
    .val = PICASSO_KEY_F4,
  },
  {
    .name = "KEY_F5",
    .val = PICASSO_KEY_F5,
  },
  {
    .name = "KEY_F6",
    .val = PICASSO_KEY_F6,
  },
  {
    .name = "KEY_F7",
    .val = PICASSO_KEY_F7,
  },
  {
    .name = "KEY_F8",
    .val = PICASSO_KEY_F8,
  },
  {
    .name = "KEY_F9",
    .val = PICASSO_KEY_F9,
  },
  {
    .name = "KEY_F10",
    .val = PICASSO_KEY_F10,
  },
  {
    .name = "KEY_F11",
    .val = PICASSO_KEY_F11,
  },
  {
    .name = "KEY_F12",
    .val = PICASSO_KEY_F12,
  },
  {
    .name = "KEY_F13",
    .val = PICASSO_KEY_F13,
  },
  {
    .name = "KEY_F14",
    .val = PICASSO_KEY_F14,
  },
  {
    .name = "KEY_F15",
    .val = PICASSO_KEY_F15,
  },
  {
    .name = "KEY_F16",
    .val = PICASSO_KEY_F16,
  },
  {
    .name = "KEY_F17",
    .val = PICASSO_KEY_F17,
  },
  {
    .name = "KEY_F18",
    .val = PICASSO_KEY_F18,
  },
  {
    .name = "KEY_F19",
    .val = PICASSO_KEY_F19,
  },
  {
    .name = "KEY_F20",
    .val = PICASSO_KEY_F20,
  },
  {
    .name = "KEY_F21",
    .val = PICASSO_KEY_F21,
  },
  {
    .name = "KEY_F22",
    .val = PICASSO_KEY_F22,
  },
  {
    .name = "KEY_F23",
    .val = PICASSO_KEY_F23,
  },
  {
    .name = "KEY_F24",
    .val = PICASSO_KEY_F24,
  },
  {
    .name = "KEY_F25",
    .val = PICASSO_KEY_F25,
  },
  {
    .name = "KEY_KP_0",
    .val = PICASSO_KEY_KP_0,
  },
  {
    .name = "KEY_KP_1",
    .val = PICASSO_KEY_KP_1,
  },
  {
    .name = "KEY_KP_2",
    .val = PICASSO_KEY_KP_2,
  },
  {
    .name = "KEY_KP_3",
    .val = PICASSO_KEY_KP_3,
  },
  {
    .name = "KEY_KP_4",
    .val = PICASSO_KEY_KP_4,
  },
  {
    .name = "KEY_KP_5",
    .val = PICASSO_KEY_KP_5,
  },
  {
    .name = "KEY_KP_6",
    .val = PICASSO_KEY_KP_6,
  },
  {
    .name = "KEY_KP_7",
    .val = PICASSO_KEY_KP_7,
  },
  {
    .name = "KEY_KP_8",
    .val = PICASSO_KEY_KP_8,
  },
  {
    .name = "KEY_KP_9",
    .val = PICASSO_KEY_KP_9,
  },
  {
    .name = "KEY_KP_DECIMAL",
    .val = PICASSO_KEY_KP_DECIMAL,
  },
  {
    .name = "KEY_KP_DIVIDE",
    .val = PICASSO_KEY_KP_DIVIDE,
  },
  {
    .name = "KEY_KP_MULTIPLY",
    .val = PICASSO_KEY_KP_MULTIPLY,
  },
  {
    .name = "KEY_KP_SUBTRACT",
    .val = PICASSO_KEY_KP_SUBTRACT,
  },
  {
    .name = "KEY_KP_ADD",
    .val = PICASSO_KEY_KP_ADD,
  },
  {
    .name = "KEY_KP_ENTER",
    .val = PICASSO_KEY_KP_ENTER,
  },
  {
    .name = "KEY_KP_EQUAL",
    .val = PICASSO_KEY_KP_EQUAL,
  },
  {
    .name = "KEY_LEFT_SHIFT",
    .val = PICASSO_KEY_LEFT_SHIFT,
  },
  {
    .name = "KEY_LEFT_CONTROL",
    .val = PICASSO_KEY_LEFT_CONTROL,
  },
  {
    .name = "KEY_LEFT_ALT",
    .val = PICASSO_KEY_LEFT_ALT,
  },
  {
    .name = "KEY_LEFT_SUPER",
    .val = PICASSO_KEY_LEFT_SUPER,
  },
  {
    .name = "KEY_RIGHT_SHIFT",
    .val = PICASSO_KEY_RIGHT_SHIFT,
  },
  {
    .name = "KEY_RIGHT_CONTROL",
    .val = PICASSO_KEY_RIGHT_CONTROL,
  },
  {
    .name = "KEY_RIGHT_ALT",
    .val = PICASSO_KEY_RIGHT_ALT,
  },
  {
    .name = "KEY_RIGHT_SUPER",
    .val = PICASSO_KEY_RIGHT_SUPER,
  },
  {
    .name = "KEY_MENU",
    .val = PICASSO_KEY_MENU,
  },
  {
    .name = NULL,
    .val = 0,
  },
};

int config_internal_resolution(lua_State *state) {
  if (lua_gettop(state) < 2) {
    printf("Config: Too few arguments to function \"resolution\".\n");
    return 0;
  }

  Config *config = (Config *)lua_topointer(state, lua_upvalueindex(1));
  config->res_width = (uint32_t)lua_tonumber(state, 1);
  config->res_height = (uint32_t)lua_tonumber(state, 2);
  config->grid_size_width = (double)config->res_width / (double)config->ascii_width;
  config->grid_size_height = (double)config->res_height / (double)config->ascii_height;

  printf("Config: Resolution set to %dx%d, grid size updated to %.1fx%.1f\n", config->res_width, config->res_height, config->grid_size_width, config->grid_size_height);

  return 0;
}

int config_internal_fullscreen(lua_State *state) {
  Config *config = (Config *)lua_topointer(state, lua_upvalueindex(1));
  config->fullscreen = true;
  printf("Config: Fullscreen\n");
  return 0;
}

int config_internal_gl_debug(lua_State *state) {
  Config *config = (Config *)lua_topointer(state, lua_upvalueindex(1));
  config->gl_debug = true;
  printf("Config: OpenGL debugging\n");
  return 0;
}

int config_internal_frame_lock(lua_State *state) {
  if (lua_gettop(state) < 1) {
    printf("Config: Too few arguments to function \"frame_lock\".\n");
    return 0;
  }

  Config *config = (Config *)lua_topointer(state, lua_upvalueindex(1));
  config->frame_lock = (uint32_t)lua_tonumber(state, 1);
  printf("Config: Frame limit set to %d fps\n", config->frame_lock);

  return 0;
}

int config_internal_bind(lua_State *state) {
  if (lua_gettop(state) < 2) {
    printf("Config: Too few arguments to function \"bind\".\n");
    return 0;
  }

  if (lua_isinteger(state, 1) != 1) {
    printf("Config: Incorrect argument type, expected integer, \"bind\".\n");
    return 0;
  }

  if (lua_isstring(state, 2) != 1) {
    printf("Config: Incorrect argument type, expected string, \"bind\".\n");
    return 0;
  }

  int32_t key = (int32_t)lua_tointeger(state, 1);
  const char *action = lua_tostring(state, 2);
  RectifyMap *map = rectify_map_create();
  rectify_map_set_uint(map, "key", (uint32_t)key);
  rectify_map_set_string(map, "action", (char *)action);

  for (uint32_t t = 0; key_names[t].name; t++) {
    if (key == key_names[t].val) {
      printf("Config: Binding %s to %s\n", key_names[t].name, action);
      break;
    }
  }

  kronos_post("input", MSG_INPUT_BIND, map);

  return 0;
}

Config config_internal = {
  .res_width = 1280,
  .res_height = 720,
  .fullscreen = false,
  .gl_debug = false,
  .frame_lock = 0,
  .ascii_width = 160,
  .ascii_height = 90,
  .grid_size_width = 8.0,
  .grid_size_height = 8.0,
};

Config *const config_init(void) {
  lua_State *state = luaL_newstate();

  lua_pushlightuserdata(state, &config_internal);
  lua_pushcclosure(state, &config_internal_resolution, 1);
  lua_setglobal(state, "resolution");

  lua_pushlightuserdata(state, &config_internal);
  lua_pushcclosure(state, &config_internal_fullscreen, 1);
  lua_setglobal(state, "fullscreen");

  lua_pushlightuserdata(state, &config_internal);
  lua_pushcclosure(state, &config_internal_gl_debug, 1);
  lua_setglobal(state, "gl_debug");

  lua_pushlightuserdata(state, &config_internal);
  lua_pushcclosure(state, &config_internal_frame_lock, 1);
  lua_setglobal(state, "frame_lock");

  lua_pushlightuserdata(state, &config_internal);
  lua_pushcclosure(state, &config_internal_bind, 1);
  lua_setglobal(state, "bind");

  for (uint32_t t = 0; key_names[t].name; t++) {
    lua_pushinteger(state, key_names[t].val);
    lua_setglobal(state, key_names[t].name);
  }

  luaL_loadfile(state, "./config.lua");
  {
    int result = lua_pcall(state, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
      const char *message = lua_tostring(state, -1);
      printf("LUA: %s: %s\n", __func__, message);
      lua_pop(state, 1);
    }
  }

  lua_close(state);

  return &config_internal;
}

Config *const config_get(void) {
  return &config_internal;
}
