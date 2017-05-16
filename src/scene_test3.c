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

uint8_t map[40 * 30] = {
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128, 128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128, 128, 128, 128, 128, 128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 128, 128, 128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128, 128, 128, 128, 128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 128, 128, 128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128, 128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 128, 128, 128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
};

typedef struct {
  uint8_t offset;
  bool neighbors[9];
} AutoTile;
AutoTile auto_tiles[] = {
  {
    .offset = 64,
    .neighbors = {
      0, 0, 0, 0, 0, 0, 0, 0, 0,
    },
  },
  {
    .offset = 0,
    .neighbors = {
      0, 0, 0, 0, 1, 1, 0, 1, 1,
    },
  },
  {
    .offset = 1,
    .neighbors = {
      0, 0, 0, 1, 1, 1, 1, 1, 1,
    },
  },
  {
    .offset = 2,
    .neighbors = {
      0, 0, 0, 1, 1, 0, 1, 1, 0,
    },
  },
  {
    .offset = 3,
    .neighbors = {
      1, 1, 1, 1, 1, 1, 1, 1, 0,
    },
  },
  {
    .offset = 16,
    .neighbors = {
      0, 1, 1, 0, 1, 1, 0, 1, 1,
    },
  },
  {
    .offset = 17,
    .neighbors = {
      1, 1, 1, 1, 1, 1, 1, 1, 1,
    },
  },
  {
    .offset = 18,
    .neighbors = {
      1, 1, 0, 1, 1, 0, 1, 1, 0,
    },
  },
  {
    .offset = 32,
    .neighbors = {
      0, 1, 1, 0, 1, 1, 0, 0, 0,
    },
  },
  {
    .offset = 33,
    .neighbors = {
      1, 1, 1, 1, 1, 1, 0, 0, 0,
    },
  },
  {
    .offset = 34,
    .neighbors = {
      1, 1, 0, 1, 1, 0, 0, 0, 0,
    },
  },
  {
    .offset = 4,
    .neighbors = {
      1, 1, 1, 1, 1, 1, 0, 1, 1,
    },
  },
  {
    .offset = 19,
    .neighbors = {
      1, 1, 0, 1, 1, 1, 1, 1, 1,
    },
  },
  {
    .offset = 20,
    .neighbors = {
      0, 1, 1, 1, 1, 1, 1, 1, 1,
    },
  },
  {
    .offset = 35,
    .neighbors = {
      0, 1, 1, 1, 1, 1, 1, 1, 0,
    },
  },
  {
    .offset = 36,
    .neighbors = {
      1, 1, 0, 1, 1, 1, 0, 1, 1,
    },
  },
};
uintmax_t num_auto_tiles = 16;

SceneTest *scene_test3_create(const Config *config) {
  SceneTest *scene = calloc(1, sizeof(SceneTest));

  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;

  scene->tiles = tiles_create(config->res_width, config->res_height, 40, 30);
  for (uintmax_t y = 0; y < 30; y++) {
    for (uintmax_t x = 0; x < 40; x++) {
      uintmax_t index = (y * 40) + x;
      if (map[index] == 0) {
        continue;
      }

      uint8_t tile = map[index];
      if (tile < 128) {
        scene->tiles->tilemap[index] = tile;
        continue;
      }

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

      scene->tiles->tilemap[index] = map[index] + offset;
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
