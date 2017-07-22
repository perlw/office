#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"

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
  double timing;
  double since_update;

  uint32_t p_x;
  uint32_t p_y;
  GossipHandle player_move_handle;

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

void scene_game_internal_movement_event(const char *groupd_id, const char *id, void *const subscriberdata, void *const userdata);

SceneGame *scene_game_create(void) {
  SceneGame *scene = calloc(1, sizeof(SceneGame));

  const Config *const config = config_get();

  scene->timing = 1.0 / 30.0;
  scene->since_update = 1.0 / (double)((rand() % 29) + 1);

  scene->p_x = 2;
  scene->p_y = 2;
  memcpy(scene->map, (uint8_t[]){
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

  scene->world = surface_create(0, 0, config->ascii_width, config->ascii_height);

  scene->player_move_handle = gossip_subscribe("player:*", &scene_game_internal_movement_event, scene);

  return scene;
}

void scene_game_destroy(SceneGame *const scene) {
  assert(scene);

  gossip_unsubscribe(scene->player_move_handle);
  surface_destroy(scene->world);

  free(scene);
}

void scene_game_update(SceneGame *const scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    surface_clear(scene->world, (Glyph){
                                  .rune = 0,
                                  .fore = 0,
                                  .back = 0,
                                });

    // +Draw world
    uint32_t offset_x = 20;
    uint32_t offset_y = 20;
    for (uint32_t y = 0; y < MAP_Y; y++) {
      uint32_t i = (y * MAP_Y);
      for (uint32_t x = 0; x < MAP_X; x++) {
        uint8_t rune = scene->map[i + x];
        GlyphColor color = (GlyphColor){ 255, 255, 255 };

        switch (rune) {
          case '.':
            color = (GlyphColor){ 102, 47, 0 };
            break;

          case 247:
            color = (GlyphColor){ 0, 71, 171 };
            break;
        }

        surface_glyph(scene->world, x + offset_x, y + offset_y, (Glyph){
                                                                  .rune = rune,
                                                                  .fore = color,
                                                                  .back = 0,
                                                                });
      }
    }
    // -Draw world

    // +Draw splashes
    for (uint8_t t = 0; t < 10; t++) {
      if (scene->splashes[t].keyframe > splash_num_frames - 1) {
        scene->splashes[t].alive = false;
      }
      if (!scene->splashes[t].alive) {
        continue;
      }

      surface_glyph(scene->world, scene->splashes[t].x + offset_x, scene->splashes[t].y + offset_y, (Glyph){
                                                                                                      .rune = splash_frames[scene->splashes[t].keyframe],
                                                                                                      .fore = splash_colors[scene->splashes[t].keyframe],
                                                                                                      .back = 0,
                                                                                                    });

      scene->splashes[t].next_frame -= scene->timing;
      if (scene->splashes[t].next_frame <= 0.0) {
        scene->splashes[t].next_frame = 1.0 / 20.0;
        scene->splashes[t].keyframe++;
      }
    }
    // -Draw splashes

    // +Draw player
    surface_glyph(scene->world, scene->p_x + offset_x, scene->p_y + offset_y, (Glyph){
                                                                                .rune = 1,
                                                                                .fore = (GlyphColor){ 255, 255, 255 },
                                                                                .back = 0,
                                                                              });
    // -Draw player
  }
}

void scene_game_draw(SceneGame *const scene, AsciiBuffer *const screen) {
  assert(scene);

  surface_draw(scene->world, screen);
}

void scene_game_internal_movement_event(const char *groupd_id, const char *id, void *const subscriberdata, void *const userdata) {
  SceneGame *scene = (SceneGame *)subscriberdata;

  uint32_t o_x = scene->p_x;
  uint32_t o_y = scene->p_y;
  if (strncmp("move_up_left", id, 128) == 0) {
    scene->p_x = (scene->p_x > 0 ? scene->p_x - 1 : scene->p_x);
    scene->p_y = (scene->p_y > 0 ? scene->p_y - 1 : scene->p_y);
  } else if (strncmp("move_up", id, 128) == 0) {
    scene->p_y = (scene->p_y > 0 ? scene->p_y - 1 : scene->p_y);
  } else if (strncmp("move_up_right", id, 128) == 0) {
    scene->p_x = (scene->p_x < MAP_X - 1 ? scene->p_x + 1 : scene->p_x);
    scene->p_y = (scene->p_y > 0 ? scene->p_y - 1 : scene->p_y);
  } else if (strncmp("move_left", id, 128) == 0) {
    scene->p_x = (scene->p_x > 0 ? scene->p_x - 1 : scene->p_x);
  } else if (strncmp("move_right", id, 128) == 0) {
    scene->p_x = (scene->p_x < MAP_X - 1 ? scene->p_x + 1 : scene->p_x);
  } else if (strncmp("move_down_left", id, 128) == 0) {
    scene->p_x = (scene->p_x > 0 ? scene->p_x - 1 : scene->p_x);
    scene->p_y = (scene->p_y < MAP_Y - 1 ? scene->p_y + 1 : scene->p_y);
  } else if (strncmp("move_down", id, 128) == 0) {
    scene->p_y = (scene->p_y < MAP_Y - 1 ? scene->p_y + 1 : scene->p_y);
  } else if (strncmp("move_down_right", id, 128) == 0) {
    scene->p_x = (scene->p_x < MAP_X - 1 ? scene->p_x + 1 : scene->p_x);
    scene->p_y = (scene->p_y < MAP_Y - 1 ? scene->p_y + 1 : scene->p_y);
  }
  uint8_t prev_rune = scene->map[(o_y * MAP_X) + o_x];
  uint8_t rune = scene->map[(scene->p_y * MAP_X) + scene->p_x];
  switch (rune) {
    case 179:
    case 191:
    case 192:
    case 196:
    case 217:
    case 218:
      scene->p_x = o_x;
      scene->p_y = o_y;
      break;

    case 247:
      //gossip_emit("sound:play_water_footsteps", 0, NULL);
      break;
  }

  switch (prev_rune) {
    case 247:
      for (uint8_t t = 0; t < 10; t++) {
        if (scene->splashes[t].alive) {
          continue;
        }

        scene->splashes[t].alive = true;
        scene->splashes[t].x = o_x;
        scene->splashes[t].y = o_y;
        scene->splashes[t].keyframe = 0;
        scene->splashes[t].next_frame = 1.0 / 20.0;

        break;
      }
      break;
  }
}
