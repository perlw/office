#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "config.h"
#include "tiles/tiles.h"

typedef struct {
  double offset;
  double timing;
  double since_update;

  TilesAscii *screen;
} SceneSoundTest;

SceneSoundTest *scene_sound_test_create(const Config *config) {
  SceneSoundTest *scene = calloc(1, sizeof(SceneSoundTest));

  scene->offset = 0.0;
  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;

  scene->screen = tiles_ascii_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);

  return scene;
}

void scene_sound_test_destroy(SceneSoundTest *scene) {
  assert(scene);

  tiles_ascii_destroy(scene->screen);

  free(scene);
}

void scene_sound_test_update(SceneSoundTest *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;
  }
}

void scene_sound_test_draw(SceneSoundTest *scene) {
  assert(scene);

  tiles_ascii_draw(scene->screen);
}
