#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "config.h"
#include "tiles/tiles.h"

typedef struct {
  double offset;
  double timing;
  double since_update;

  uint32_t num_layers;
  Tiles **layers;
} SceneGame;

SceneGame *scene_game_create(const Config *config) {
  SceneGame *scene = calloc(1, sizeof(SceneGame));

  scene->offset = 0.0;
  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;

  scene->num_layers = 2;
  scene->layers = calloc(scene->num_layers, sizeof(Tiles));
  for (uint32_t t = 0; t < scene->num_layers; t++) {
    scene->layers[t] = tiles_create(config->res_width, config->res_height, 40, 30);
  }

  return scene;
}

void scene_game_destroy(SceneGame *scene) {
  assert(scene);

  for (uint32_t t = 0; t < scene->num_layers; t++) {
    tiles_destroy(scene->layers[t]);
  }
  free(scene->layers);

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

  for (uint32_t t = 0; t < scene->num_layers; t++) {
    tiles_draw(scene->layers[t]);
  }
}
