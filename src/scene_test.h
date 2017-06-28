#pragma once

#include "scenes.h"

typedef void SceneTest;

SceneTest *scene_test_create(const Config *config);
void scene_test_destroy(SceneTest *scene);
void scene_test_update(SceneTest *scene, double delta);
void scene_test_draw(void *scene);

Scene scene_test = {
  .name = "test",
  .create = &scene_test_create,
  .destroy = &scene_test_destroy,
  .update = &scene_test_update,
  .draw = &scene_test_draw,
};
