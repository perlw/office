#include <assert.h>

#define USE_KRONOS
#include "bedrock/bedrock.h"

#define USE_ASCII
#define USE_CONFIG
#define USE_MESSAGES
#define USE_SCREEN
#include "main.h"

typedef struct {
  double offset;
  Surface *surface;
} SceneTest;

SceneTest *scene_test_start(void);
void scene_test_stop(void **scene);
void scene_test_update(SceneTest *scene, double delta);

KronosSystem scene_test = {
  .name = "scene_test",
  .frames = 30,
  .start = &scene_test_start,
  .stop = &scene_test_stop,
  .update = &scene_test_update,
  .message = NULL,
};

void scene_test_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

SceneTest *scene_test_start(void) {
  Config *const config = config_get();

  SceneTest *scene = calloc(1, sizeof(SceneTest));
  *scene = (SceneTest){
    .offset = 0.0,
    .surface = surface_create(0, 0, config->ascii_width, config->ascii_height),
  };

  screen_hook_render(&scene_test_internal_render_hook, scene, 0);

  return scene;
}

void scene_test_stop(void **scene) {
  SceneTest *ptr = *scene;

  screen_unhook_render(&scene_test_internal_render_hook, ptr);
  surface_destroy(&ptr->surface);

  free(ptr);
  *scene = NULL;
}

void scene_test_update(SceneTest *scene, double delta) {
  assert(scene);

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
        surface_glyph(scene->surface, x, y, glyph);
      }
    }
  }
}

void scene_test_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  SceneTest *scene = userdata;
  surface_draw(scene->surface, screen);
}
