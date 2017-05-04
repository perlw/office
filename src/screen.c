#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "assets.h"
#include "config.h"
#include "tiles/tiles.h"

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
  TilesAscii *asciilayer;
  TilesAscii *asciilayer2;
  Surface **surfaces;
} Screen;

Surface *surface_create(Screen *screen, uint32_t pos_x, uint32_t pos_y, uint32_t width, uint32_t height) {
  Surface *surface = calloc(1, sizeof(Surface));

  // TODO: Rethink, screen should control lifecycle?
  assert(screen);
  screen->surfaces = rectify_array_push(screen->surfaces, &surface);

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
// -Surface

// +Screen
Screen *screen_create(const Config *config) {
  Screen *screen = calloc(1, sizeof(Screen));

  screen->asciilayer = tiles_ascii_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);
  screen->asciilayer2 = tiles_ascii_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);
  screen->surfaces = rectify_array_alloc(10, sizeof(Surface *));

  for (uintmax_t t = 0; t < screen->asciilayer2->ascii_size; t++) {
    if (t % 16) {
      screen->asciilayer2->asciimap[t].rune = 0;
      screen->asciilayer2->asciimap[t].fore = (GlyphColor){ 0, 0, 0 };
      screen->asciilayer2->asciimap[t].back = (GlyphColor){ 255, 0, 255 };
    } else {
      screen->asciilayer2->asciimap[t].rune = 2;
      screen->asciilayer2->asciimap[t].fore = (GlyphColor){ 128, 0, 0 };
      screen->asciilayer2->asciimap[t].back = (GlyphColor){ 192, 255, 0 };
    }
  }

  return screen;
}

void screen_destroy(Screen *screen) {
  assert(screen);

  rectify_array_free(screen->surfaces);
  tiles_ascii_destroy(screen->asciilayer2);
  tiles_ascii_destroy(screen->asciilayer);

  free(screen);
}

void screen_draw(Screen *screen) {
  for (uintmax_t t = 0; t < rectify_array_size(screen->surfaces); t++) {
    Surface *surface = screen->surfaces[t];
    for (uintmax_t y = 0; y < surface->height; y++) {
      for (uintmax_t x = 0; x < surface->width; x++) {
        uintmax_t s_index = (y * surface->width) + x;
        uintmax_t index = ((y + surface->y) * screen->asciilayer->ascii_width) + (x + surface->x);
        if (index >= screen->asciilayer->ascii_size) {
          continue;
        }

        screen->asciilayer->asciimap[index] = surface->asciimap[s_index];
      }
    }
  }

  tiles_ascii_draw(screen->asciilayer2);
  tiles_ascii_draw(screen->asciilayer);
}
// -Screen
