#pragma once

// +Tiles
typedef struct {
  uint32_t num_tiles_x;
  uint32_t num_tiles_y;
  uint32_t num_tiles;
  uint8_t *tilemap;
  uint8_t *last_tilemap;

  PicassoBufferGroup *quad;
  PicassoProgram *program;
  PicassoTexture *tileset_texture;
  PicassoTexture *tilemap_texture;
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
} TilesAscii;

TilesAscii *tiles_ascii_create(uint32_t width, uint32_t height, uint32_t ascii_width, uint32_t ascii_height);
void tiles_ascii_destroy(TilesAscii *layer);

void tiles_ascii_draw(TilesAscii *layer);
// -TilesAscii
