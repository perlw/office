#include <assert.h>

#include "bedrock/bedrock.h"

#include "ascii.h"

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

void surface_destroy(Surface *const surface) {
  assert(surface);

  free(surface->buffer);

  free(surface);
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

void surface_rect(Surface *const surface, uint32_t x, uint32_t y, uint32_t width, uint32_t height, SurfaceRectTiles rect_tiles, bool filled, GlyphColor fore_color, GlyphColor back_color) {
  assert(surface);

  if (x + width > surface->width || y + height > surface->height) {
    return;
  }

  uint32_t end_y = fmin(height, surface->height);
  uint32_t end_x = fmin(width, surface->width);
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

void surface_draw(Surface *const surface, AsciiBuffer *const tiles) {
  assert(surface);

  for (uint32_t y = 0; y < surface->height; y++) {
    for (uint32_t x = 0; x < surface->width; x++) {
      uint32_t s_index = (y * surface->width) + x;
      uint32_t index = ((y + surface->y) * tiles->width) + (x + surface->x);
      if (index >= tiles->size || surface->buffer[s_index].rune == 0) {
        continue;
      }

      tiles->buffer[index] = surface->buffer[s_index];
    }
  }
}
