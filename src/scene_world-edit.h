#pragma once

#include "scenes.h"

typedef void SceneWorldEdit;

SceneWorldEdit *scene_world_edit_create(const Config *config);
void scene_world_edit_destroy(SceneWorldEdit *const scene);
void scene_world_edit_update(SceneWorldEdit *const scene, double delta);
void scene_world_edit_draw(SceneWorldEdit *const scene, AsciiBuffer *const screen);

Scene scene_world_edit = {
  .name = "world-edit",
  .create = &scene_world_edit_create,
  .destroy = &scene_world_edit_destroy,
  .update = &scene_world_edit_update,
  .draw = &scene_world_edit_draw,
};
