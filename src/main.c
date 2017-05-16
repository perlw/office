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
#include "input.h"
#include "messages.h"
#include "scenes.h"
#include "sound.h"
#include "tiles/tiles.h"

#include "scene_test.h"

TilesAscii *debug_overlay;

bool quit_game = false;
void game_kill_event(int32_t id, void *subscriberdata, void *userdata) {
  quit_game = true;
}

void ascii_text(TilesAscii *tiles, uint32_t x, uint32_t y, uintmax_t length, const char *string) {
  assert(tiles);

  if (x >= tiles->ascii_width || y >= tiles->ascii_height) {
    return;
  }

  bool skip = false;
  uint32_t index = (y * tiles->ascii_width) + x;
  uint32_t max = index + (x + length >= tiles->ascii_width ? tiles->ascii_width - x : x + length);
  for (uint32_t t = index, u = 0; t < max; t++, u++) {
    if (string[u] == '\0') {
      skip = true;
    }

    if (!skip) {
      tiles->asciimap[t].rune = string[u];
      tiles->asciimap[t].fore.r = 255;
      tiles->asciimap[t].fore.g = 255;
      tiles->asciimap[t].fore.b = 255;
      tiles->asciimap[t].back.r = 128;
      tiles->asciimap[t].back.g = 0;
      tiles->asciimap[t].back.b = 0;
    } else {
      tiles->asciimap[t].rune = 0;
      tiles->asciimap[t].fore = (GlyphColor){ 0, 0, 0 };
      tiles->asciimap[t].back = (GlyphColor){ 255, 0, 255 };
    }
  }
}

void navigate_scene(int32_t id, void *subscriberdata, void *userdata) {
  Scenes *scenes = (Scenes *)subscriberdata;

  switch (id) {
    case MSG_SCENE_PREV: {
      Scene *scene = scenes_prev(scenes);

      char scene_buffer[32];
      snprintf(scene_buffer, 32, "SCENE: %s", scene->name);
      ascii_text(debug_overlay, 80 - strnlen(scene_buffer, 32), 59, 32, scene_buffer);
      break;
    }

    case MSG_SCENE_NEXT: {
      Scene *scene = scenes_next(scenes);

      char scene_buffer[32];
      snprintf(scene_buffer, 32, "SCENE: %s", scene->name);
      ascii_text(debug_overlay, 80 - strnlen(scene_buffer, 32), 59, 32, scene_buffer);
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

  Muse *muse = muse_create();

  input_action_callback(&input_action, muse);

  input_init();

  Config config = read_config();

  if (picasso_window_init("Office", config.res_width, config.res_height, config.gl_debug) != PICASSO_WINDOW_OK) {
    printf("Window: failed to init\n");
    return -1;
  }
  picasso_window_keyboard_callback(&input_keyboard_callback);

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

  SoundSys *soundsys = soundsys_create();

  Scenes *scenes = scenes_create(&config);
  scenes_register(scenes, &scene_test);
  scenes_register(scenes, &scene_test2);
  scenes_register(scenes, &scene_test3);
  gossip_subscribe(MSG_SCENE_PREV, &navigate_scene, (void *)scenes);
  gossip_subscribe(MSG_SCENE_NEXT, &navigate_scene, (void *)scenes);

  scenes_goto(scenes, init_scene);

  double last_tick = bedrock_time();

  double frame_timing = (config.frame_lock > 0 ? 1.0 / (double)config.frame_lock : 0);
  double next_frame = frame_timing;

  gossip_subscribe(MSG_GAME_KILL, &game_kill_event, NULL);

  gossip_emit(MSG_GAME_INIT, NULL);

  debug_overlay = tiles_ascii_create(config.res_width, config.res_height, config.res_width / 8, config.res_height / 8);
  for (uint32_t t = 0; t < debug_overlay->ascii_size; t++) {
    debug_overlay->asciimap[t].rune = 0;
    debug_overlay->asciimap[t].fore = (GlyphColor){ 0, 0, 0 };
    debug_overlay->asciimap[t].back = (GlyphColor){ 255, 0, 255 };
  }

  uint32_t frames = 0;
  char fps_buffer[32];
  double current_second = 0.0;
  snprintf(fps_buffer, 32, "FPS: 0 | MEM: 0.00kb");
  ascii_text(debug_overlay, 0, 59, 32, fps_buffer);

  char scene_buffer[32];
  snprintf(scene_buffer, 32, "SCENE: %s", init_scene);
  ascii_text(debug_overlay, 80 - strnlen(scene_buffer, 32), 59, 32, scene_buffer);
  while (!picasso_window_should_close() && !quit_game) {
    double tick = bedrock_time();
    double delta = tick - last_tick;
    last_tick = tick;

    soundsys_update(soundsys, delta);
    scenes_update(scenes, delta);

    next_frame += delta;
    if (next_frame >= frame_timing) {
      next_frame = 0.0;
      picasso_window_clear();

      tiles_ascii_draw(debug_overlay);
      scenes_draw(scenes);

      picasso_window_swap();
      frames++;
    }

    picasso_window_update();

    current_second += delta;
    if (current_second >= 1) {
      snprintf(fps_buffer, 32, "FPS: %d | MEM: %.2fkb", frames, (double)occulus_current_allocated() / 1024.0);
      ascii_text(debug_overlay, 0, 59, 32, fps_buffer);

      current_second = 0;
      frames = 0;
    }
  }
  tiles_ascii_destroy(debug_overlay);

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
