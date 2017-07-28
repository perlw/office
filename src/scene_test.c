#include <assert.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"

typedef struct {
  double timing;
  double since_update;

  double offset;
  Surface *surface;
} SceneTest;

SceneTest *scene_test_create(void) {
  const Config *const config = config_get();

  SceneTest *scene = calloc(1, sizeof(SceneTest));

  *scene = (SceneTest){
    .timing = 1.0 / 30.0,
    .since_update = 0.0,
    .offset = 0.0,
    .surface = surface_create(0, 0, config->ascii_width, config->ascii_height),
  };

  return scene;
}

void scene_test_destroy(SceneTest *const scene) {
  assert(scene);
  surface_destroy(scene->surface);
  free(scene);
}

void scene_test_update(SceneTest *const scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    // Wave
    {
      scene->offset += 0.1;

      double wave_depth = 0.25;
      double wave_thickness = M_PI * 4.0;
      double cx = scene->surface->width / 2;
      double cy = scene->surface->height / 2;
      for (uintmax_t y = 0; y < scene->surface->height; y++) {
        for (uintmax_t x = 0; x < scene->surface->width; x++) {
          double dx = fabs((double)x - cx);
          double dy = fabs((double)y - cy);
          double dist = sqrt(pow(dx, 2) + pow(dy, 2));
          double ndist = dist / 50.0;

          double final_color = ((cos((ndist * wave_thickness) + scene->offset) + 1.0) / 4.0) + wave_depth;

          uintmax_t i = (y * scene->surface->width) + x;
          uint8_t color = (uint8_t)(final_color * 255.0);
          if (color < 96) {
            scene->surface->buffer[i].rune = '.';
          } else if (color < 178) {
            scene->surface->buffer[i].rune = '+';
          } else {
            scene->surface->buffer[i].rune = '*';
          }
          scene->surface->buffer[i].fore.r = (uint8_t)(255.0 * (1.0 - final_color));
          scene->surface->buffer[i].fore.g = (uint8_t)(255.0 * final_color);
          scene->surface->buffer[i].fore.b = 255;
        }
      }
    }
  }
}

void scene_test_draw(SceneTest *const scene, AsciiBuffer *const screen) {
  assert(scene);
  surface_draw(scene->surface, screen);
}
