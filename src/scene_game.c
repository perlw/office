#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "screen.h"

bool scene_game_start(void);
void scene_game_stop(void);
void scene_game_update(double delta);
void scene_game_message(uint32_t id, RectifyMap *const map);

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

typedef struct {
  char *id;
  Glyph glyph;
  bool collides;
} TileDef;

typedef struct {
  TileDef *def;
} Tile;

typedef struct {
  char *id;
} BaseTilemap;
const BaseTilemap base_tilemap[MAP_SIZE] = {
  "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "water", "water", "water", "water", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "water", "water", "water", "water", "water", "water", "rock",
  "rock", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "rock",
  "rock", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "water", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "water", "water", "water", "water", "water", "water", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "water", "water", "water", "water", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "dirt", "rock",
  "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock", "rock",
};

typedef struct {
  uint32_t p_x;
  uint32_t p_y;

  TileDef *tiledefs;
  Tile *tilemap;

  Surface *world;
} SceneGame;

void scene_game_internal_build_map(void);
void scene_game_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

SceneGame *scene_game_internal = NULL;
bool scene_game_start(void) {
  if (scene_game_internal) {
    return false;
  }

  scene_game_internal = calloc(1, sizeof(SceneGame));

  Config *const config = config_get();

  scene_game_internal->p_x = 2;
  scene_game_internal->p_y = 2;

  scene_game_internal->tiledefs = rectify_array_alloc(10, sizeof(TileDef));
  scene_game_internal->tilemap = calloc(1, sizeof(Tile) * MAP_SIZE);
  scene_game_internal_build_map();

  scene_game_internal->world = surface_create(0, 0, config->ascii_width, config->ascii_height);

  screen_hook_render(&scene_game_internal_render_hook, NULL, 0);

  gossip_post("lua_bridge", MSG_MATERIALS_LOAD, NULL);

  return true;
}

void scene_game_stop(void) {
  if (!scene_game_internal) {
    return;
  }

  free(scene_game_internal->tilemap);
  for (uint32_t t = 0; t < rectify_array_size(scene_game_internal->tiledefs); t++) {
    free(scene_game_internal->tiledefs[t].id);
  }
  rectify_array_free((void **)&scene_game_internal->tiledefs);

  screen_unhook_render(&scene_game_internal_render_hook, NULL);
  surface_destroy(&scene_game_internal->world);

  free(scene_game_internal);
  scene_game_internal = NULL;
}

void scene_game_update(double delta) {
  if (!scene_game_internal) {
    return;
  }

  surface_clear(scene_game_internal->world, (Glyph){
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
      Tile *tile = &scene_game_internal->tilemap[i + x];

      if (tile->def) {
        surface_glyph(scene_game_internal->world, x + offset_x, y + offset_y, tile->def->glyph);
      } else {
        surface_glyph(scene_game_internal->world, x + offset_x, y + offset_y, (Glyph){
                                                                                .rune = '?',
                                                                                .fore = glyphcolor_hex(0xffffff),
                                                                                .back = glyphcolor_hex(0xff0000),
                                                                              });
      }
    }
  }
  // -Draw world

  // +Draw player
  surface_glyph(scene_game_internal->world, scene_game_internal->p_x + offset_x, scene_game_internal->p_y + offset_y, (Glyph){
                                                                                                                        .rune = 1,
                                                                                                                        .fore = (GlyphColor){ 255, 255, 255 },
                                                                                                                        .back = 0,
                                                                                                                      });
  // -Draw player
}

void scene_game_message(uint32_t id, RectifyMap *const map) {
  if (!scene_game_internal) {
    return;
  }

  uint32_t o_x = scene_game_internal->p_x;
  uint32_t o_y = scene_game_internal->p_y;
  switch (id) {
    case MSG_PLAYER_MOVE_UP_LEFT: {
      scene_game_internal->p_x = (scene_game_internal->p_x > 0 ? scene_game_internal->p_x - 1 : scene_game_internal->p_x);
      scene_game_internal->p_y = (scene_game_internal->p_y > 0 ? scene_game_internal->p_y - 1 : scene_game_internal->p_y);
      break;
    }

    case MSG_PLAYER_MOVE_UP: {
      scene_game_internal->p_y = (scene_game_internal->p_y > 0 ? scene_game_internal->p_y - 1 : scene_game_internal->p_y);
      break;
    }

    case MSG_PLAYER_MOVE_UP_RIGHT: {
      scene_game_internal->p_x = (scene_game_internal->p_x < MAP_X - 1 ? scene_game_internal->p_x + 1 : scene_game_internal->p_x);
      scene_game_internal->p_y = (scene_game_internal->p_y > 0 ? scene_game_internal->p_y - 1 : scene_game_internal->p_y);
      break;
    }

    case MSG_PLAYER_MOVE_LEFT: {
      scene_game_internal->p_x = (scene_game_internal->p_x > 0 ? scene_game_internal->p_x - 1 : scene_game_internal->p_x);
      break;
    }

    case MSG_PLAYER_MOVE_RIGHT: {
      scene_game_internal->p_x = (scene_game_internal->p_x < MAP_X - 1 ? scene_game_internal->p_x + 1 : scene_game_internal->p_x);
      break;
    }

    case MSG_PLAYER_MOVE_DOWN_LEFT: {
      scene_game_internal->p_x = (scene_game_internal->p_x > 0 ? scene_game_internal->p_x - 1 : scene_game_internal->p_x);
      scene_game_internal->p_y = (scene_game_internal->p_y < MAP_Y - 1 ? scene_game_internal->p_y + 1 : scene_game_internal->p_y);
      break;
    }

    case MSG_PLAYER_MOVE_DOWN: {
      scene_game_internal->p_y = (scene_game_internal->p_y < MAP_Y - 1 ? scene_game_internal->p_y + 1 : scene_game_internal->p_y);
      break;
    }

    case MSG_PLAYER_MOVE_DOWN_RIGHT: {
      scene_game_internal->p_x = (scene_game_internal->p_x < MAP_X - 1 ? scene_game_internal->p_x + 1 : scene_game_internal->p_x);
      scene_game_internal->p_y = (scene_game_internal->p_y < MAP_Y - 1 ? scene_game_internal->p_y + 1 : scene_game_internal->p_y);
      break;
    }

    case MSG_MATERIAL_REGISTER: {
      char *const id = rectify_map_get_string(map, "id");
      uint8_t rune = rectify_map_get_byte(map, "rune");
      GlyphColor fore = glyphcolor_hex(rectify_map_get_uint(map, "fore_color"));
      GlyphColor back = glyphcolor_hex(rectify_map_get_uint(map, "back_color"));
      RectifyMap *tags = rectify_map_get_map(map, "tags");

      TileDef def = {
        .id = rectify_memory_alloc_copy(id, sizeof(char) * (strnlen(id, 128) + 1)),
        .glyph = (Glyph){
          .rune = rune,
          .fore = fore,
          .back = back,
        },
        .collides = false,
      };

      printf("%s => { %d|%c, (%d %d %d) (%d %d %d)}\n", id, rune, rune, fore.r, fore.g, fore.b, back.r, back.g, back.b);
      RectifyMapIter iter = rectify_map_iter(tags);
      for (RectifyMapItem item; rectify_map_iter_next(&iter, &item);) {
        switch (item.type) {
          case RECTIFY_MAP_TYPE_STRING: {
            char *const tag = (char *const)item.val;
            if (strncmp(tag, "wall", 128) == 0) {
              def.collides = true;
            }
          }
        }
      }

      scene_game_internal->tiledefs = rectify_array_push(scene_game_internal->tiledefs, &def);
      break;
    }

    case MSG_MATERIALS_LOADED:
      scene_game_internal_build_map();
      break;
  }

  Tile *current = &scene_game_internal->tilemap[(scene_game_internal->p_y * MAP_X) + scene_game_internal->p_x];
  if (!current->def || current->def->collides) {
    scene_game_internal->p_x = o_x;
    scene_game_internal->p_y = o_y;
  }
}

void scene_game_internal_build_map(void) {
  TileDef *tiledefs = scene_game_internal->tiledefs;
  Tile *tilemap = scene_game_internal->tilemap;
  for (uint32_t t = 0; t < MAP_SIZE; t++) {
    tilemap[t] = (Tile){
      .def = NULL,
    };

    for (uint32_t d = 0; d < rectify_array_size(tiledefs); d++) {
      if (strncmp(tiledefs[d].id, base_tilemap[t].id, 128) == 0) {
        tilemap[t].def = &tiledefs[d];
        break;
      }
    }
  }
}

void scene_game_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  if (!scene_game_internal) {
    return;
  }
  surface_draw(scene_game_internal->world, screen);
}
