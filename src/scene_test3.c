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
  double timing;
  double since_update;

  uint8_t num_layers;
  Tiles *layers[2];
} SceneTest;

SceneTest *scene_test3_create(const Config *config) {
  SceneTest *scene = calloc(1, sizeof(SceneTest));

  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;

  scene->num_layers = 2;
  scene->layers[0] = tiles_create(config->res_width, config->res_height, 40, 30);
  scene->layers[1] = tiles_create(config->res_width, config->res_height, 40, 30);

  // Floor
  for (uintmax_t y = 0; y < 30; y++) {
    for (uintmax_t x = 0; x < 40; x++) {
      uintmax_t index = (y * 40) + x;
      scene->layers[0]->tilemap[index] = 3;
    }
  }

  // Walls
  for (uintmax_t y = 0; y < 30; y++) {
    for (uintmax_t x = 0; x < 40; x++) {
      uintmax_t index = (y * 40) + x;
      if (map[index] == 0) {
        continue;
      }

      uint8_t tile = map[index];

      // Find neighbors
      bool neighbors[9] = { 0 };
      if (x == 0) {
        neighbors[0] = 1;
        neighbors[3] = 1;
        neighbors[6] = 1;
      }
      if (y == 0) {
        neighbors[0] = 1;
        neighbors[1] = 1;
        neighbors[2] = 1;
      }
      if (x == 39) {
        neighbors[2] = 1;
        neighbors[5] = 1;
        neighbors[8] = 1;
      }
      if (y == 29) {
        neighbors[6] = 1;
        neighbors[7] = 1;
        neighbors[8] = 1;
      }
      for (intmax_t yy = (y == 0 ? 0 : -1); yy <= (y == 29 ? 0 : 1); yy++) {
        for (intmax_t xx = (x == 0 ? 0 : -1); xx <= (x == 39 ? 0 : 1); xx++) {
          uintmax_t t_index = ((y + yy) * 40) + (x + xx);
          uintmax_t n_index = ((yy + 1) * 3) + (xx + 1);
          neighbors[n_index] = (map[t_index] == tile);
        }
      }
      for (intmax_t yy = 0; yy < 3; yy++) {
        for (intmax_t xx = 0; xx < 3; xx++) {
          intmax_t n_index = (yy * 3) + xx;
          if (!neighbors[n_index]) {
            continue;
          }

          intmax_t u = ((yy - 1) * 3) + xx;
          intmax_t d = ((yy + 1) * 3) + xx;
          intmax_t l = (yy * 3) + (xx - 1);
          intmax_t r = (yy * 3) + (xx + 1);

          uint8_t count = 0;
          if (yy > 0 && u >= 0 && neighbors[u]) {
            count++;
          }
          if (yy < 2 && d < 9 && neighbors[d]) {
            count++;
          }
          if (xx > 0 && l >= 0 && neighbors[l]) {
            count++;
          }
          if (xx < 2 && r < 9 && neighbors[r]) {
            count++;
          }

          if (count < 2) {
            neighbors[n_index] = 0;
          }
        }
      }

      // Find offset
      uint8_t offset = 0;
      for (uintmax_t t = 0; t < num_auto_tiles; t++) {
        bool skip = false;
        for (uintmax_t n = 0; n < 9; n++) {
          if (auto_tiles[t].neighbors[n] != neighbors[n]) {
            skip = true;
            break;
          }
        }
        if (!skip) {
          offset = auto_tiles[t].offset;
          break;
        }
      }
      if (offset == 0) {
        printf("%d %d %d\n", neighbors[0], neighbors[1], neighbors[2]);
        printf("%d %d %d\n", neighbors[3], neighbors[4], neighbors[5]);
        printf("%d %d %d\n\n", neighbors[6], neighbors[7], neighbors[8]);
      }

      scene->layers[1]->tilemap[index] = map[index] + offset;
    }
  }

  return scene;
}

void scene_test3_destroy(SceneTest *scene) {
  assert(scene);

  tiles_destroy(scene->layers[1]);
  tiles_destroy(scene->layers[0]);

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

  for (int32_t t = scene->num_layers - 1; t >= 0; t--) {
    tiles_draw(scene->layers[t]);
  }
}
