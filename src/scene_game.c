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
void scene_game_update(void);
void scene_game_message(uint32_t id, RectifyMap *const map);

KronosSystem scene_game = {
  .name = "scene_game",
  .frames = 30,
  .start = &scene_game_start,
  .stop = &scene_game_stop,
  .update = &scene_game_update,
  .message = &scene_game_message,
};

const uint8_t splash_num_frames = 4;
uint8_t splash_frames[4] = { '*', '+', '.', 'o' };
GlyphColor *splash_colors = (GlyphColor[4]){
  { 128, 163, 213 },
  { 0, 71, 171 },
  { 0, 53, 128 },
  { 0, 71, 171 },
};

#define MAP_X 20
#define MAP_Y 20
#define MAP_SIZE (MAP_X * MAP_Y)
typedef struct {
  uint32_t p_x;
  uint32_t p_y;

  uint8_t map[MAP_SIZE];

  struct {
    bool alive;
    uint32_t x;
    uint32_t y;
    uint8_t keyframe;
    double next_frame;
  } splashes[10];

  Surface *world;
} SceneGame;

void scene_game_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

SceneGame *scene_game_internal = NULL;
bool scene_game_start(void) {
  if (scene_game_internal) {
    return false;
  }

  scene_game_internal = calloc(1, sizeof(SceneGame));

  const Config *const config = config_get();

  scene_game_internal->p_x = 2;
  scene_game_internal->p_y = 2;
  memcpy(scene_game_internal->map, (uint8_t[]){
                                     218, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 191,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 247, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 247, 46, 247, 247, 179,
                                     179, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 179,
                                     179, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 247, 247, 247, 247, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 247, 247, 247, 247, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 247, 247, 247, 247, 247, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 247, 247, 247, 247, 247, 179,
                                     179, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 247, 247, 247, 247, 179,
                                     192, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 217,
                                   },
    sizeof(uint8_t) * MAP_SIZE);

  scene_game_internal->world = surface_create(0, 0, config->ascii_width, config->ascii_height);

  screen_hook_render(&scene_game_internal_render_hook, NULL, 0);

  return true;
}

void scene_game_stop(void) {
  if (!scene_game_internal) {
    return;
  }

  screen_unhook_render(&scene_game_internal_render_hook, NULL);
  surface_destroy(scene_game_internal->world);

  free(scene_game_internal);
  scene_game_internal = NULL;
}

void scene_game_update(void) {
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
      uint8_t rune = scene_game_internal->map[i + x];
      GlyphColor color = (GlyphColor){ 255, 255, 255 };

      switch (rune) {
        case '.':
          color = (GlyphColor){ 102, 47, 0 };
          break;

        case 247:
          color = (GlyphColor){ 0, 71, 171 };
          break;
      }

      surface_glyph(scene_game_internal->world, x + offset_x, y + offset_y, (Glyph){
                                                                              .rune = rune,
                                                                              .fore = color,
                                                                              .back = 0,
                                                                            });
    }
  }
  // -Draw world

  // +Draw splashes
  for (uint8_t t = 0; t < 10; t++) {
    if (scene_game_internal->splashes[t].keyframe > splash_num_frames - 1) {
      scene_game_internal->splashes[t].alive = false;
    }
    if (!scene_game_internal->splashes[t].alive) {
      continue;
    }

    surface_glyph(scene_game_internal->world, scene_game_internal->splashes[t].x + offset_x, scene_game_internal->splashes[t].y + offset_y, (Glyph){
                                                                                                                                              .rune = splash_frames[scene_game_internal->splashes[t].keyframe],
                                                                                                                                              .fore = splash_colors[scene_game_internal->splashes[t].keyframe],
                                                                                                                                              .back = 0,
                                                                                                                                            });

    scene_game_internal->splashes[t].next_frame -= 1.0 / 30.0;
    if (scene_game_internal->splashes[t].next_frame <= 0.0) {
      scene_game_internal->splashes[t].next_frame = 1.0 / 20.0;
      scene_game_internal->splashes[t].keyframe++;
    }
  }
  // -Draw splashes

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
  }

  uint8_t prev_rune = scene_game_internal->map[(o_y * MAP_X) + o_x];
  uint8_t rune = scene_game_internal->map[(scene_game_internal->p_y * MAP_X) + scene_game_internal->p_x];
  switch (rune) {
    case 179:
    case 191:
    case 192:
    case 196:
    case 217:
    case 218:
      scene_game_internal->p_x = o_x;
      scene_game_internal->p_y = o_y;
      break;

    case 247:
      //gossip_emit("sound:play_water_footsteps", 0, NULL);
      break;
  }

  switch (prev_rune) {
    case 247:
      for (uint8_t t = 0; t < 10; t++) {
        if (scene_game_internal->splashes[t].alive) {
          continue;
        }

        scene_game_internal->splashes[t].alive = true;
        scene_game_internal->splashes[t].x = o_x;
        scene_game_internal->splashes[t].y = o_y;
        scene_game_internal->splashes[t].keyframe = 0;
        scene_game_internal->splashes[t].next_frame = 1.0 / 20.0;

        break;
      }
      break;
  }
}

void scene_game_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  if (!scene_game_internal) {
    return;
  }
  surface_draw(scene_game_internal->world, screen);
}
