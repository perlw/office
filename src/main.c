#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "glad/glad.h"
#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"
#undef MATH_3D_IMPLEMENTATION

#define USE_PICASSO
#define USE_TOME
#include "bedrock/bedrock.h"

#include "assets.h"
#include "config.h"
#include "messages.h"
#include "scenes/scenes.h"
#include "screen.h"
#include "systems/systems.h"

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

  tome_init();
  kronos_init();

  setup_asset_loaders();

  Config *const config = config_init();

  PicassoWindowInit window_init = {
    .width = config->res_width,
    .height = config->res_height,
    .fullscreen = config->fullscreen,
    .gl_debug = config->gl_debug,
  };
  if (picasso_window_init() != PICASSO_WINDOW_OK) {
    printf("Window: failed to init\n");
    return -1;
  }
  PicassoWindow *window = picasso_window_create("Office", &window_init);
  if (!window) {
    return -1;
  }
  config->res_width = window_init.width;
  config->res_height = window_init.height;

  screen_init(window);

  kronos_register(&systems);
  kronos_register(&scenes);

  system_input_bind_input(window);

  kronos_emit(MSG_GAME_INIT, NULL);

  {
    char buffer[128] = { 0 };
    snprintf(buffer, 128, "scene_%s", init_scene);

    RectifyMap *map = rectify_map_create();
    rectify_map_set_string(map, "scene", buffer);
    kronos_post("scenes", MSG_SCENE_GOTO, map);
  }

  const double frame_timing = (config->frame_lock > 0 ? 1.0 / (double)config->frame_lock : 0);
  double next_frame = frame_timing;
  double last_tick = bedrock_time();
  while (!picasso_window_should_close(window) && !system_game_should_kill()) {
    double tick = bedrock_time();
    double delta = tick - last_tick;
    last_tick = tick;

    kronos_update(delta);

    next_frame += delta;
    if (next_frame >= frame_timing) {
      next_frame = 0.0;
      picasso_window_clear(window);
      screen_render();
      picasso_window_swap(window);
    }

    picasso_window_update();
  }

  screen_kill();

  kronos_kill();
  tome_kill();

  picasso_window_destroy(&window);
  picasso_window_kill();

  occulus_print();

  return 0;
}
