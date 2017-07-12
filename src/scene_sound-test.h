#pragma once

#include "scenes.h"

typedef void SceneSoundTest;

SceneSoundTest *scene_sound_test_create(void);
void scene_sound_test_destroy(SceneSoundTest *const scene);
void scene_sound_test_update(SceneSoundTest *const scene, double delta);
void scene_sound_test_draw(SceneSoundTest *const scene, AsciiBuffer *const screen);

Scene scene_sound_test = {
  .name = "sound-test",
  .create = &scene_sound_test_create,
  .destroy = &scene_sound_test_destroy,
  .update = &scene_sound_test_update,
  .draw = &scene_sound_test_draw,
};
