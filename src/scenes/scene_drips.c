#include <assert.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "arkanis/math_3d.h"

#define USE_KRONOS
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "screen.h"

bool scene_drips_start(void);
void scene_drips_stop(void);
void scene_drips_update(double delta);
void scene_drips_message(uint32_t id, RectifyMap *const map);

KronosSystem scene_drips = {
  .name = "scene_drips",
  .frames = 30,
  .start = &scene_drips_start,
  .stop = &scene_drips_stop,
  .update = &scene_drips_update,
  .message = &scene_drips_message,
};

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
  double wave_depth;
  double wave_thickness;
  uint32_t num_drips;
  Drip *drips;

  double next_drip;

  Surface *surface;
} SceneDrips;

void scene_drips_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

SceneDrips *scene_drips_internal = NULL;
bool scene_drips_start(void) {
  if (scene_drips_internal) {
    return false;
  }

  scene_drips_internal = calloc(1, sizeof(SceneDrips));

  Config *const config = config_get();

  scene_drips_internal->offset = 0.0;
  scene_drips_internal->wave_depth = 0.5;
  scene_drips_internal->wave_thickness = M_PI * 8.0;
  scene_drips_internal->num_drips = 10;
  scene_drips_internal->drips = calloc(scene_drips_internal->num_drips, sizeof(Drip));
  for (uint32_t t = 0; t < scene_drips_internal->num_drips; t++) {
    scene_drips_internal->drips[t].alive = false;
    scene_drips_internal->drips[t].surface = surface_create(0, 0, 32, 32);
  }
  scene_drips_internal->next_drip = 2.0 * (double)(rand() % 100) / 100.0;

  scene_drips_internal->surface = surface_create(0, 0, config->ascii_width, config->ascii_height);

  screen_hook_render(&scene_drips_internal_render_hook, NULL, 0);

  return true;
}

void scene_drips_stop(void) {
  if (!scene_drips_internal) {
    return;
  }

  screen_unhook_render(&scene_drips_internal_render_hook, NULL);

  for (uint32_t t = 0; t < scene_drips_internal->num_drips; t++) {
    surface_destroy(&scene_drips_internal->drips[t].surface);
  }
  free(scene_drips_internal->drips);
  surface_destroy(&scene_drips_internal->surface);

  free(scene_drips_internal);
  scene_drips_internal = NULL;
}

void scene_drips_update(double delta) {
  if (!scene_drips_internal) {
    return;
  }

  scene_drips_internal->next_drip -= delta;
  if (scene_drips_internal->next_drip <= 0.0) {
    for (uint32_t t = 0; t < scene_drips_internal->num_drips; t++) {
      Drip *drip = &scene_drips_internal->drips[t];

      if (drip->alive) {
        continue;
      }

      drip->alive = true;
      drip->distance = 4;
      drip->fade = 1.0;
      drip->surface->x = (double)(rand() % (scene_drips_internal->surface->width - 32));
      drip->surface->y = (double)(rand() % (scene_drips_internal->surface->height - 32));
      surface_clear(drip->surface, (Glyph){
                                     .rune = 0,
                                   });

      RectifyMap *map = rectify_map_create();
      rectify_map_set(map, "sound", RECTIFY_MAP_TYPE_STRING, sizeof(char) * 5, "drip");
      kronos_post("sound", MSG_SOUND_PLAY, map);
      break;
    }

    scene_drips_internal->next_drip = 2.0 * (double)(rand() % 100) / 100.0;
  }

  for (uint32_t t = 0; t < scene_drips_internal->num_drips; t++) {
    Drip *drip = &scene_drips_internal->drips[t];

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

        double wave = ((cos((ndist * scene_drips_internal->wave_thickness) - drip->distance) + 1.0) / 4.0) + scene_drips_internal->wave_depth;

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
        uint32_t i2 = ((drip->surface->y + y) * scene_drips_internal->surface->width) + (drip->surface->x + x);
        drip->surface->buffer[i].fore = glyphcolor_muls(scene_drips_internal->surface->buffer[i2].fore, 0.5 + final_color);
      }
    }
  }

  scene_drips_internal->offset += 0.025;
  for (uintmax_t y = 0; y < scene_drips_internal->surface->height; y++) {
    for (uintmax_t x = 0; x < scene_drips_internal->surface->width; x++) {
      double dist = (double)x + (double)y;
      double wave = (dist / 50.0) * (M_PI * 2.0);

      double final_color = (cos(wave - scene_drips_internal->offset) + 1.0) / 2.0;
      final_color *= 0.15;
      final_color += 0.25;

      uintmax_t i = (y * scene_drips_internal->surface->width) + x;
      uint8_t color = (uint8_t)(final_color * 255.0);
      scene_drips_internal->surface->buffer[i].rune = '~';

      scene_drips_internal->surface->buffer[i].fore.r = 0;
      scene_drips_internal->surface->buffer[i].fore.g = (uint8_t)(196.0 * final_color);
      scene_drips_internal->surface->buffer[i].fore.b = (uint8_t)(255.0 * final_color);
    }
  }
}

void scene_drips_message(uint32_t id, RectifyMap *const map) {
  if (!scene_drips_internal) {
    return;
  }

  /*switch (id) {
    case MSG_INPUT_KEY: {
      PicassoWindowKeyboardEvent *const event = (PicassoWindowKeyboardEvent * const)rectify_map_get(map, "event");
      printf("KEY %d\n", event->key);
      break;
    }
  }*/
}

void scene_drips_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  if (!scene_drips_internal) {
    return;
  }

  surface_draw(scene_drips_internal->surface, screen);
  for (uint32_t t = 0; t < scene_drips_internal->num_drips; t++) {
    if (scene_drips_internal->drips[t].alive) {
      surface_draw(scene_drips_internal->drips[t].surface, screen);
    }
  }
}
