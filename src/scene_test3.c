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
  GossipHandle keyboard_handle;
  GossipHandle mouse_handle;
  uint8_t *map;
  bool map_dirty;

  uint32_t m_x, m_y;

  uint8_t num_layers;
  Tiles *layers[3];
} SceneTest;

void scene_test3_mouse_event(int32_t id, void *subscriberdata, void *userdata) {
  SceneTest *scene = (SceneTest *)subscriberdata;
  PicassoWindowMouseEvent *event = (PicassoWindowMouseEvent *)userdata;

  uint32_t grid_x = (uint32_t)(event->x / 16.0) + 0.5;
  uint32_t grid_y = (uint32_t)(event->y / 16.0) + 0.5;
  if (event->pressed) {
    gossip_emit(MSG_SOUND_PLAY_TAP, NULL);

    printf("#%d | %.2f/%.2f | grid %dx%d\n", event->button, event->x, event->y, grid_x, grid_y);

    uintmax_t index = (grid_y * 40) + grid_x;
    switch (event->button) {
      case 0:
        scene->map[index] = 128;
        scene->map_dirty = true;
        break;
      case 1:
        scene->map[index] = 0;
        scene->map_dirty = true;
        break;
      case 2:
        scene->map[index] = 176;
        scene->map_dirty = true;
        break;
    }
  }

  scene->map_dirty = (scene->map_dirty || ((scene->m_x != event->x) || (scene->m_y != event->y)));
  scene->m_x = grid_x;
  scene->m_y = grid_y;
}

void scene_test3_key_event(int32_t id, void *subscriberdata, void *userdata) {
  SceneTest *scene = (SceneTest *)subscriberdata;
  PicassoWindowKeyboardEvent *event = (PicassoWindowKeyboardEvent *)userdata;

  if (event->pressed) {
    gossip_emit(MSG_SOUND_PLAY_TAP, NULL);

    if (event->key == PICASSO_KEY_D) {
      printf("\nMAPDUMP\n");
      for (uintmax_t y = 0; y < 30; y++) {
        for (uintmax_t x = 0; x < 40; x++) {
          uintmax_t index = (y * 40) + x;
          printf("%3d, ", scene->map[index]);
        }
        printf("\n");
      }
      printf("\n");
    }
  }
}

void scene_test3_recalc(SceneTest *scene) {
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
      if (scene->map[index] == 0) {
        scene->layers[1]->tilemap[index] = 0;
        continue;
      }

      uint8_t tile = scene->map[index];

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
          neighbors[n_index] = (scene->map[t_index] == tile);
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

      /*if (scene->layers[1]->tilemap[index] == 0) {
        printf("\n\nFOUND\n");
        printf("%d %d %d\n", neighbors[0], neighbors[1], neighbors[2]);
        printf("%d %d %d\n", neighbors[3], neighbors[4], neighbors[5]);
        printf("%d %d %d\n", neighbors[6], neighbors[7], neighbors[8]);
        printf("FOUND\n\n");
      }*/

      // Find offset
      bool found = false;
      uint8_t offset = 17;
      if (neighbors[4]) {
        for (uintmax_t t = 0; t < num_auto_tiles; t++) {
          bool skip = false;
          for (uintmax_t n = 0; n < 9; n++) {
            if (auto_tiles[t].neighbors[n] != neighbors[n]) {
              skip = true;
              break;
            }
          }
          if (!skip) {
            found = true;
            offset = auto_tiles[t].offset;
            break;
          }
        }
        if (!found) {
          printf("%d %d %d\n", neighbors[0], neighbors[1], neighbors[2]);
          printf("%d %d %d\n", neighbors[3], neighbors[4], neighbors[5]);
          printf("%d %d %d\n\n", neighbors[6], neighbors[7], neighbors[8]);
        }
      }

      scene->layers[1]->tilemap[index] = tile + offset;
    }
  }

  // Mouse
  {
    for (uintmax_t y = 0; y < 30; y++) {
      for (uintmax_t x = 0; x < 40; x++) {
        uintmax_t index = (y * 40) + x;
        scene->layers[2]->tilemap[index] = 0;
      }
    }
    uintmax_t index = (scene->m_y * 40) + scene->m_x;
    scene->layers[2]->tilemap[index] = 4;
  }
}

SceneTest *scene_test3_create(const Config *config) {
  SceneTest *scene = calloc(1, sizeof(SceneTest));

  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;
  scene->keyboard_handle = gossip_subscribe(MSG_INPUT_KEYBOARD, &scene_test3_key_event, scene);
  scene->mouse_handle = gossip_subscribe(MSG_INPUT_MOUSE, &scene_test3_mouse_event, scene);
  scene->map = rectify_memory_alloc_copy(map, (40 * 30) * sizeof(uint8_t));
  scene->map_dirty = false;
  scene->m_x = 0;
  scene->m_y = 0;

  scene->num_layers = 3;
  for (uint32_t t = 0; t < scene->num_layers; t++) {
    scene->layers[t] = tiles_create(config->res_width, config->res_height, 40, 30);
  }

  scene_test3_recalc(scene);

  return scene;
}

void scene_test3_destroy(SceneTest *scene) {
  assert(scene);

  for (uint32_t t = 0; t < scene->num_layers; t++) {
    tiles_destroy(scene->layers[t]);
  }

  gossip_unsubscribe(MSG_INPUT_MOUSE, scene->mouse_handle);
  gossip_unsubscribe(MSG_INPUT_KEYBOARD, scene->keyboard_handle);
  free(scene->map);

  free(scene);
}

void scene_test3_update(SceneTest *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    if (scene->map_dirty) {
      scene->map_dirty = false;
      scene_test3_recalc(scene);
    }
  }
}

void scene_test3_draw(SceneTest *scene) {
  assert(scene);

  for (int32_t t = scene->num_layers - 1; t >= 0; t--) {
    tiles_draw(scene->layers[t]);
  }
}
