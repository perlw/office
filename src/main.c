#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "glad/glad.h"
#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"
#undef MATH_3D_IMPLEMENTATION

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "assets.h"
#include "config.h"
#include "debug/debug.h"
#include "input.h"
#include "lua_bridge.h"
#include "scenes.h"
#include "sound.h"
#include "ui/ui.h"

#include "scene_game.h"
#include "scene_sound-test.h"
#include "scene_test.h"
#include "scene_world-edit.h"

bool quit_game = false;
void game_kill_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  quit_game = true;
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
  input_init();

  const Config *const config = config_init();

  LuaBridge *const lua_bridge = lua_bridge_create();

  if (picasso_window_init("Office", config->res_width, config->res_height, config->gl_debug) != PICASSO_WINDOW_OK) {
    printf("Window: failed to init\n");
    return -1;
  }
  picasso_window_keyboard_callback(&input_keyboard_callback);
  picasso_window_mouse_move_callback(&input_mousemove_callback);
  picasso_window_mouse_button_callback(&input_click_callback);

  SoundSys *const soundsys = soundsys_create();

  Scenes *const scenes = scenes_create();
  scenes_register(scenes, &scene_test);
  scenes_register(scenes, &scene_game);
  scenes_register(scenes, &scene_sound_test);
  scenes_register(scenes, &scene_world_edit);

  DebugOverlay *const debug_overlay = debugoverlay_create();

  AsciiBuffer *ascii_screen = ascii_buffer_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);

  gossip_subscribe("game:kill", &game_kill_event, NULL);

  gossip_emit("game:init", NULL);
  scenes_goto(scenes, init_scene);

  const double frame_timing = (config->frame_lock > 0 ? 1.0 / (double)config->frame_lock : 0);
  double next_frame = frame_timing;
  double last_tick = bedrock_time();
  while (!picasso_window_should_close() && !quit_game) {
    double tick = bedrock_time();
    double delta = tick - last_tick;
    last_tick = tick;

    gossip_gc();

    soundsys_update(soundsys, delta);
    scenes_update(scenes, delta);
    ui_update(delta);
    debugoverlay_update(debug_overlay, delta);

    next_frame += delta;
    if (next_frame >= frame_timing) {
      next_frame = 0.0;
      picasso_window_clear();

      scenes_draw(scenes, ascii_screen);
      ui_draw(ascii_screen);
      debugoverlay_draw(debug_overlay, ascii_screen);

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

  gossip_destroy();
  picasso_window_kill();

#ifdef MEM_DEBUG
  occulus_print(false);
#endif

  return 0;
}
