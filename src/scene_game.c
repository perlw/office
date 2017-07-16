#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"

typedef struct {
  double timing;
  double since_update;
} SceneGame;

SceneGame *scene_game_create(void) {
  SceneGame *scene = calloc(1, sizeof(SceneGame));

  scene->timing = 1.0 / 30.0;
  scene->since_update = 1.0 / (double)((rand() % 29) + 1);

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
