#pragma once

#include "scenes.h"

typedef void SceneSoundTest;

SceneSoundTest *scene_sound_test_create(const Config *config);
void scene_sound_test_destroy(SceneSoundTest *scene);
void scene_sound_test_update(SceneSoundTest *scene, double delta);
void scene_sound_test_draw(SceneSoundTest *scene);

Scene scene_sound_test = {
  .name = "sound-test",
  .create = &scene_sound_test_create,
  .destroy = &scene_sound_test_destroy,
  .update = &scene_sound_test_update,
  .draw = &scene_sound_test_draw,
};
