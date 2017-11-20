#include <assert.h>

#define USE_KRONOS
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#define USE_ASCII
#define USE_CONFIG
#define USE_MESSAGES
#include "main.h"

typedef struct {
  Surface *surface;
} SceneTitle;

SceneTitle *scene_title_start(void);
void scene_title_stop(void **scene);
void scene_title_update(SceneTitle *scene, double delta);
RectifyMap *scene_title_message(SceneTitle *scene, uint32_t id, RectifyMap *const map);

void scene_title_internal_setup(SceneTitle *const scene);

KronosSystem scene_title = {
  .name = "scene_title",
  .frames = 30,
  .start = &scene_title_start,
  .stop = &scene_title_stop,
  .update = &scene_title_update,
  .message = &scene_title_message,
};

SceneTitle *scene_title_start(void) {
  Config *const config = config_get();

  SceneTitle *scene = calloc(1, sizeof(SceneTitle));
  *scene = (SceneTitle){
    .surface = NULL,
  };
  scene_title_internal_setup(scene);

  return scene;
}

void scene_title_stop(void **scene) {
  SceneTitle *ptr = *scene;
  assert(ptr && scene);

  surface_destroy(&ptr->surface);

  free(ptr);
  *scene = NULL;
}

void scene_title_update(SceneTitle *scene, double delta) {
  assert(scene);
}

RectifyMap *scene_title_message(SceneTitle *scene, uint32_t id, RectifyMap *const map) {
  assert(scene);

  switch (id) {
    case MSG_RENDER_SETTINGS_UPDATE: {
      scene_title_internal_setup(scene);
      break;
    }

    case MSG_SYSTEM_RENDER: {
      surface_draw(scene->surface, *(AsciiBuffer **)rectify_map_get(map, "screen"));
      break;
    }
  }

  return NULL;
}

void scene_title_internal_setup(SceneTitle *const scene) {
  assert(scene);

  Config *const config = config_get();

  if (scene->surface) {
    surface_destroy(&scene->surface);
  }
  scene->surface = surface_create(0, 0, config->ascii_width, config->ascii_height);
}
