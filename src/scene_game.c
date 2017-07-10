#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"

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

void scene_game_destroy(SceneGame *const scene) {
  assert(scene);

  free(scene);
}

void scene_game_update(SceneGame *const scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;
  }
}

void scene_game_draw(SceneGame *const scene, AsciiBuffer *const screen) {
  assert(scene);
}
