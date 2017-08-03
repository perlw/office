#include <assert.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "screen.h"

bool scene_test_start(void);
void scene_test_stop(void);
void scene_test_update(double delta);

KronosSystem scene_test = {
  .name = "scene_test",
  .frames = 30,
  .start = &scene_test_start,
  .stop = &scene_test_stop,
  .update = &scene_test_update,
  .message = NULL,
};

typedef struct {
  double offset;
  Surface *surface;
} SceneTest;

void scene_test_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

SceneTest *scene_test_internal = NULL;
bool scene_test_start(void) {
  if (scene_test_internal) {
    return false;
  }

  const Config *const config = config_get();

  scene_test_internal = calloc(1, sizeof(SceneTest));
  *scene_test_internal = (SceneTest){
    .offset = 0.0,
    .surface = surface_create(0, 0, config->ascii_width, config->ascii_height),
  };

  screen_hook_render(&scene_test_internal_render_hook, NULL, 0);

  return true;
}

void scene_test_stop(void) {
  if (!scene_test_internal) {
    return;
  }

  screen_unhook_render(&scene_test_internal_render_hook, NULL);
  surface_destroy(&scene_test_internal->surface);

  free(scene_test_internal);
  scene_test_internal = NULL;
}

void scene_test_update(double delta) {
  if (!scene_test_internal) {
    return;
  }

  // Wave
  {
    scene_test_internal->offset += 0.1;

    double wave_depth = 0.25;
    double wave_thickness = M_PI * 4.0;
    double cx = scene_test_internal->surface->width / 2;
    double cy = scene_test_internal->surface->height / 2;
    for (uintmax_t y = 0; y < scene_test_internal->surface->height; y++) {
      for (uintmax_t x = 0; x < scene_test_internal->surface->width; x++) {
        double dx = fabs((double)x - cx);
        double dy = fabs((double)y - cy);
        double dist = sqrt(pow(dx, 2) + pow(dy, 2));
        double ndist = dist / 50.0;

        double final_color = ((cos((ndist * wave_thickness) + scene_test_internal->offset) + 1.0) / 4.0) + wave_depth;

        uint8_t color = (uint8_t)(final_color * 255.0);
        Glyph glyph = {
          .rune = 0,
          .fore = 0,
          .back = 0,
        };
        if (color < 96) {
          glyph.rune = '.';
        } else if (color < 178) {
          glyph.rune = '+';
        } else {
          glyph.rune = '*';
        }
        glyph.fore.r = (uint8_t)(255.0 * (1.0 - final_color));
        glyph.fore.g = (uint8_t)(255.0 * final_color);
        glyph.fore.b = 255;
        surface_glyph(scene_test_internal->surface, x, y, glyph);
      }
    }
  }
}

void scene_test_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  if (!scene_test_internal) {
    return;
  }
  surface_draw(scene_test_internal->surface, screen);
}
