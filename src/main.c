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
#include "messages.h"
#include "scenes.h"
#include "sound.h"

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

  Muse *const muse = muse_create();

  input_action_callback(&input_action, muse);

  input_init();

  Config config = read_config();

  if (picasso_window_init("Office", config.res_width, config.res_height, config.gl_debug) != PICASSO_WINDOW_OK) {
    printf("Window: failed to init\n");
    return -1;
  }
  picasso_window_keyboard_callback(&input_keyboard_callback);
  picasso_window_mouse_move_callback(&input_mouse_callback);
  picasso_window_mouse_button_callback(&input_mouse_callback);

  MuseFunctionDef action_def = {
    .name = "action",
    .func = &lua_action,
    .num_arguments = 2,
    .arguments = (MuseType[]){
      MUSE_TYPE_STRING,
      MUSE_TYPE_FUNCTION,
    },
    .userdata = NULL,
  };
  muse_add_func(muse, &action_def);
  muse_load_file(muse, "main.lua");

  SoundSys *const soundsys = soundsys_create();

  DebugOverlay *const debug_overlay = debugoverlay_create(&config);

  Scenes *const scenes = scenes_create(&config);
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

  const double frame_timing = (config.frame_lock > 0 ? 1.0 / (double)config.frame_lock : 0);
  double next_frame = frame_timing;

  gossip_subscribe(MSG_GAME, MSG_GAME_KILL, &game_kill_event, NULL, NULL);

  gossip_emit(MSG_GAME, MSG_GAME_INIT, NULL, NULL);

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

      for (uint32_t t = MSG_SYSTEM_DRAW_LAYER0; t <= MSG_SYSTEM_DRAW; t++) {
        gossip_emit(MSG_SYSTEM, t, NULL, NULL);
      }

      picasso_window_swap();
    }

    picasso_window_update();
  }
  debugoverlay_destroy(debug_overlay);

  scenes_destroy(scenes);
  input_kill();

  soundsys_destroy(soundsys);
  tome_kill();
  muse_destroy(muse);
  gossip_destroy();
  picasso_window_kill();

#ifdef MEM_DEBUG
  occulus_print(false);
#endif

  return 0;
}
