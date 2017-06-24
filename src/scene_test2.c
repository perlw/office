#include <assert.h>
#include <inttypes.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "bedrock/bedrock.h"

#include "config.h"
#include "messages.h"
#include "scene_test_map.h"
#include "tiles/tiles.h"

typedef struct {
  double offset;
  double timing;
  double since_update;

  uint8_t num_layers;
  Tiles *layers[2];
} SceneTest;

SceneTest *scene_test2_create(const Config *config) {
  SceneTest *scene = calloc(1, sizeof(SceneTest));

  scene->offset = M_PI;
  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;

  scene->num_layers = 2;
  scene->layers[0] = tiles_create(config->res_width, config->res_height, 40, 30);
  scene->layers[1] = tiles_create(config->res_width, config->res_height, 40, 30);

  for (uintmax_t t = 0; t < scene->layers[0]->num_tiles; t++) {
    scene->layers[0]->tilemap[t] = (t % 2 ? 1 : 0);
  }

  for (uintmax_t t = 0; t < scene->layers[1]->num_tiles; t++) {
    scene->layers[1]->tilemap[t] = (t % 2 ? 0 : 2);
  }

  return scene;
}

void scene_test2_destroy(SceneTest *scene) {
  assert(scene);

  tiles_destroy(scene->layers[1]);
  tiles_destroy(scene->layers[0]);

  free(scene);
}

void scene_test2_update(SceneTest *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    scene->offset += 0.025;
    {
      int32_t offset_uniform = picasso_program_uniform_location(scene->layers[0]->program, "offset");
      picasso_program_uniform_float(scene->layers[0]->program, offset_uniform, (float)(cos(scene->offset) / 2.0));
    }
    {
      int32_t offset_uniform = picasso_program_uniform_location(scene->layers[1]->program, "offset");
      picasso_program_uniform_float(scene->layers[1]->program, offset_uniform, (float)(cos(scene->offset) / 2.0));
    }
  }
}

void scene_test2_draw(SceneTest *scene) {
  assert(scene);

  for (int32_t t = scene->num_layers - 1; t >= 0; t--) {
    tiles_draw(scene->layers[t]);
  }
}
