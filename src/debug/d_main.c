#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "scenes.h"

typedef struct {
  TilesAscii *ascii;
  uint32_t frames;
  double current_second;
  char fps_buffer[32];
  char scene_buffer[32];
  uintmax_t raw_mem[10];
  float mem_values[10];
} DebugOverlay;

void ascii_text(TilesAscii *tiles, uint32_t x, uint32_t y, uint32_t length, const char *string) {
  assert(tiles);

  if (x >= tiles->ascii_width || y >= tiles->ascii_height) {
    return;
  }

  bool skip = false;
  uint32_t index = (y * tiles->ascii_width) + x;
  uint32_t max = index + (x + length >= tiles->ascii_width ? tiles->ascii_width - x : x + length);
  for (uint32_t t = index, u = 0; t < max; t++, u++) {
    if (string[u] == '\0') {
      skip = true;
    }

    if (!skip) {
      tiles->asciimap[t].rune = string[u];
      tiles->asciimap[t].fore = (GlyphColor){ 255, 255, 255 };
      tiles->asciimap[t].back = (GlyphColor){ 128, 0, 0 };
    } else {
      tiles->asciimap[t].rune = 0;
      tiles->asciimap[t].fore = (GlyphColor){ 0, 0, 0 };
      tiles->asciimap[t].back = (GlyphColor){ 255, 0, 255 };
    }
  }
}

float lerp(float a, float b, float t) {
  return ((1.0f - t) * a) + (t * b);
}

void ascii_graph(TilesAscii *tiles, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t num_values, const float *values) {
  assert(tiles);
  assert(values);

  if (x >= tiles->ascii_width || y >= tiles->ascii_height || x + width > tiles->ascii_width || y + height > tiles->ascii_height) {
    return;
  }

  uint32_t width_per_step = width / num_values;
  for (uint32_t t = 0; t < num_values; t++) {
    uint32_t val_height = (uint32_t)((float)(height - 1) * (1.0 - values[t]));
    uint32_t next_val_height = (t < num_values - 1 ? (uint32_t)((float)(height - 1) * (1.0 - values[t + 1])) : val_height);

    for (uint32_t yy = 0; yy < height; yy++) {
      for (uint32_t xx = 0; xx < width_per_step; xx++) {
        uint32_t fudged_height = (uint32_t)(lerp((float)val_height, (float)next_val_height, (float)xx / (float)width_per_step) + 0.5f);

        uint32_t index = ((y + yy) * tiles->ascii_width) + (x + (t * width_per_step) + xx);
        if (yy >= fudged_height) {
          if (yy == fudged_height) {
            tiles->asciimap[index].rune = '+';
            tiles->asciimap[index].fore = (GlyphColor){ 255, 255, 255 };
          } else if (yy > fudged_height) {
            tiles->asciimap[index].rune = '.';
            tiles->asciimap[index].fore = (GlyphColor){ 128, 128, 128 };
          }
          tiles->asciimap[index].back = (GlyphColor){ 66, 66, 66 };
        } else {
          tiles->asciimap[index].rune = 0;
          tiles->asciimap[index].fore = (GlyphColor){ 0, 0, 0 };
          tiles->asciimap[index].back = (GlyphColor){ 255, 0, 255 };
        }
      }
    }
  }
}

void scene_changed(int32_t id, void *subscriberdata, void *userdata) {
  DebugOverlay *overlay = (DebugOverlay *)subscriberdata;
  Scene *scene = (Scene *)userdata;

  snprintf(overlay->scene_buffer, 32, "SCENE: %s", scene->name);
  printf("%s\n", overlay->scene_buffer);
  ascii_text(overlay->ascii, 80 - (uint32_t)strnlen(overlay->scene_buffer, 32), 59, 32, overlay->scene_buffer);
}

DebugOverlay *debugoverlay_create(const Config *config) {
  DebugOverlay *overlay = calloc(1, sizeof(DebugOverlay));

  overlay->ascii = tiles_ascii_create(config->res_width, config->res_height, config->res_width / 8, config->res_height / 8);
  for (uint32_t t = 0; t < overlay->ascii->ascii_size; t++) {
    overlay->ascii->asciimap[t].rune = 0;
    overlay->ascii->asciimap[t].fore = (GlyphColor){ 0, 0, 0 };
    overlay->ascii->asciimap[t].back = (GlyphColor){ 255, 0, 255 };
  }

  overlay->frames = 0;
  overlay->current_second = 0.0;
  snprintf(overlay->fps_buffer, 32, "FPS: 0 | MEM: 0.00kb");
  ascii_text(overlay->ascii, 0, 59, 32, overlay->fps_buffer);

  memset(overlay->raw_mem, 0, 10 * sizeof(uintmax_t));
  memset(overlay->mem_values, 0, 10 * sizeof(float));
  ascii_graph(overlay->ascii, 60, 0, 20, 6, 10, overlay->mem_values);

  memset(overlay->scene_buffer, 0, 32 * sizeof(char));
  snprintf(overlay->scene_buffer, 32, "SCENE: na");
  ascii_text(overlay->ascii, 80 - (uint32_t)strnlen(overlay->scene_buffer, 32), 59, 32, overlay->scene_buffer);

  gossip_subscribe(MSG_SCENE_CHANGED, &scene_changed, overlay);

  return overlay;
}

void debugoverlay_destroy(DebugOverlay *overlay) {
  assert(overlay);

  tiles_ascii_destroy(overlay->ascii);

  free(overlay);
}

void debugoverlay_update(DebugOverlay *overlay, double dt) {
  assert(overlay);

  overlay->current_second += dt;
  if (overlay->current_second >= 1) {
    snprintf(overlay->fps_buffer, 32, "FPS: %d | MEM: %.2fkb", overlay->frames, (double)occulus_current_allocated() / 1024.0);
    ascii_text(overlay->ascii, 0, 59, 32, overlay->fps_buffer);

    {
      uintmax_t raw_new_mem = occulus_current_allocated();
      uintmax_t raw_mem_max = raw_new_mem;
      for (uint32_t t = 0; t < 9; t++) {
        overlay->raw_mem[t] = overlay->raw_mem[t + 1];
        if (overlay->raw_mem[t] > raw_mem_max) {
          raw_mem_max = overlay->raw_mem[t];
        }
      }
      overlay->raw_mem[9] = raw_new_mem;
      for (uint32_t t = 0; t < 10; t++) {
        overlay->mem_values[t] = (float)overlay->raw_mem[t] / (float)raw_mem_max;
      }

      ascii_graph(overlay->ascii, 60, 0, 20, 6, 10, overlay->mem_values);
    }

    overlay->current_second = 0;
    overlay->frames = 0;
  }
}

void debugoverlay_draw(DebugOverlay *overlay) {
  assert(overlay);

  tiles_ascii_draw(overlay->ascii);
  overlay->frames++;
}
