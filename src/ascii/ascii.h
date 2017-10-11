#pragma once

#include <stdbool.h>

#include "arkanis/math_3d.h"

#define USE_PICASSO
#include "bedrock/bedrock.h"

// +AsciiBuffer
typedef union {
  struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };
} GlyphColor;

typedef struct {
  uint8_t rune;
  GlyphColor fore;
  GlyphColor back;
} Glyph;

GlyphColor glyphcolor(uint8_t r, uint8_t g, uint8_t b);
GlyphColor glyphcolor_hex(uint32_t hex);
GlyphColor glyphcolor_add(GlyphColor a, GlyphColor b);
GlyphColor glyphcolor_sub(GlyphColor a, GlyphColor b);
GlyphColor glyphcolor_adds(GlyphColor c, float s);
GlyphColor glyphcolor_subs(GlyphColor c, float s);
GlyphColor glyphcolor_muls(GlyphColor c, float s);
GlyphColor glyphcolor_divs(GlyphColor c, float s);
bool glyphcolor_eq(GlyphColor a, GlyphColor b);

typedef struct {
  uint8_t *rune_buffer;
  GlyphColor *fore_buffer;
  GlyphColor *back_buffer;
} DisplayBuffer;

typedef struct {
  PicassoWindow *window;

  PicassoBufferGroup *quad;
  PicassoProgram *program;
  PicassoTexture *font_texture;

  uint32_t width;
  uint32_t height;
  uint32_t size;

  bool dirty;
  DisplayBuffer buffers[2];
  DisplayBuffer *front_buffer;
  DisplayBuffer *back_buffer;

  PicassoTexture *asciimap_texture;
  PicassoTexture *forecolors_texture;
  PicassoTexture *backcolors_texture;

  struct {
    PicassoFramebuffer *framebuffer;
    PicassoTexture *texture;
    PicassoProgram *program;
    PicassoBufferGroup *quad;
  } fbo;
} AsciiBuffer;

AsciiBuffer *ascii_buffer_create(PicassoWindow *window, uint32_t width, uint32_t height, uint32_t ascii_width, uint32_t ascii_height);
void ascii_buffer_destroy(AsciiBuffer **ascii);

void ascii_buffer_glyph(AsciiBuffer *const ascii, uint32_t x, uint32_t y, Glyph glyph);
void ascii_buffer_draw(AsciiBuffer *const ascii);
// -AsciiBuffer

// +Surface
typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  uint32_t size;
  Glyph *buffer;
} Surface;

typedef struct {
  uint8_t tl, t, tr;
  uint8_t l, c, r;
  uint8_t bl, b, br;
} SurfaceRectTiles;

Surface *surface_create(uint32_t pos_x, uint32_t pos_y, uint32_t width, uint32_t height);
Surface *surface_clone(Surface *const original);
void surface_destroy(Surface **surface);

void surface_clear(Surface *const surface, Glyph glyph);
void surface_text(Surface *const surface, uint32_t x, uint32_t y, uint32_t length, const char *string, GlyphColor fore_color, GlyphColor back_color);
void surface_textc(Surface *const surface, uint32_t x, uint32_t y, uint32_t length, const char *string);
void surface_rect(Surface *const surface, uint32_t x, uint32_t y, uint32_t width, uint32_t height, SurfaceRectTiles rect_tiles, bool filled, GlyphColor fore_color, GlyphColor back_color);
void surface_line(Surface *const surface, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, Glyph glyph);
void surface_graph(Surface *const surface, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t num_values, const float *values);
void surface_glyph(Surface *const surface, uint32_t x, uint32_t y, Glyph glyph);

void surface_draw(Surface *const surface, AsciiBuffer *const tiles);
// -Surface
