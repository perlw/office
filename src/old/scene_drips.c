#include <assert.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"

typedef struct {
  bool alive;
  double distance;
  double fade;
  uint32_t x;
  uint32_t y;
  Surface *surface;
} Drip;

typedef struct {
  double offset;
  double timing;
  double since_update;

  double wave_depth;
  double wave_thickness;
  uint32_t num_drips;
  Drip *drips;

  double next_drip;

  Surface *surface;
} SceneDrips;

SceneDrips *scene_drips_create(void) {
  SceneDrips *scene = calloc(1, sizeof(SceneDrips));

  const Config *const config = config_get();

  scene->offset = 0.0;
  scene->timing = 1.0 / 30.0;
  scene->since_update = 1.0 / (double)((rand() % 29) + 1);

  scene->wave_depth = 0.5;
  scene->wave_thickness = M_PI * 8.0;
  scene->num_drips = 10;
  scene->drips = calloc(scene->num_drips, sizeof(Drip));
  for (uint32_t t = 0; t < scene->num_drips; t++) {
    scene->drips[t].alive = false;
    scene->drips[t].surface = surface_create(0, 0, 32, 32);
  }
  scene->next_drip = 2.0 * (double)(rand() % 100) / 100.0;

  scene->surface = surface_create(0, 0, config->ascii_width, config->ascii_height);

  return scene;
}

void scene_drips_destroy(SceneDrips *const scene) {
  assert(scene);

  for (uint32_t t = 0; t < scene->num_drips; t++) {
    surface_destroy(scene->drips[t].surface);
  }
  free(scene->drips);
  surface_destroy(scene->surface);

  free(scene);
}

void scene_drips_update(SceneDrips *const scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    scene->next_drip -= scene->timing;
    if (scene->next_drip <= 0.0) {
      for (uint32_t t = 0; t < scene->num_drips; t++) {
        Drip *drip = &scene->drips[t];

        if (drip->alive) {
          continue;
        }

        drip->alive = true;
        drip->distance = 4;
        drip->fade = 1.0;
        drip->surface->x = (double)(rand() % (scene->surface->width - 32));
        drip->surface->y = (double)(rand() % (scene->surface->height - 32));
        surface_clear(drip->surface, (Glyph){
                                       .rune = 0,
                                     });

        gossip_emit("sound:play_drip", 0, NULL);
        break;
      }

      scene->next_drip = 2.0 * (double)(rand() % 100) / 100.0;
    }

    for (uint32_t t = 0; t < scene->num_drips; t++) {
      Drip *drip = &scene->drips[t];

      if (!drip->alive) {
        continue;
      }

      drip->distance += 0.2;
      drip->fade -= 0.016;
      if (drip->fade < 0.0) {
        drip->fade = 0.0;
      }

      double cx = drip->surface->width / 2;
      double cy = drip->surface->height / 2;
      for (uint32_t y = 0; y < drip->surface->width; y++) {
        for (uint32_t x = 0; x < drip->surface->height; x++) {
          double dx = fabs((double)x - cx);
          double dy = fabs((double)y - cy);
          double dist = sqrt(pow(dx, 2) + pow(dy, 2));
          double ndist = dist / 50.0;

          if (dist > drip->distance) {
            continue;
          }
          if (drip->distance > 14) {
            drip->alive = false;
          }

          double wave = ((cos((ndist * scene->wave_thickness) - drip->distance) + 1.0) / 4.0) + scene->wave_depth;

          uint32_t i = (y * drip->surface->width) + x;
          uint8_t rune = (uint8_t)(wave * 255.0);
          if (rune < 210 || (drip->distance > 10 && dist < 6)) {
            drip->surface->buffer[i].rune = 0;
          } else if (rune < 245) {
            drip->surface->buffer[i].rune = 247;
          } else {
            drip->surface->buffer[i].rune = '*';
          }

          double final_color = wave * drip->fade;
          uint32_t i2 = ((drip->surface->y + y) * scene->surface->width) + (drip->surface->x + x);
          drip->surface->buffer[i].fore = glyphcolor_muls(scene->surface->buffer[i2].fore, 0.5 + final_color);
        }
      }
    }

    scene->offset += 0.025;
    for (uintmax_t y = 0; y < scene->surface->height; y++) {
      for (uintmax_t x = 0; x < scene->surface->width; x++) {
        double dist = (double)x + (double)y;
        double wave = (dist / 50.0) * (M_PI * 2.0);

        double final_color = (cos(wave - scene->offset) + 1.0) / 2.0;
        final_color *= 0.15;
        final_color += 0.25;

        uintmax_t i = (y * scene->surface->width) + x;
        uint8_t color = (uint8_t)(final_color * 255.0);
        scene->surface->buffer[i].rune = '~';

        scene->surface->buffer[i].fore.r = 0;
        scene->surface->buffer[i].fore.g = (uint8_t)(196.0 * final_color);
        scene->surface->buffer[i].fore.b = (uint8_t)(255.0 * final_color);
      }
    }
  }
}

void scene_drips_draw(SceneDrips *const scene, AsciiBuffer *const screen) {
  assert(scene);

  surface_draw(scene->surface, screen);
  for (uint32_t t = 0; t < scene->num_drips; t++) {
    if (scene->drips[t].alive) {
      surface_draw(scene->drips[t].surface, screen);
    }
  }
}
