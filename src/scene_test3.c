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
  bool painting;
  int32_t paint_type;

  uint32_t m_x, m_y;

  uint8_t num_layers;
  Tiles *layers[3];
} SceneTest;

typedef struct {
  uint8_t base_tile;
  bool corners[4];
} Tile;
void scene_test3_add_tile(SceneTest *scene, uint32_t x, uint32_t y, uint8_t base_tile) {
  Tile tiles[9] = { 0 };

  printf("Adding tile...\n");
  bool has_corners = (base_tile >= 128);

  printf("\tNeighbors and corners...\n");
  uint32_t n = 0;
  uint32_t min_y = (y > 0 ? y - 1 : 0);
  uint32_t max_y = (y < scene->layers[1]->num_tiles_y - 1 ? y + 1 : scene->layers[1]->num_tiles_y - 2);
  uint32_t min_x = (x > 0 ? x - 1 : 0);
  uint32_t max_x = (x < scene->layers[1]->num_tiles_x - 1 ? x + 1 : scene->layers[1]->num_tiles_x - 2);
  for (uint32_t yy = min_y; yy <= max_y; yy++) {
    printf("\t");
    for (uint32_t xx = min_x; xx <= max_x; xx++) {
      if (xx != x || yy != y) {
        tiles[n].base_tile = scene->map[(yy * 40) + xx];

        // Find corners
        if (xx == 0) {
          tiles[n].corners[0] = 1;
          tiles[n].corners[2] = 1;
        }
        if (yy == 0) {
          tiles[n].corners[0] = 1;
          tiles[n].corners[1] = 1;
        }
        if (xx == 39) {
          tiles[n].corners[1] = 1;
          tiles[n].corners[3] = 1;
        }
        if (yy == 29) {
          tiles[n].corners[2] = 1;
          tiles[n].corners[3] = 1;
        }

        if (!tiles[n].corners[0]) {
         bool i0 = (scene->map[((yy - 1) * 40) + (xx - 1)] > 0);
          bool i1 = (scene->map[((yy - 1) * 40) + xx] > 0);
          bool i2 = (scene->map[(yy * 40) + (xx - 1)] > 0);
          tiles[n].corners[0] = (i0 && i1 && i2);
        }
        if (!tiles[n].corners[1]) {
          bool i0 = (scene->map[((yy - 1) * 40) + (xx + 1)] > 0);
          bool i1 = (scene->map[((yy - 1) * 40) + xx] > 0);
          bool i2 = (scene->map[(yy * 40) + (xx + 1)] > 0);
          tiles[n].corners[1] = (i0 && i1 && i2);
        }
        if (!tiles[n].corners[2]) {
          bool i0 = (scene->map[((yy + 1) * 40) + (xx - 1)] > 0);
          bool i1 = (scene->map[((yy + 1) * 40) + xx] > 0);
          bool i2 = (scene->map[(yy * 40) + (xx - 1)] > 0);
          tiles[n].corners[2] = (i0 && i1 && i2);
        }
        if (!tiles[n].corners[3]) {
          bool i0 = (scene->map[((yy + 1) * 40) + (xx + 1)] > 0);
          bool i1 = (scene->map[((yy + 1) * 40) + xx] > 0);
          bool i2 = (scene->map[(yy * 40) + (xx + 1)] > 0);
          tiles[n].corners[3] = (i0 && i1 && i2);
        }
      } else {
        tiles[n] = (Tile){
          .base_tile = base_tile,
          .corners = { has_corners, has_corners, has_corners, has_corners },
        };
      }

      printf("[ %d %d | %d %d ] ", tiles[n].corners[0], tiles[n].corners[1], tiles[n].corners[2], tiles[n].corners[3]);

      n++;
    }
    printf("\n");
  }

  // Update corners
  //bool have_corners
  tiles[0].corners[3] = has_corners;
  tiles[1].corners[2] = has_corners;
  tiles[1].corners[3] = has_corners;
  tiles[2].corners[2] = has_corners;
  tiles[3].corners[1] = has_corners;
  tiles[3].corners[3] = has_corners;
  tiles[5].corners[0] = has_corners;
  tiles[5].corners[2] = has_corners;
  tiles[6].corners[1] = has_corners;
  tiles[7].corners[0] = has_corners;
  tiles[7].corners[1] = has_corners;
  tiles[8].corners[0] = has_corners;
  printf("\tTweak corners...\n");
  for (uint32_t yy = 0; yy < 3; yy++) {
    printf("\t");
    for (uint32_t xx = 0; xx < 3; xx++) {
      uint32_t n = (yy * 3) + xx;

      if (tiles[n].corners[0] || tiles[n].corners[1] || tiles[n].corners[2] || tiles[n].corners[3]) {
        tiles[n].base_tile = (base_tile > 0 ? base_tile : tiles[n].base_tile);
      } else  {
        tiles[n].base_tile = 0;
      }
      printf("#%d [ %d %d | %d %d ] ", tiles[n].base_tile, tiles[n].corners[0], tiles[n].corners[1], tiles[n].corners[2], tiles[n].corners[3]);
    }
    printf("\n");
  }

  printf("\tPainting...\n");
  uint32_t tile_index = 0;
  for (uint32_t yy = min_y; yy <= max_y; yy++) {
    printf("\t");
    for (uint32_t xx = min_x; xx <= max_x; xx++) {
      uint32_t index = (yy * 40) + xx;
      /*if (tiles[tile_index].base_tile < 128) {
        printf("foo%d, ", tiles[tile_index].base_tile);
        scene->map[index] = tiles[tile_index].base_tile;
        scene->layers[1]->tilemap[index] = tiles[tile_index].base_tile;
        tile_index++;
        continue;
      }*/

      uint8_t tile = tiles[tile_index].base_tile;
      // Find tile from corners
      uint8_t offset = 17;
      for (uintmax_t t = 0; t < num_auto_tiles; t++) {
        bool skip = false;
        for (uintmax_t n = 0; n < 4; n++) {
          if (auto_tiles[t].corners[n] != tiles[tile_index].corners[n]) {
            skip = true;
            break;
          }
        }
        if (!skip) {
          offset = auto_tiles[t].offset;
          break;
        }
      }
      printf("%d, ", offset);
      scene->map[index] = tile;
      scene->layers[1]->tilemap[index] = tile + offset;

      tile_index++;
    }
    printf("\n");
  }

  scene->map_dirty = true;
}

void scene_test3_mouse_event(int32_t id, void *subscriberdata, void *userdata) {
  SceneTest *scene = (SceneTest *)subscriberdata;
  PicassoWindowMouseEvent *event = (PicassoWindowMouseEvent *)userdata;

  uint32_t grid_x = (uint32_t)(event->x / 16.0) + 0.5;
  uint32_t grid_y = (uint32_t)(event->y / 16.0) + 0.5;
  if (event->pressed) {
    gossip_emit(MSG_SOUND_PLAY_TAP, NULL);
    scene->painting = true;
    scene->paint_type = event->button;
  }
  if (event->released) {
    gossip_emit(MSG_SOUND_PLAY_TAP, NULL);
    scene->painting = false;
  }

  if (scene->painting) {
    printf("#%d | %.2f/%.2f | grid %dx%d\n", event->button, event->x, event->y, grid_x, grid_y);

    switch (scene->paint_type) {
      case 0:
        scene_test3_add_tile(scene, grid_x, grid_y, 128);
        break;
      case 1:
        scene_test3_add_tile(scene, grid_x, grid_y, 0);
        break;
      case 2:
        scene_test3_add_tile(scene, grid_x, grid_y, 176);
        break;
    }
  }

  scene->map_dirty = (scene->map_dirty || ((scene->m_x != grid_x) || (scene->m_y != grid_y)));
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
  // Walls
  for (uintmax_t y = 0; y < 30; y++) {
    for (uintmax_t x = 0; x < 40; x++) {
      uintmax_t index = (y * 40) + x;
      if (scene->map[index] == 0) {
        scene->layers[1]->tilemap[index] = 0;
        continue;
      }

      uint8_t tile = scene->map[index];

      // Find corners
      bool corners[4] = { 0 };
      if (x == 0) {
        corners[0] = 1;
        corners[2] = 1;
      }
      if (y == 0) {
        corners[0] = 1;
        corners[1] = 1;
      }
      if (x == 39) {
        corners[1] = 1;
        corners[3] = 1;
      }
      if (y == 29) {
        corners[2] = 1;
        corners[3] = 1;
      }

      if (!corners[0]) {
        bool i0 = (scene->map[((y - 1) * 40) + (x - 1)] > 0);
        bool i1 = (scene->map[((y - 1) * 40) + x] > 0);
        bool i2 = (scene->map[(y * 40) + (x - 1)] > 0);
        corners[0] = (i0 && i1 && i2);
      }
      if (!corners[1]) {
        uintmax_t i0 = (scene->map[((y - 1) * 40) + (x + 1)] > 0);
        uintmax_t i1 = (scene->map[((y - 1) * 40) + x] > 0);
        uintmax_t i2 = (scene->map[(y * 40) + (x + 1)] > 0);
        corners[1] = (i0 && i1 && i2);
      }
      if (!corners[2]) {
        uintmax_t i0 = (scene->map[((y + 1) * 40) + (x - 1)] > 0);
        uintmax_t i1 = (scene->map[((y + 1) * 40) + x] > 0);
        uintmax_t i2 = (scene->map[(y * 40) + (x - 1)] > 0);
        corners[2] = (i0 && i1 && i2);
      }
      if (!corners[3]) {
        uintmax_t i0 = (scene->map[((y + 1) * 40) + (x + 1)] > 0);
        uintmax_t i1 = (scene->map[((y + 1) * 40) + x] > 0);
        uintmax_t i2 = (scene->map[(y * 40) + (x + 1)] > 0);
        corners[3] = (i0 && i1 && i2);
      }

      // Find offset
      bool found = false;
      uint8_t offset = 17;
      for (uintmax_t t = 0; t < num_auto_tiles; t++) {
        bool skip = false;
        for (uintmax_t n = 0; n < 4; n++) {
          if (auto_tiles[t].corners[n] != corners[n]) {
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
        printf("%d %d\n", corners[0], corners[1]);
        printf("%d %d\n", corners[2], corners[3]);
      }

      scene->layers[1]->tilemap[index] = tile + offset;
    }
  }
}

SceneTest *scene_test3_create(const Config *config) {
  SceneTest *scene = calloc(1, sizeof(SceneTest));

  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;
  scene->keyboard_handle = gossip_subscribe(MSG_INPUT_KEYBOARD, &scene_test3_key_event, scene);
  scene->mouse_handle = gossip_subscribe(MSG_INPUT_MOUSE, &scene_test3_mouse_event, scene);
  scene->map = rectify_memory_alloc_copy(map, (40 * 30) * sizeof(uint8_t));
  scene->map_dirty = true;
  scene->painting = false;
  scene->paint_type = 0;
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

      // Floor
      for (uintmax_t y = 0; y < 30; y++) {
        for (uintmax_t x = 0; x < 40; x++) {
          uintmax_t index = (y * 40) + x;
          scene->layers[0]->tilemap[index] = 3;
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
  }
}

void scene_test3_draw(SceneTest *scene) {
  assert(scene);

  for (int32_t t = scene->num_layers - 1; t >= 0; t--) {
    tiles_draw(scene->layers[t]);
  }
}
