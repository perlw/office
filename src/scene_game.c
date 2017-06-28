#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "config.h"
#include "tiles/tiles.h"

typedef struct {
  double timing;
  double since_update;
} SceneGame;

SceneGame *scene_game_create(const Config *config) {
  SceneGame *scene = calloc(1, sizeof(SceneGame));

  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;

  return scene;
}

void scene_game_destroy(SceneGame *scene) {
  assert(scene);

  free(scene);
}

void scene_game_update(SceneGame *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;
  }
}

void scene_game_draw(SceneGame *scene) {
  assert(scene);
}
