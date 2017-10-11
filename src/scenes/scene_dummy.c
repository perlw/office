#include <assert.h>

#define USE_KRONOS
#include "bedrock/bedrock.h"

#include "ascii/ascii.h"

typedef struct {
  int dummy;
} SceneDummy;

SceneDummy *scene_dummy_create(void) {
  SceneDummy *scene = calloc(1, sizeof(SceneDummy));

  return scene;
}

void scene_dummy_destroy(SceneDummy *const scene) {
  assert(scene);

  free(scene);
}

void scene_dummy_update(SceneDummy *const scene, double delta) {
  assert(scene);
}

void scene_dummy_draw(SceneDummy *const scene, AsciiBuffer *const screen) {
  assert(scene);
}
