#pragma once

#include "scenes.h"

typedef void SceneGame;

SceneGame *scene_game_create(const Config *config);
void scene_game_destroy(SceneGame *scene);
void scene_game_update(SceneGame *scene, double delta);
void scene_game_draw(SceneGame *scene);

Scene scene_game = {
  .name = "game",
  .create = &scene_game_create,
  .destroy = &scene_game_destroy,
  .update = &scene_game_update,
  .draw = &scene_game_draw,
};
