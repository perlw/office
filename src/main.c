#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "glad/glad.h"
#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"
#undef MATH_3D_IMPLEMENTATION

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "assets.h"
#include "config.h"
#include "debug/debug.h"
#include "input.h"
#include "lua_bridge.h"
#include "messages.h"
#include "scenes.h"
#include "sound.h"
#include "ui/ui.h"

#include "scene_game.h"
#include "scene_sound-test.h"
#include "scene_test.h"
#include "scene_world-edit.h"

bool quit_game = false;
void game_kill_event(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata) {
  quit_game = true;
}

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

int main(int argc, char **argv) {
  srand(time(NULL));

  char *init_scene = "test";
  // +Flags
  // Module idea. Stackbased, popping off values etc?
  for (int t = 0; t < argc; t++) {
    if (argv[t][0] == '-' && argv[t][1] == '-') {
      if (strncmp("scene", &argv[t][2], 32) == 0) {
        t++;
        if (t < argc) {
          init_scene = argv[t];
        }
      }
    }
  }
  // -Flags

  gossip_init();
  tome_init();
  setup_asset_loaders();

  lua_State *state = luaL_newstate();
  luaL_openlibs(state);
  LuaBridge *const lua_bridge = lua_bridge_create(state);

  input_action_callback(&input_action, state);

  input_init();

  const Config *const config = config_init();

  if (picasso_window_init("Office", config->res_width, config->res_height, config->gl_debug) != PICASSO_WINDOW_OK) {
    printf("Window: failed to init\n");
    return -1;
  }
  picasso_window_keyboard_callback(&input_keyboard_callback);
  picasso_window_mouse_move_callback(&input_mouse_callback);
  picasso_window_mouse_button_callback(&input_mouse_callback);

  {
    lua_pushcclosure(state, &internal_action, 0);
    lua_setglobal(state, "action");

    register_lua_module(state, "testlib", internal_lua_testlib);
    register_lua_module(state, "ui", internal_lua_ui);

    luaL_loadfile(state, "main.lua");
    {
      int result = lua_pcall(state, 0, LUA_MULTRET, 0);
      if (result != LUA_OK) {
        const char *message = lua_tostring(state, -1);
        printf("LUA: %s: %s\n", __func__, message);
        lua_pop(state, 1);
      }
    }
  }

  SoundSys *const soundsys = soundsys_create();

  DebugOverlay *const debug_overlay = debugoverlay_create();

  Scenes *const scenes = scenes_create();
  scenes_register(scenes, &scene_test);
  scenes_register(scenes, &scene_game);
  scenes_register(scenes, &scene_sound_test);
  scenes_register(scenes, &scene_world_edit);
  scenes_goto(scenes, init_scene);

  AsciiBuffer *ascii_screen = ascii_buffer_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);

  gossip_subscribe(MSG_GAME, MSG_GAME_KILL, &game_kill_event, NULL, NULL);
  gossip_emit(MSG_GAME, MSG_GAME_INIT, NULL, NULL);

  const double frame_timing = (config->frame_lock > 0 ? 1.0 / (double)config->frame_lock : 0);
  double next_frame = frame_timing;
  double last_tick = bedrock_time();
  while (!picasso_window_should_close() && !quit_game) {
    double tick = bedrock_time();
    double delta = tick - last_tick;
    last_tick = tick;

    gossip_emit(MSG_SYSTEM, MSG_SYSTEM_UPDATE, NULL, &delta);

    next_frame += delta;
    if (next_frame >= frame_timing) {
      next_frame = 0.0;
      picasso_window_clear();

      for (uint32_t t = MSG_SYSTEM_DRAW; t <= MSG_SYSTEM_DRAW_TOP; t++) {
        gossip_emit(MSG_SYSTEM, t, NULL, ascii_screen);
      }

      ascii_buffer_draw(ascii_screen);

      picasso_window_swap();
    }

    picasso_window_update();
  }
  debugoverlay_destroy(debug_overlay);

  scenes_destroy(scenes);
  input_kill();

  ascii_buffer_destroy(ascii_screen);

  soundsys_destroy(soundsys);
  tome_kill();

  lua_bridge_destroy(lua_bridge);
  lua_close(state);

  gossip_destroy();
  picasso_window_kill();

#ifdef MEM_DEBUG
  occulus_print(false);
#endif

  return 0;
}
