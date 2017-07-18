#pragma once

#include "scenes.h"

typedef void SceneDrips;

SceneDrips *scene_drips_create(void);
void scene_drips_destroy(SceneDrips *const scene);
void scene_drips_update(SceneDrips *const scene, double delta);
void scene_drips_draw(SceneDrips *const scene, AsciiBuffer *const screen);

Scene scene_drips = {
  .name = "drips",
  .create = &scene_drips_create,
  .destroy = &scene_drips_destroy,
  .update = &scene_drips_update,
  .draw = &scene_drips_draw,
};
