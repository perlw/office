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

// TODO: Animated tiles
typedef struct {
  uint32_t num_glyphs;
  Glyph glyphs[4];
  bool collides;
} TileDef;

typedef struct {
  uint32_t current_glyph;
  double next_update;
  TileDef *def;
} Tile;

#define MAP_X 20
#define MAP_Y 20
#define MAP_SIZE (MAP_X * MAP_Y)
typedef struct {
  uint32_t p_x;
  uint32_t p_y;

  TileDef tiledefs[8];
  Tile map[MAP_SIZE];

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

  // +Temporary defs
  memcpy(scene_game_internal->tiledefs, (TileDef[]){
                                          // Walls
                                          {
                                            .num_glyphs = 1,
                                            .glyphs = { (Glyph){.rune = 218, .fore = glyphcolor(255, 255, 255), .back = 0 } },
                                            .collides = true,
                                          },
                                          {
                                            .num_glyphs = 1,
                                            .glyphs = { (Glyph){.rune = 196, .fore = glyphcolor(255, 255, 255), .back = 0 } },
                                            .collides = true,
                                          },
                                          {
                                            .num_glyphs = 1,
                                            .glyphs = { (Glyph){.rune = 191, .fore = glyphcolor(255, 255, 255), .back = 0 } },
                                            .collides = true,
                                          },
                                          {
                                            .num_glyphs = 1,
                                            .glyphs = { (Glyph){.rune = 179, .fore = glyphcolor(255, 255, 255), .back = 0 } },
                                            .collides = true,
                                          },
                                          {
                                            .num_glyphs = 1,
                                            .glyphs = { (Glyph){.rune = 192, .fore = glyphcolor(255, 255, 255), .back = 0 } },
                                            .collides = true,
                                          },
                                          {
                                            .num_glyphs = 1,
                                            .glyphs = { (Glyph){.rune = 217, .fore = glyphcolor(255, 255, 255), .back = 0 } },
                                            .collides = true,
                                          },
                                          // Ground/dirt
                                          {
                                            .num_glyphs = 1,
                                            .glyphs = { (Glyph){.rune = 46, .fore = glyphcolor(102, 47, 0), .back = 0 } },
                                            .collides = false,
                                          },
                                          // Water
                                          {
                                            .num_glyphs = 3,
                                            .glyphs = {
                                              (Glyph){
                                                .rune = 247,
                                                .fore = glyphcolor(0, 71, 171),
                                                .back = 0,
                                              },
                                              (Glyph){
                                                .rune = 126,
                                                .fore = glyphcolor(255, 255, 255),
                                                .back = 0,
                                              },
                                              (Glyph){
                                                .rune = 126,
                                                .fore = glyphcolor(0, 71, 171),
                                                .back = 0,
                                              },
                                            },
                                            .collides = false,
                                          },
                                        },
    sizeof(TileDef) * 8);
  // -Temporary defs

  // +Temporary map
  TileDef *tiledefs = scene_game_internal->tiledefs;
  Tile map[] = {
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[0] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[2] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[6] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[7] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[3] },
    {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[4] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[1] }, {.current_glyph = 0, .next_update = 0.0, .def = &tiledefs[5] },
  };
  // Temporary setup of water
  for (uint32_t t = 0; t < MAP_SIZE; t++) {
    if (map[t].def->num_glyphs > 1) {
      map[t].current_glyph = (rand() % map[t].def->num_glyphs);
      if (map[t].current_glyph == 0) {
        map[t].next_update = 1.0 + (double)(rand() % 25);
      } else {
        map[t].next_update = 1.0 / 2.0;
      }
    }
  }
  memcpy(scene_game_internal->map, map, sizeof(Tile) * MAP_SIZE);
  // -Temporary map

  scene_game_internal->world = surface_create(0, 0, config->ascii_width, config->ascii_height);

  screen_hook_render(&scene_game_internal_render_hook, NULL, 0);

  return true;
}

void scene_game_stop(void) {
  if (!scene_game_internal) {
    return;
  }

  screen_unhook_render(&scene_game_internal_render_hook, NULL);
  surface_destroy(&scene_game_internal->world);

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
      Tile *tile = &scene_game_internal->map[i + x];
      surface_glyph(scene_game_internal->world, x + offset_x, y + offset_y, tile->def->glyphs[tile->current_glyph]);
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

  // +Temporary animated tile
  for (uint32_t t = 0; t < MAP_SIZE; t++) {
    scene_game_internal->map[t].next_update -= 1.0 / 30.0;
    if (scene_game_internal->map[t].next_update <= 0.0) {
      scene_game_internal->map[t].next_update = 1.0 / 2.0;

      scene_game_internal->map[t].current_glyph++;
      if (scene_game_internal->map[t].current_glyph >= scene_game_internal->map[t].def->num_glyphs - 1) {
        scene_game_internal->map[t].current_glyph = 0;
        scene_game_internal->map[t].next_update = 1.0 + (double)(rand() % 25);
      }
    }
  }
  // -Temporary animated tile
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

  Tile *current = &scene_game_internal->map[(scene_game_internal->p_y * MAP_X) + scene_game_internal->p_x];
  if (current->def->collides) {
    scene_game_internal->p_x = o_x;
    scene_game_internal->p_y = o_y;
  }
}

void scene_game_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  if (!scene_game_internal) {
    return;
  }
  surface_draw(scene_game_internal->world, screen);
}
