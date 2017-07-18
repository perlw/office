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
  int dummy;
} Drip;

typedef struct {
  double timing;
  double since_update;

  uint32_t num_drips;
  Drip *drips;

  Surface *surface;
} SceneDrips;

SceneDrips *scene_drips_create(void) {
  SceneDrips *scene = calloc(1, sizeof(SceneDrips));

  const Config *const config = config_get();

  scene->timing = 1.0 / 30.0;
  scene->since_update = 1.0 / (double)((rand() % 29) + 1);

  scene->num_drips = 10;
  scene->drips = calloc(scene->num_drips, sizeof(Drip));
  for (uint32_t t = 0; t < scene->num_drips; t++) {
  }

  scene->surface = surface_create(0, 0, config->ascii_width, config->ascii_height);

  return scene;
}

void scene_drips_destroy(SceneDrips *const scene) {
  assert(scene);

  free(scene->drips);
  surface_destroy(scene->surface);

  free(scene);
}

void scene_drips_update(SceneDrips *const scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    for (uint32_t t = 0; t < scene->num_drips; t++) {
    }
  }
}

void scene_drips_draw(SceneDrips *const scene, AsciiBuffer *const screen) {
  assert(scene);

  surface_draw(scene->surface, screen);
}
