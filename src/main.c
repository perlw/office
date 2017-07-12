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
#include "messages.h"
#include "scenes.h"
#include "sound.h"
#include "ui/ui.h"

#include "scene_game.h"
#include "scene_sound-test.h"
#include "scene_test.h"
#include "scene_world-edit.h"

bool quit_game = false;
void game_kill_event(uint32_t id, void *const subscriberdata, void *const userdata) {
  quit_game = true;
}

void navigate_scene(uint32_t id, void *const subscriberdata, void *const userdata) {
  Scenes *scenes = (Scenes *)subscriberdata;

  switch (id) {
    case MSG_SCENE_PREV: {
      Scene *scene = scenes_prev(scenes);
      gossip_emit(MSG_SCENE, MSG_SCENE_CHANGED, NULL, scene);
      break;
    }

    case MSG_SCENE_NEXT: {
      Scene *scene = scenes_next(scenes);
      gossip_emit(MSG_SCENE, MSG_SCENE_CHANGED, NULL, scene);
      break;
    }
  }
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

struct {
  char *name;
  uint32_t key;
  struct {
    char *name;
    uint32_t key;
  } children[10];
} gossip_keys[] = {
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
  int32_t func_ref;

  uint32_t group;
  uint32_t id;
} GossipLuaPackage;

void internal_lua_gossip_call(uint32_t id, void *const subscriberdata, void *const userdata) {
  GossipLuaPackage *pkg = (GossipLuaPackage *)subscriberdata;

  lua_rawgeti(pkg->state, LUA_REGISTRYINDEX, pkg->func_ref);

  uint32_t num_args = 0;
  switch (pkg->group) {
    case MSG_SCENE:
      switch (pkg->id) {
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
}

int internal_lua_gossip_subscribe(lua_State *state) {
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

  int32_t group = -1;
  int32_t id = -1;
  char *tokens = rectify_memory_alloc_copy(msg_name, strlen(msg_name) + 1);
  char *token_group = strtok(tokens, ":");
  char *token_id = strtok(NULL, ":");
  for (uint32_t t = 0; gossip_keys[t].name; t++) {
    if (strcmp(token_group, gossip_keys[t].name) == 0) {
      group = gossip_keys[t].key;
      for (uint32_t u = 0; gossip_keys[t].children[u].name; u++) {
        if (strcmp(token_id, gossip_keys[t].children[u].name) == 0) {
          id = gossip_keys[t].children[u].key;
          break;
        }
      }
      break;
    }
  }
  free(tokens);
  if (group < 0 || id < 0) {
    return 0;
  }

  GossipLuaPackage *pkg = calloc(1, sizeof(GossipLuaPackage));
  *pkg = (GossipLuaPackage){
    .state = state,
    .func_ref = func_ref,
    .group = group,
    .id = id,
  };
  gossip_subscribe(group, id, &internal_lua_gossip_call, pkg, NULL);
  return 0;
}

int internal_lua_gossip_emit(lua_State *state) {
  if (lua_gettop(state) < 1) {
    printf("Main: Too few arguments to function \"gossip.emit\".\n");
    return 0;
  }

  if (!lua_isstring(state, 1)) {
    printf("Main: Incorrect argumenttypes, expected string, function.\n");
    return 0;
  }

  const char *msg_name = lua_tolstring(state, 1, NULL);

  int32_t group = -1;
  int32_t id = -1;
  char *tokens = rectify_memory_alloc_copy(msg_name, strlen(msg_name) + 1);
  char *token_group = strtok(tokens, ":");
  char *token_id = strtok(NULL, ":");
  for (uint32_t t = 0; gossip_keys[t].name; t++) {
    if (strcmp(token_group, gossip_keys[t].name) == 0) {
      group = gossip_keys[t].key;
      for (uint32_t u = 0; gossip_keys[t].children[u].name; u++) {
        if (strcmp(token_id, gossip_keys[t].children[u].name) == 0) {
          id = gossip_keys[t].children[u].key;
          break;
        }
      }
      break;
    }
  }
  free(tokens);
  if (group < 0 || id < 0) {
    return 0;
  }

  gossip_emit(group, id, NULL, NULL);
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

int internal_lua_gossiplib(lua_State *state) {
  lua_newtable(state);

  lua_pushcfunction(state, &internal_lua_gossip_subscribe);
  lua_setfield(state, -2, "subscribe");
  lua_pushcfunction(state, &internal_lua_gossip_emit);
  lua_setfield(state, -2, "emit");

  for (uint32_t t = 0; gossip_keys[t].name; t++) {
    lua_pushnumber(state, gossip_keys[t].key);
    lua_setfield(state, -2, gossip_keys[t].name);
  }

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
    register_lua_module(state, "gossip", internal_lua_gossiplib);
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
  gossip_subscribe(MSG_SCENE, MSG_SCENE_PREV, &navigate_scene, (void *)scenes, NULL);
  gossip_subscribe(MSG_SCENE, MSG_SCENE_NEXT, &navigate_scene, (void *)scenes, NULL);

  {
    Scene *const scene = scenes_goto(scenes, init_scene);
    gossip_emit(MSG_SCENE, MSG_SCENE_CHANGED, NULL, scene);
  }

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

  lua_close(state);

  gossip_destroy();
  picasso_window_kill();

#ifdef MEM_DEBUG
  occulus_print(false);
#endif

  return 0;
}
