#include <assert.h>
#include <inttypes.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "bedrock/bedrock.h"

#include "config.h"
#include "messages.h"
#include "tiles/tiles.h"

typedef struct {
  double timing;
  double since_update;

  Tiles *tiles;
} SceneTest;

SceneTest *scene_test3_create(const Config *config) {
  SceneTest *scene = calloc(1, sizeof(SceneTest));

  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;

  scene->tiles = tiles_create(config->res_width, config->res_height, 40, 30);
  for (uintmax_t y = 0; y < 30; y++) {
    for (uintmax_t x = 0; x < 40; x++) {
      uintmax_t index = (y * 40) + x;

      scene->tiles->tilemap[index] = 1;
    }
  }

  return scene;
}

void scene_test3_destroy(SceneTest *scene) {
  assert(scene);

  tiles_destroy(scene->tiles);

  free(scene);
}

void scene_test3_update(SceneTest *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;
  }
}

void scene_test3_draw(SceneTest *scene) {
  assert(scene);

  tiles_draw(scene->tiles);
}
