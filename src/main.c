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
#include "debug/debug.h"
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
      tiles->asciimap[t].fore = (GlyphColor){ 255, 255, 255 };
      tiles->asciimap[t].back = (GlyphColor){ 128, 0, 0 };
    } else {
      tiles->asciimap[t].rune = 0;
      tiles->asciimap[t].fore = (GlyphColor){ 0, 0, 0 };
      tiles->asciimap[t].back = (GlyphColor){ 255, 0, 255 };
    }
  }
}

float lerp(float a, float b, float t) {
  return ((1.0f - t) * a) + (t * b);
}

void ascii_graph(TilesAscii *tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t num_values, const float *values) {
  assert(tiles);
  assert(values);

  if (x >= tiles->ascii_width || y >= tiles->ascii_height || x + width > tiles->ascii_width || y + height > tiles->ascii_height) {
    return;
  }

  uint32_t width_per_step = width / num_values;
  for (uint32_t t = 0; t < num_values; t++) {
    uint32_t val_height = (uint32_t)((float)(height - 1) * (1.0 - values[t]));
    uint32_t next_val_height = (t < num_values - 1 ? (uint32_t)((float)(height - 1) * (1.0 - values[t + 1])) : val_height);

    for (uint32_t yy = 0; yy < height; yy++) {
      for (uint32_t xx = 0; xx < width_per_step; xx++) {
        uint32_t fudged_height = (uint32_t)(lerp((float)val_height, (float)next_val_height, (float)xx / (float)width_per_step) + 0.5f);

        uint32_t index = ((y + yy) * tiles->ascii_width) + (x + (t * width_per_step) + xx);
        if (yy >= fudged_height) {
          if (yy == fudged_height) {
            tiles->asciimap[index].rune = '+';
            tiles->asciimap[index].fore = (GlyphColor){ 255, 255, 255 };
          } else if (yy > fudged_height) {
            tiles->asciimap[index].rune = '.';
            tiles->asciimap[index].fore = (GlyphColor){ 128, 128, 128 };
          }
          tiles->asciimap[index].back = (GlyphColor){ 66, 66, 66 };
        } else {
          tiles->asciimap[index].rune = 0;
          tiles->asciimap[index].fore = (GlyphColor){ 0, 0, 0 };
          tiles->asciimap[index].back = (GlyphColor){ 255, 0, 255 };
        }
      }
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

  dummy(10);

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

  uintmax_t raw_mem[10] = { 0 };
  float mem_values[10] = { 0.0f };
  ascii_graph(debug_overlay, 60, 0, 20, 6, 10, mem_values);

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

      {
        uintmax_t raw_new_mem = occulus_current_allocated();
        uintmax_t raw_mem_max = raw_new_mem;
        for (uint32_t t = 0; t < 9; t++) {
          raw_mem[t] = raw_mem[t + 1];
          if (raw_mem[t] > raw_mem_max) {
            raw_mem_max = raw_mem[t];
          }
        }
        raw_mem[9] = raw_new_mem;
        for (uint32_t t = 0; t < 10; t++) {
          mem_values[t] = (float)raw_mem[t] / (float)raw_mem_max;
        }

        ascii_graph(debug_overlay, 60, 0, 20, 6, 10, mem_values);
      }

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
