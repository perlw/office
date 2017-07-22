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
#include "input.h"
#include "scenes.h"
#include "screen.h"
#include "systems.h"
#include "ui/ui.h"

#include "scene_drips.h"
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

  occulus_init();

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

  if (picasso_window_init("Office", config->res_width, config->res_height, config->fullscreen, config->gl_debug) != PICASSO_WINDOW_OK) {
    printf("Window: failed to init\n");
    return -1;
  }
  picasso_window_keyboard_callback(&input_keyboard_callback);
  picasso_window_mouse_move_callback(&input_mousemove_callback);
  picasso_window_mouse_button_callback(&input_click_callback);

  screen_init();
  systems_init();

  Scenes *const scenes = scenes_create();
  scenes_register(scenes, &scene_test);
  scenes_register(scenes, &scene_drips);
  scenes_register(scenes, &scene_sound_test);
  scenes_register(scenes, &scene_game);
  scenes_register(scenes, &scene_world_edit);

  gossip_subscribe("game:kill", &game_kill_event, NULL);

  gossip_emit("game:init", 0, NULL);
  scenes_goto(scenes, init_scene);

  const double frame_timing = (config->frame_lock > 0 ? 1.0 / (double)config->frame_lock : 0);
  double next_frame = frame_timing;
  double last_tick = bedrock_time();
  while (!picasso_window_should_close() && !quit_game) {
    double tick = bedrock_time();
    double delta = tick - last_tick;
    last_tick = tick;

    scenes_update(scenes, delta);

    systems_update(delta);

    gossip_update();

    next_frame += delta;
    if (next_frame >= frame_timing) {
      next_frame = 0.0;
      picasso_window_clear();
      screen_render();
      picasso_window_swap();
    }

    picasso_window_update();
  }

  scenes_destroy(scenes);

  systems_kill();
  screen_kill();
  input_kill();

  tome_kill();
  gossip_kill();
  picasso_window_kill();

  occulus_print();

  return 0;
}
