#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "config.h"

// +AsciiLayer
typedef struct {
  uint8_t rune;
  uint8_t fore;
  uint8_t back;
} Glyph;

typedef struct AsciiLayer AsciiLayer;

AsciiLayer *asciilayer_create(uint32_t width, uint32_t height, uint32_t ascii_width, uint32_t ascii_height);
void asciilayer_destroy(AsciiLayer *layer);
void asciilayer_draw(AsciiLayer *layer, bool dirty);
// -AsciiLayer

// +Screen
typedef struct Screen Screen;

Screen *screen_create(const Config *config);
void screen_destroy(Screen *screen);
void screen_draw(Screen *screen, bool dirty);
// -Screen

// +Surface
typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  uint32_t size;
  Glyph *asciimap;
} Surface;

Surface *surface_create(Screen *screen, uint32_t pos_x, uint32_t pos_y, uint32_t width, uint32_t height);
void surface_destroy(Surface *surface);
void surface_text(Surface *surface, uint32_t x, uint32_t y, uintmax_t length, const char *string);
// -Surface
