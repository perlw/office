#include <assert.h>
#include <string.h>

#define USE_RECTIFY
#include "bedrock/bedrock.h"

#define USE_ASCII
#include "main.h"

Surface *surface_create(uint32_t pos_x, uint32_t pos_y, uint32_t width, uint32_t height) {
  Surface *surface = calloc(1, sizeof(Surface));

  surface->x = pos_x;
  surface->y = pos_y;
  surface->width = width;
  surface->height = height;
  surface->size = surface->width * surface->height;
  surface->buffer = calloc(surface->size, sizeof(Glyph));

  return surface;
}

Surface *surface_clone(Surface *const original) {
  assert(original);

  Surface *surface = calloc(1, sizeof(Surface));

  *surface = (Surface){
    .x = original->x,
    .y = original->y,
    .width = original->width,
    .height = original->height,
    .size = original->size,
    .buffer = rectify_memory_alloc_copy(original->buffer, sizeof(Glyph) * original->size),
  };

  return surface;
}

void surface_destroy(Surface **surface) {
  Surface *ptr = *surface;

  assert(surface && ptr);
  free(ptr->buffer);
  free(ptr);
  *surface = NULL;
}

void surface_clear(Surface *const surface, Glyph glyph) {
  assert(surface);

  for (uint32_t t = 0; t < surface->size; t++) {
    surface->buffer[t] = glyph;
  }
}

void surface_text(Surface *const surface, uint32_t x, uint32_t y, uint32_t length, const char *string, GlyphColor fore_color, GlyphColor back_color) {
  assert(surface);

  if (x >= surface->width || y >= surface->height) {
    return;
  }

  if (length == 0 && string && string[0] != 0) {
    length = (uint32_t)strnlen(string, 256) + 1;
  }

  uint32_t index = (y * surface->width) + x;
  uint32_t max = index + (x + length >= surface->width ? surface->width - x : x + length);
  for (uint32_t t = index, u = 0; t < max; t++, u++) {
    if (string[u] == '\0') {
      break;
    }

    surface->buffer[t].rune = string[u];
    surface->buffer[t].fore = fore_color;
    surface->buffer[t].back = back_color;
  }
}

void surface_textc(Surface *const surface, uint32_t x, uint32_t y, uint32_t length, const char *string) {
  assert(surface);

  if (x >= surface->width || y >= surface->height) {
    return;
  }

  if (length == 0 && string && string[0] != 0) {
    length = (uint32_t)strnlen(string, 256) + 1;
  }

  uint32_t index = (y * surface->width) + x;
  uint32_t max = index + (x + length >= surface->width ? surface->width - x : x + length);
  char color_buffer[7] = { 0, 0, 0, 0, 0, 0, 0 };
  uint32_t fore = 0xffffff;
  uint32_t back = 0;
  for (uint32_t t = index, u = 0; t < max; t++, u++) {
    if (string[u] == '\0') {
      break;
    }

    if (string[u] == '#' && u + 1 < length && string[u + 1] == '{') {
      if (u + 15 < length && string[u + 15] == '}') {
        for (uint32_t i = u + 2; i < u + 8; i++) {
          color_buffer[i - (u + 2)] = string[i];
        }
        fore = strtol(color_buffer, NULL, 16);
        for (uint32_t i = u + 9; i < u + 15; i++) {
          color_buffer[i - (u + 9)] = string[i];
        }
        back = strtol(color_buffer, NULL, 16);
        u += 16;
      } else if (u + 8 < length && string[u + 8] == '}') {
        for (uint32_t i = u + 2; i < u + 8; i++) {
          color_buffer[i - (u + 2)] = string[i];
        }
        fore = strtol(color_buffer, NULL, 16);
        u += 9;
      }
    }

    surface->buffer[t].rune = string[u];
    surface->buffer[t].fore = glyphcolor_hex(fore);
    surface->buffer[t].back = glyphcolor_hex(back);
  }
}

void surface_rect(Surface *const surface, uint32_t x, uint32_t y, uint32_t width, uint32_t height, SurfaceRectTiles rect_tiles, bool filled, GlyphColor fore_color, GlyphColor back_color) {
  assert(surface);

  if (x + width > surface->width || y + height > surface->height) {
    return;
  }

  uint32_t end_y = (uint32_t)fmin(y + height, surface->height);
  uint32_t end_x = (uint32_t)fmin(x + width, surface->width);
  for (uint32_t yy = y; yy < end_y; yy++) {
    for (uint32_t xx = x; xx < end_x; xx++) {
      uint32_t index = (yy * surface->width) + xx;

      if (yy == y) {
        if (xx == x) {
          surface->buffer[index].rune = rect_tiles.tl;
        } else if (xx == end_x - 1) {
          surface->buffer[index].rune = rect_tiles.tr;
        } else {
          surface->buffer[index].rune = rect_tiles.t;
        }
      } else if (yy == end_y - 1) {
        if (xx == x) {
          surface->buffer[index].rune = rect_tiles.bl;
        } else if (xx == end_x - 1) {
          surface->buffer[index].rune = rect_tiles.br;
        } else {
          surface->buffer[index].rune = rect_tiles.b;
        }
      } else {
        if (xx == x) {
          surface->buffer[index].rune = rect_tiles.l;
        } else if (xx == end_x - 1) {
          surface->buffer[index].rune = rect_tiles.r;
        } else {
          if (!filled) {
            continue;
          }

          surface->buffer[index].rune = rect_tiles.c;
        }
      }

      surface->buffer[index].fore = fore_color;
      surface->buffer[index].back = back_color;
    }
  }
}

void surface_line(Surface *const surface, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, Glyph glyph) {
  assert(surface);

  int32_t ax = (uint32_t)x1;
  int32_t ay = (uint32_t)y1;
  int32_t bx = (uint32_t)x2;
  int32_t by = (uint32_t)y2;

  int32_t dx = abs(bx - ax);
  int32_t dy = abs(by - ay);
  int32_t sx = (ax < bx ? 1 : -1);
  int32_t sy = (ay < by ? 1 : -1);
  int32_t err = (dx > dy ? dx : -dy) / 2;
  int32_t e2 = 0;

  int32_t x = ax;
  int32_t y = ay;
  for (;;) {
    surface_glyph(surface, x, y, glyph);
    if (x == bx && y == by) {
      break;
    }
    e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x += sx;
    }
    if (e2 < dy) {
      err += dx;
      y += sy;
    }
  }
}

float lerp(float a, float b, float t) {
  return ((1.0f - t) * a) + (t * b);
}

void surface_graph(Surface *const surface, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t num_values, const float *values) {
  assert(surface);
  assert(values);

  if (x >= surface->width || y >= surface->height || x + width > surface->width || y + height > surface->height) {
    return;
  }

  uint32_t width_per_step = width / num_values;
  for (uint32_t t = 0; t < num_values; t++) {
    uint32_t val_height = (uint32_t)((float)(height - 1) * (1.0 - values[t]));
    uint32_t next_val_height = (t < num_values - 1 ? (uint32_t)((float)(height - 1) * (1.0 - values[t + 1])) : val_height);

    for (uint32_t yy = 0; yy < height; yy++) {
      for (uint32_t xx = 0; xx < width_per_step; xx++) {
        uint32_t fudged_height = (uint32_t)(lerp((float)val_height, (float)next_val_height, (float)xx / (float)width_per_step) + 0.5f);

        uint32_t index = ((y + yy) * surface->width) + (x + (t * width_per_step) + xx);
        if (yy >= fudged_height) {
          if (yy == fudged_height) {
            surface->buffer[index].rune = '+';
            surface->buffer[index].fore = (GlyphColor){ 255, 255, 255 };
          } else if (yy > fudged_height) {
            surface->buffer[index].rune = '.';
            surface->buffer[index].fore = (GlyphColor){ 128, 128, 128 };
          }
          surface->buffer[index].back = (GlyphColor){ 66, 66, 66 };
        } else {
          surface->buffer[index].rune = 0;
          surface->buffer[index].fore = (GlyphColor){ 0, 0, 0 };
          surface->buffer[index].back = (GlyphColor){ 255, 0, 255 };
        }
      }
    }
  }
}

void surface_glyph(Surface *const surface, uint32_t x, uint32_t y, Glyph glyph) {
  assert(surface);

  if (x > surface->width - 1 || y > surface->height - 1) {
    return;
  }

  uint32_t index = (y * surface->width) + x;
  surface->buffer[index] = glyph;
}

void surface_draw(Surface *const surface, AsciiBuffer *const ascii) {
  assert(surface);

  uint32_t max_width = (surface->x + surface->width <= ascii->width ? surface->width : ascii->width);
  uint32_t max_height = (surface->y + surface->height <= ascii->height ? surface->height : ascii->height);
  for (uint32_t y = 0; y < max_height; y++) {
    for (uint32_t x = 0; x < max_width; x++) {
      uint32_t index = (y * surface->width) + x;
      if (surface->buffer[index].rune == 0) {
        continue;
      }

      ascii_buffer_glyph(ascii, surface->x + x, surface->y + y, surface->buffer[index]);
    }
  }
}
