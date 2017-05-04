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

#include "scene_test.h"

bool quit_game = false;
void game_kill_event(int32_t id, void *subscriberdata, void *userdata) {
  quit_game = true;
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

  scenes_goto(scenes, "test");

  double last_tick = bedrock_time();

  double frame_timing = (config.frame_lock > 0 ? 1.0 / (double)config.frame_lock : 0);
  double next_frame = frame_timing;

  gossip_subscribe(MSG_GAME_KILL, &game_kill_event, NULL);

  gossip_emit(MSG_GAME_INIT, NULL);

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
      scenes_draw(scenes);
      picasso_window_swap();
    }

    picasso_window_update();
  }

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
