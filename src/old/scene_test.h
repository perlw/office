#pragma once

#include "scenes.h"

typedef void SceneTest;

SceneTest *scene_test_create(void);
void scene_test_destroy(SceneTest *const scene);
void scene_test_update(SceneTest *const scene, double delta);
void scene_test_draw(void *const scene, AsciiBuffer *const screen);

Scene scene_test = {
  .name = "test",
  .create = &scene_test_create,
  .destroy = &scene_test_destroy,
  .update = &scene_test_update,
  .draw = &scene_test_draw,
};
