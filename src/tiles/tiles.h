#pragma once

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
