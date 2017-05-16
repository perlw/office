#pragma once

#include "scenes.h"

typedef void SceneTest;

// +SceneTest
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
// -SceneTest

// +SceneTest2
Scene scene_test2_setup(void);
SceneTest *scene_test2_create(const Config *config);
void scene_test2_destroy(SceneTest *scene);
void scene_test2_update(SceneTest *scene, double delta);
void scene_test2_draw(SceneTest *scene);

Scene scene_test2 = {
  .name = "test2",
  .create = &scene_test2_create,
  .destroy = &scene_test2_destroy,
  .update = &scene_test2_update,
  .draw = &scene_test2_draw,
};
// -SceneTest2

// +SceneTest3
Scene scene_test3_setup(void);
SceneTest *scene_test3_create(const Config *config);
void scene_test3_destroy(SceneTest *scene);
void scene_test3_update(SceneTest *scene, double delta);
void scene_test3_draw(SceneTest *scene);

Scene scene_test3 = {
  .name = "playable_map",
  .create = &scene_test3_create,
  .destroy = &scene_test3_destroy,
  .update = &scene_test3_update,
  .draw = &scene_test3_draw,
};
// -SceneTest3
