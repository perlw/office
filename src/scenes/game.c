#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "cJSON.h"

#define USE_ARCHIVIST
#define USE_KRONOS
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#define USE_ASCII
#define USE_CONFIG
#define USE_MESSAGES
#include "main.h"

typedef struct {
  char *id;
  Glyph glyph;
  bool collides;
} TileDef;

typedef struct {
  TileDef *def;
} Tile;

typedef struct {
  uint32_t p_x;
  uint32_t p_y;

  TileDef *tiledefs;
  Tile *tilemap;

  Surface *world;
} SceneGame;

SceneGame *scene_game_start(void);
void scene_game_stop(void **scene);
void scene_game_update(SceneGame *scene, double delta);
RectifyMap *scene_game_message(SceneGame *scene, uint32_t id, RectifyMap *const map);

KronosSystem scene_game = {
  .name = "scene_game",
  .frames = 30,
  .start = &scene_game_start,
  .stop = &scene_game_stop,
  .update = &scene_game_update,
  .message = &scene_game_message,
};

#define MAP_X 20
#define MAP_Y 20
#define MAP_SIZE (MAP_X * MAP_Y)

void scene_game_internal_build_map(SceneGame *scene);

SceneGame *scene_game_start(void) {
  SceneGame *scene = calloc(1, sizeof(SceneGame));

  Config *const config = config_get();

  scene->p_x = 2;
  scene->p_y = 2;

  scene->tiledefs = rectify_array_alloc(10, sizeof(TileDef));
  scene->tilemap = calloc(1, sizeof(Tile) * MAP_SIZE);
  scene_game_internal_build_map(scene);

  scene->world = surface_create(0, 0, config->ascii_width, config->ascii_height);

  {
    size_t num_bytes = 0;
    uint8_t *data = NULL;
    archivist_read_file("tiledefs.json", &num_bytes, &data);

    cJSON *root = cJSON_Parse((const char *)data);
    for (int32_t t = 0; t < cJSON_GetArraySize(root); t++) {
      cJSON *item = cJSON_GetArrayItem(root, t);

      cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");
      cJSON *rune = cJSON_GetObjectItemCaseSensitive(item, "rune");
      cJSON *fore_color = cJSON_GetObjectItemCaseSensitive(item, "fore_color");
      cJSON *back_color = cJSON_GetObjectItemCaseSensitive(item, "back_color");
      cJSON *tags = cJSON_GetObjectItemCaseSensitive(item, "tags");

      TileDef def = {
        .id = rectify_memory_alloc_copy(id->valuestring, sizeof(char) * (strnlen(id->valuestring, 128) + 1)),
        .glyph = (Glyph){
          .rune = rune->valueint,
          .fore = glyphcolor_hex(fore_color->valueint),
          .back = glyphcolor_hex(back_color->valueint),
        },
        .collides = false,
      };
      for (int32_t u = 0; u < cJSON_GetArraySize(tags); u++) {
        cJSON *tag = cJSON_GetArrayItem(tags, u);
        if (strncmp(tag->valuestring, "wall", 128) == 0) {
          def.collides = true;
        }
      }

      printf("%s => { %d|%c, (%d %d %d) (%d %d %d) }\n", def.id, def.glyph.rune, def.glyph.rune, def.glyph.fore.r, def.glyph.fore.g, def.glyph.fore.b, def.glyph.back.r, def.glyph.back.g, def.glyph.back.b);

      scene->tiledefs = rectify_array_push(scene->tiledefs, &def);
    }
    cJSON_Delete(root);

    free(data);
  }
  scene_game_internal_build_map(scene);

  return scene;
}

void scene_game_stop(void **scene) {
  SceneGame *ptr = *scene;
  assert(ptr && scene);

  free(ptr->tilemap);
  for (uint32_t t = 0; t < rectify_array_size(ptr->tiledefs); t++) {
    free(ptr->tiledefs[t].id);
  }
  rectify_array_free((void **)&ptr->tiledefs);

  surface_destroy(&ptr->world);

  free(ptr);
  *scene = NULL;
}

void scene_game_update(SceneGame *scene, double delta) {
  assert(scene);

  surface_clear(scene->world, (Glyph){
                                .rune = ' ',
                                .fore = 0,
                                .back = 0,
                              });

  // +Draw world
  uint32_t offset_x = 20;
  uint32_t offset_y = 20;
  for (uint32_t y = 0; y < MAP_Y; y++) {
    uint32_t i = (y * MAP_Y);
    for (uint32_t x = 0; x < MAP_X; x++) {
      Tile *tile = &scene->tilemap[i + x];

      if (tile->def) {
        surface_glyph(scene->world, x + offset_x, y + offset_y, tile->def->glyph);
      } else {
        surface_glyph(scene->world, x + offset_x, y + offset_y, (Glyph){
                                                                  .rune = '?',
                                                                  .fore = glyphcolor_hex(0xffffff),
                                                                  .back = glyphcolor_hex(0xff0000),
                                                                });
      }
    }
  }
  // -Draw world

  // +Draw player
  surface_glyph(scene->world, scene->p_x + offset_x, scene->p_y + offset_y, (Glyph){
                                                                              .rune = 1,
                                                                              .fore = (GlyphColor){ 255, 255, 255 },
                                                                              .back = 0,
                                                                            });
  // -Draw player
}

RectifyMap *scene_game_message(SceneGame *scene, uint32_t id, RectifyMap *const map) {
  assert(scene);

  uint32_t o_x = scene->p_x;
  uint32_t o_y = scene->p_y;
  switch (id) {
    case MSG_INPUT_ACTION: {
      const char *action = rectify_map_get_string(map, "action");
      if (strncmp(action, "plr_move_uplt", 128) == 0) {
        scene->p_x = (scene->p_x > 0 ? scene->p_x - 1 : scene->p_x);
        scene->p_y = (scene->p_y > 0 ? scene->p_y - 1 : scene->p_y);
      } else if (strncmp(action, "plr_move_up", 128) == 0) {
        scene->p_y = (scene->p_y > 0 ? scene->p_y - 1 : scene->p_y);
      } else if (strncmp(action, "plr_move_uprt", 128) == 0) {
        scene->p_x = (scene->p_x < MAP_X - 1 ? scene->p_x + 1 : scene->p_x);
        scene->p_y = (scene->p_y > 0 ? scene->p_y - 1 : scene->p_y);
      } else if (strncmp(action, "plr_move_lt", 128) == 0) {
        scene->p_x = (scene->p_x > 0 ? scene->p_x - 1 : scene->p_x);
      } else if (strncmp(action, "plr_move_rt", 128) == 0) {
        scene->p_x = (scene->p_x < MAP_X - 1 ? scene->p_x + 1 : scene->p_x);
      } else if (strncmp(action, "plr_move_dnlt", 128) == 0) {
        scene->p_x = (scene->p_x > 0 ? scene->p_x - 1 : scene->p_x);
        scene->p_y = (scene->p_y < MAP_Y - 1 ? scene->p_y + 1 : scene->p_y);
      } else if (strncmp(action, "plr_move_dn", 128) == 0) {
        scene->p_y = (scene->p_y < MAP_Y - 1 ? scene->p_y + 1 : scene->p_y);
      } else if (strncmp(action, "plr_move_dnrt", 128) == 0) {
        scene->p_x = (scene->p_x < MAP_X - 1 ? scene->p_x + 1 : scene->p_x);
        scene->p_y = (scene->p_y < MAP_Y - 1 ? scene->p_y + 1 : scene->p_y);
      }

      break;
    }

    case MSG_SYSTEM_RENDER: {
      surface_draw(scene->world, *(AsciiBuffer **)rectify_map_get(map, "screen"));
      break;
    }
  }

  Tile *current = &scene->tilemap[(scene->p_y * MAP_X) + scene->p_x];
  if (!current->def || current->def->collides) {
    scene->p_x = o_x;
    scene->p_y = o_y;
  }

  return NULL;
}

void scene_game_internal_build_map(SceneGame *scene) {
  assert(scene);

  TileDef *tiledefs = scene->tiledefs;

  RectifyMap *tiles = rectify_map_create();
  uint32_t tilemap[MAP_SIZE] = {};
  {
    size_t num_bytes = 0;
    uint8_t *data = NULL;
    archivist_read_file("testmap.json", &num_bytes, &data);

    cJSON *root = cJSON_Parse((const char *)data);
    cJSON *ids = cJSON_GetObjectItemCaseSensitive(root, "ids");
    cJSON *map = cJSON_GetObjectItemCaseSensitive(root, "map");
    for (int32_t t = 0; t < cJSON_GetArraySize(ids); t++) {
      cJSON *item = cJSON_GetArrayItem(ids, t);

      for (uint32_t u = 0; u < rectify_array_size(tiledefs); u++) {
        if (strncmp(tiledefs[u].id, item->valuestring, 128) == 0) {
          char num[32];
          snprintf(num, 32, "%d", u);
          rectify_map_set_uint(tiles, num, t);
          break;
        }
      }
    }

    uint32_t num_tiledefs = rectify_array_size(tiledefs);
    Tile *tilemap = scene->tilemap;
    for (uint32_t t = 0; t < MAP_SIZE; t++) {
      cJSON *item = cJSON_GetArrayItem(map, t);

      if (num_tiledefs) {
        char num[32];
        snprintf(num, 32, "%d", item->valueint);
        uint32_t id = rectify_map_get_uint(tiles, num);
        tilemap[t] = (Tile){
          .def = &tiledefs[id],
        };
      } else {
        tilemap[t] = (Tile){
          .def = NULL,
        };
      }
    }
    cJSON_Delete(root);

    free(data);
  }
  rectify_map_destroy(&tiles);
}
