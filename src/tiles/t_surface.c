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

  for (uintmax_t y = 0; y < surface->height; y++) {
    for (uintmax_t x = 0; x < surface->width; x++) {
      uintmax_t index = (y * surface->width) + x;
      uint8_t shade = (uint8_t)((x ^ y) + 32);
      surface->asciimap[index].rune = 219;
      surface->asciimap[index].fore.r = shade;
      surface->asciimap[index].fore.g = shade;
      surface->asciimap[index].fore.b = shade;
    }
  }

  return surface;
}

void surface_destroy(Surface *surface) {
  assert(surface);

  free(surface->asciimap);

  free(surface);
}

void surface_text(Surface *surface, uint32_t x, uint32_t y, uintmax_t length, const char *string) {
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
    surface->asciimap[t].fore.r = 255;
    surface->asciimap[t].fore.g = 255;
    surface->asciimap[t].fore.b = 255;
    surface->asciimap[t].back.r = 128;
    surface->asciimap[t].back.g = 0;
    surface->asciimap[t].back.b = 0;
  }
}

void surface_draw(Surface *surface, TilesAscii *tiles) {
  for (uintmax_t y = 0; y < surface->height; y++) {
    for (uintmax_t x = 0; x < surface->width; x++) {
      uintmax_t s_index = (y * surface->width) + x;
      uintmax_t index = ((y + surface->y) * tiles->ascii_width) + (x + surface->x);
      if (index >= tiles->ascii_size) {
        continue;
      }

      tiles->asciimap[index] = surface->asciimap[s_index];
    }
  }
}
// -Surface
