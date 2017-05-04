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
  double offset;
  double timing;
  double since_update;

  Tiles *tiles;
} SceneTest;

SceneTest *scene_test2_create(const Config *config) {
  SceneTest *scene = calloc(1, sizeof(SceneTest));

  scene->offset = M_PI;
  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;

  scene->tiles = tiles_create(config->res_width, config->res_height, 40, 30);
  for (uintmax_t y = 0; y < 30; y++) {
    for (uintmax_t x = 0; x < 40; x++) {
      uintmax_t index = (y * 40) + x;
      bool has_tile = (x + (y % 2)) % 2;
      scene->tiles->tilemap[index] = (has_tile ? (x % 2) + 1 : 0);
    }
  }

  scene->tiles->tilemap[(14 * 40) + 18] = 0;
  scene->tiles->tilemap[(14 * 40) + 19] = 3;
  scene->tiles->tilemap[(14 * 40) + 20] = 4;
  scene->tiles->tilemap[(14 * 40) + 21] = 0;

  return scene;
}

void scene_test2_destroy(SceneTest *scene) {
  assert(scene);

  tiles_destroy(scene->tiles);

  free(scene);
}

void scene_test2_update(SceneTest *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    scene->offset += 0.025;
    int32_t offset_uniform = picasso_program_uniform_location(scene->tiles->program, "offset");
    picasso_program_uniform_float(scene->tiles->program, offset_uniform, cos(scene->offset) / 2.0);
  }
}

void scene_test2_draw(SceneTest *scene) {
  assert(scene);

  tiles_draw(scene->tiles);
}
