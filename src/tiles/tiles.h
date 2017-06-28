#pragma once

#include <stdbool.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

// +TileSet
typedef struct {
  bool auto_tile;
} TileDef;

typedef struct {
  TileDef tile_defs[256];
  PicassoTexture *texture;
} TileSet;

TileSet *tileset_create(void);
void tileset_destroy(TileSet *const tileset);

void tileset_load_defs(TileSet *const tileset, const char *filepath);
// -TileSet

// +Tiles
typedef struct {
  uint32_t num_tiles_x;
  uint32_t num_tiles_y;
  uint32_t num_tiles;
  uint8_t *tilemap;
  uint8_t *last_tilemap;

  TileSet *tileset;

  PicassoBufferGroup *quad;
  PicassoProgram *program;
  PicassoTexture *tilemap_texture;

  struct {
    int32_t pmatrix_uniform;
    mat4_t projection_matrix;
    int32_t num_tiles_x_uniform;
    int32_t num_tiles_y_uniform;
  } shader;
} Tiles;

Tiles *tiles_create(uint32_t width, uint32_t height, uint32_t num_tiles_x, uint32_t num_tiles_y);
void tiles_destroy(Tiles *tiles);

void tiles_draw(Tiles *tiles);
// -Tiles

// +TilesAscii
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} GlyphColor;

typedef struct {
  uint8_t rune;
  GlyphColor fore;
  GlyphColor back;
} Glyph;

GlyphColor glyphcolor_add(GlyphColor a, GlyphColor b);
GlyphColor glyphcolor_sub(GlyphColor a, GlyphColor b);
GlyphColor glyphcolor_adds(GlyphColor c, float s);
GlyphColor glyphcolor_subs(GlyphColor c, float s);
GlyphColor glyphcolor_muls(GlyphColor c, float s);
GlyphColor glyphcolor_divs(GlyphColor c, float s);

typedef struct {
  PicassoBufferGroup *quad;
  PicassoProgram *program;
  PicassoTexture *font_texture;

  uint32_t ascii_width;
  uint32_t ascii_height;
  uint32_t ascii_size;
  Glyph *asciimap;
  Glyph *last_asciimap;

  PicassoTexture *asciimap_texture;
  PicassoTexture *forecolors_texture;
  PicassoTexture *backcolors_texture;

  struct {
    int32_t pmatrix_uniform;
    mat4_t projection_matrix;
    int32_t ascii_width_uniform;
    int32_t ascii_height_uniform;
  } shader;
} TilesAscii;

TilesAscii *tiles_ascii_create(uint32_t width, uint32_t height, uint32_t ascii_width, uint32_t ascii_height);
void tiles_ascii_destroy(TilesAscii *layer);

void tiles_ascii_draw(TilesAscii *layer);
// -TilesAscii

// +Surface
typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  uint32_t size;
  Glyph *asciimap;
} Surface;

typedef struct {
  uint8_t tl, t, tr;
  uint8_t l, c, r;
  uint8_t bl, b, br;
} SurfaceRectTiles;

Surface *surface_create(uint32_t pos_x, uint32_t pos_y, uint32_t width, uint32_t height);
void surface_destroy(Surface *surface);

void surface_text(Surface *surface, uint32_t x, uint32_t y, uint32_t length, const char *string, GlyphColor fore_color, GlyphColor back_color);
void surface_rect(Surface *surface, uint32_t x, uint32_t y, uint32_t width, uint32_t height, SurfaceRectTiles rect_tiles, bool filled, GlyphColor fore_color, GlyphColor back_color);

void surface_draw(Surface *surface, TilesAscii *tiles);
// -Surface
