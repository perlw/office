#include <assert.h>

#include "bedrock/bedrock.h"

#include "tiles.h"

Surface *surface_create(uint32_t pos_x, uint32_t pos_y, uint32_t width, uint32_t height) {
  Surface *surface = calloc(1, sizeof(Surface));

  surface->x = pos_x;
  surface->y = pos_y;
  surface->width = width;
  surface->height = height;
  surface->size = surface->width * surface->height;
  surface->asciimap = calloc(surface->size, sizeof(Glyph));

  return surface;
}

void surface_destroy(Surface *surface) {
  assert(surface);

  free(surface->asciimap);

  free(surface);
}

void surface_text(Surface *surface, uint32_t x, uint32_t y, uint32_t length, const char *string, GlyphColor fore_color, GlyphColor back_color) {
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

    surface->asciimap[t].rune = string[u];
    surface->asciimap[t].fore = fore_color;
    surface->asciimap[t].back = back_color;
  }
}

void surface_rect(Surface *surface, uint32_t x, uint32_t y, uint32_t width, uint32_t height, SurfaceRectTiles rect_tiles, bool filled, GlyphColor fore_color, GlyphColor back_color) {
  assert(surface);

  if (x + width > surface->width || y + height > surface->height) {
    return;
  }

  uint32_t end_y = min(height, surface->height);
  uint32_t end_x = min(width, surface->width);
  for (uint32_t yy = y; yy < end_y; yy++) {
    for (uint32_t xx = x; xx < end_x; xx++) {
      uint32_t index = (yy * surface->width) + xx;

      if (yy == y) {
        if (xx == x) {
          surface->asciimap[index].rune = rect_tiles.tl;
        } else if (xx == end_x - 1) {
          surface->asciimap[index].rune = rect_tiles.tr;
        } else {
          surface->asciimap[index].rune = rect_tiles.t;
        }
      } else if (yy == end_y - 1) {
        if (xx == x) {
          surface->asciimap[index].rune = rect_tiles.bl;
        } else if (xx == end_x - 1) {
          surface->asciimap[index].rune = rect_tiles.br;
        } else {
          surface->asciimap[index].rune = rect_tiles.b;
        }
      } else {
        if (xx == x) {
          surface->asciimap[index].rune = rect_tiles.l;
        } else if (xx == end_x - 1) {
          surface->asciimap[index].rune = rect_tiles.r;
        } else {
          if (!filled) {
            continue;
          }

          surface->asciimap[index].rune = rect_tiles.c;
        }
      }

      surface->asciimap[index].fore = fore_color;
      surface->asciimap[index].back = back_color;
    }
  }
}

void surface_draw(Surface *surface, TilesAscii *tiles) {
  for (uint32_t y = 0; y < surface->height; y++) {
    for (uint32_t x = 0; x < surface->width; x++) {
      uint32_t s_index = (y * surface->width) + x;
      uint32_t index = ((y + surface->y) * tiles->ascii_width) + (x + surface->x);
      if (index >= tiles->ascii_size) {
        continue;
      }

      tiles->asciimap[index] = surface->asciimap[s_index];
    }
  }
}
// -Surface
