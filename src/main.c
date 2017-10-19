#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "glad/glad.h"
#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"
#undef MATH_3D_IMPLEMENTATION

#define USE_KRONOS
#define USE_RECTIFY
#define USE_TOME
#include "bedrock/bedrock.h"

#define USE_ASSETS
#define USE_CONFIG
#define USE_MESSAGES
#define USE_SCENES
#define USE_SYSTEMS
#include "main.h"

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

  config_init();

  kronos_register(&systems);
  kronos_register(&scenes);

  kronos_emit(MSG_GAME_INIT, NULL);

  {
    char buffer[128] = { 0 };
    snprintf(buffer, 128, "scene_%s", init_scene);

    RectifyMap *map = rectify_map_create();
    rectify_map_set_string(map, "scene", buffer);
    kronos_post("scenes", MSG_SCENE_GOTO, map, NULL);
  }

  //rectify_map_print(kronos_post_immediate("foo", MSG_DEBUG_TEST, NULL));

  double tick = 0;
  double last_tick = bedrock_time();
  while (!kronos_should_halt()) {
    tick = bedrock_time();
    kronos_update(tick - last_tick);
    last_tick = tick;
  }

  kronos_kill();
  tome_kill();

  occulus_print();

  return 0;
}
