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

#include "assets.h"
#include "config.h"
#include "messages.h"
#include "scenes.h"
#include "screen.h"
#include "systems.h"

#include "system_game.h"

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
  kronos_init();

  setup_asset_loaders();

  Config *const config = config_init();

  if (picasso_window_init("Office", config->res_width, config->res_height, config->fullscreen, config->gl_debug) != PICASSO_WINDOW_OK) {
    printf("Window: failed to init\n");
    return -1;
  }

  screen_init();

  kronos_register(&systems);
  kronos_register(&scenes);

  gossip_emit(MSG_GAME_INIT, NULL);

  {
    char buffer[128] = { 0 };
    snprintf(buffer, 128, "scene_%s", init_scene);

    RectifyMap *map = rectify_map_create();
    rectify_map_set_string(map, "scene", buffer);
    gossip_post("scenes", MSG_SCENE_GOTO, map);
  }

  const double frame_timing = (config->frame_lock > 0 ? 1.0 / (double)config->frame_lock : 0);
  double next_frame = frame_timing;
  double last_tick = bedrock_time();
  while (!picasso_window_should_close() && !system_game_should_kill()) {
    double tick = bedrock_time();
    double delta = tick - last_tick;
    last_tick = tick;

    kronos_update(delta);
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

  screen_kill();

  kronos_kill();
  tome_kill();
  gossip_kill();
  picasso_window_kill();

  occulus_print();

  return 0;
}
