#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "bedrock/bedrock.h"

#include "config.h"
#include "messages.h"

typedef struct {
  double offset;
  double timing;
  double since_update;
  uint32_t frames;
  double current_second;
} SceneTest;

SceneTest *scene_test2_create(const Config *config) {
  SceneTest *scene = calloc(1, sizeof(SceneTest));

  scene->offset = 0.0;
  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;
  scene->frames = 0;

  return scene;
}

void scene_test2_destroy(SceneTest *scene) {
  assert(scene);

  free(scene);
}

void scene_test2_update(SceneTest *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;
  }

  scene->current_second += delta;
  if (scene->current_second >= 1) {
    printf("FPS: %d | MEM: %.2fkb", scene->frames, (double)occulus_current_allocated() / 1024.0);

    scene->current_second = 0;
    scene->frames = 0;
  }
}

void scene_test2_draw(SceneTest *scene) {
  assert(scene);

  scene->frames++;
}
