#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "glad/glad.h"

#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "assets.h"
#include "config.h"
#include "input.h"
#include "messages.h"
#include "scenes.h"
#include "sound.h"
#include "tiles/tiles.h"

#include "scene_test.h"

bool quit_game = false;
void game_kill_event(int32_t id, void *subscriberdata, void *userdata) {
  quit_game = true;
}

void navigate_scene(int32_t id, void *subscriberdata, void *userdata) {
  Scenes *scenes = (Scenes *)subscriberdata;

  switch (id) {
    case MSG_SCENE_PREV:
      scenes_prev(scenes);
      break;

    case MSG_SCENE_NEXT:
      scenes_next(scenes);
      break;
  }
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

int main() {
  srand(time(NULL));

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
  Scene scene_test = {
    .name = "test",
    .create = &scene_test_create,
    .destroy = &scene_test_destroy,
    .update = &scene_test_update,
    .draw = &scene_test_draw,
  };
  Scene scene_test2 = {
    .name = "test2",
    .create = &scene_test2_create,
    .destroy = &scene_test2_destroy,
    .update = &scene_test2_update,
    .draw = &scene_test2_draw,
  };
  scenes_register(scenes, &scene_test);
  scenes_register(scenes, &scene_test2);
  gossip_subscribe(MSG_SCENE_PREV, &navigate_scene, (void *)scenes);
  gossip_subscribe(MSG_SCENE_NEXT, &navigate_scene, (void *)scenes);

  scenes_goto(scenes, "test");

  double last_tick = bedrock_time();

  double frame_timing = (config.frame_lock > 0 ? 1.0 / (double)config.frame_lock : 0);
  double next_frame = frame_timing;

  gossip_subscribe(MSG_GAME_KILL, &game_kill_event, NULL);

  gossip_emit(MSG_GAME_INIT, NULL);

  TilesAscii *fps_tiles = tiles_ascii_create(640, 8, 80, 1);
  for (uint8_t t = 0; t < 80; t++) {
    fps_tiles->asciimap[t].rune = 0;
    fps_tiles->asciimap[t].fore = (GlyphColor){ 0, 0, 0 };
    fps_tiles->asciimap[t].back = (GlyphColor){ 255, 0, 255 };
  }
  uint32_t frames = 0;
  char fps_buffer[32];
  double current_second = 0.0;
  snprintf(fps_buffer, 32, "FPS: 0 | MEM: 0.00kb");
  ascii_text(fps_tiles, 0, 0, 32, fps_buffer);
  while (!picasso_window_should_close() && !quit_game) {
    double tick = bedrock_time();
    double delta = tick - last_tick;
    last_tick = tick;

    MuseArgument arg = {
      .type = MUSE_TYPE_NUMBER,
      .argument = &delta,
    };
    muse_call_name(muse, "update", 1, &arg, 0, NULL);

    soundsys_update(soundsys, delta);
    scenes_update(scenes, delta);

    next_frame += delta;
    if (next_frame >= frame_timing) {
      next_frame = 0.0;

      picasso_window_clear();
      tiles_ascii_draw(fps_tiles);
      scenes_draw(scenes);

      picasso_window_swap();
      frames++;
    }

    picasso_window_update();

    current_second += delta;
    if (current_second >= 1) {
      snprintf(fps_buffer, 32, "FPS: %d | MEM: %.2fkb", frames, (double)occulus_current_allocated() / 1024.0);
      ascii_text(fps_tiles, 0, 0, 32, fps_buffer);

      current_second = 0;
      frames = 0;
    }
  }
  tiles_ascii_destroy(fps_tiles);

  gossip_cleanup();

  scenes_destroy(scenes);

  input_kill();

  soundsys_destroy(soundsys);

  tome_kill();
  muse_destroy(muse);
  picasso_window_kill();

#ifdef MEM_DEBUG
  occulus_print(false);
#endif

  return 0;
}
