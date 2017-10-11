#pragma once

#define USE_KRONOS
#include "scenes.h"

typedef void SceneDummy;

SceneDummy *scene_dummy_create(void);
void scene_dummy_destroy(SceneDummy *const scene);
void scene_dummy_update(SceneDummy *const scene, double delta);
void scene_dummy_draw(void *const scene, AsciiBuffer *const screen);

Scene scene_dummy = {
  .name = "dummy",
  .create = &scene_dummy_create,
  .destroy = &scene_dummy_destroy,
  .update = &scene_dummy_update,
  .draw = &scene_dummy_draw,
};
