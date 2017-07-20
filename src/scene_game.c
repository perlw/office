#include <assert.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"

#define MAP_X 20
#define MAP_Y 20
#define MAP_SIZE (MAP_X * MAP_Y)
typedef struct {
  double timing;
  double since_update;

  uint32_t p_x;
  uint32_t p_y;

  uint8_t map[MAP_SIZE];

  Surface *world;
} SceneGame;

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

  return scene;
}

void scene_game_destroy(SceneGame *const scene) {
  assert(scene);

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
