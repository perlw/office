#pragma once

#include "scenes.h"

typedef void SceneWorldEdit;

SceneWorldEdit *scene_world_edit_create(const Config *config);
void scene_world_edit_destroy(SceneWorldEdit *scene);
void scene_world_edit_update(SceneWorldEdit *scene, double delta);
void scene_world_edit_draw(SceneWorldEdit *scene);

Scene scene_world_edit = {
  .name = "world-edit",
  .create = &scene_world_edit_create,
  .destroy = &scene_world_edit_destroy,
  .update = &scene_world_edit_update,
  .draw = &scene_world_edit_draw,
};
