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

  uint32_t runesel;
  uint32_t colsel;
  uint32_t tilesel;
} SceneUITest;

SceneUITest *scene_ui_test_start(void);
void scene_ui_test_stop(void **scene);
void scene_ui_test_update(SceneUITest *scene, double delta);
RectifyMap *scene_ui_test_message(SceneUITest *scene, uint32_t id, RectifyMap *const map);

void scene_ui_test_internal_setup(SceneUITest *const scene);

KronosSystem scene_ui_test = {
  .name = "scene_ui-test",
  .frames = 30,
  .start = &scene_ui_test_start,
  .stop = &scene_ui_test_stop,
  .update = &scene_ui_test_update,
  .message = &scene_ui_test_message,
};

SceneUITest *scene_ui_test_start(void) {
  Config *const config = config_get();

  SceneUITest *scene = calloc(1, sizeof(SceneUITest));
  *scene = (SceneUITest){
    .surface = NULL,
  };
  scene_ui_test_internal_setup(scene);

  // +RuneSel
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set_string(map, "title", "RuneSel");
    rectify_map_set_uint(map, "x", 141);
    rectify_map_set_uint(map, "y", 7);
    rectify_map_set_uint(map, "width", 18);
    rectify_map_set_uint(map, "height", 18);
    rectify_map_set_string(map, "widget", "runesel");
    RectifyMap *response = kronos_post_immediate("ui", MSG_UI_WINDOW_CREATE, map);
    if (response) {
      scene->runesel = rectify_map_get_uint(response, "handle");
      rectify_map_destroy(&response);
    }
    rectify_map_destroy(&map);
  }
  // -RuneSel

  // +ColSel
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set_string(map, "title", "ColSel");
    rectify_map_set_uint(map, "x", 141);
    rectify_map_set_uint(map, "y", 26);
    rectify_map_set_uint(map, "width", 18);
    rectify_map_set_uint(map, "height", 18);
    rectify_map_set_string(map, "widget", "colsel");
    RectifyMap *response = kronos_post_immediate("ui", MSG_UI_WINDOW_CREATE, map);
    if (response) {
      scene->colsel = rectify_map_get_uint(response, "handle");
      rectify_map_destroy(&response);
    }
    rectify_map_destroy(&map);
  }
  // -ColSel

  // +TileSel
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set_string(map, "title", "TileSel");
    rectify_map_set_uint(map, "x", 141);
    rectify_map_set_uint(map, "y", 45);
    rectify_map_set_uint(map, "width", 18);
    rectify_map_set_uint(map, "height", 18);
    rectify_map_set_string(map, "widget", "tilesel");
    RectifyMap *response = kronos_post_immediate("ui", MSG_UI_WINDOW_CREATE, map);
    if (response) {
      scene->tilesel = rectify_map_get_uint(response, "handle");
      rectify_map_destroy(&response);
    }
    rectify_map_destroy(&map);
  }
  // -TileSel

  return scene;
}

void scene_ui_test_stop(void **scene) {
  SceneUITest *ptr = *scene;
  assert(ptr && scene);

  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set_uint(map, "handle", ptr->runesel);
    RectifyMap *response = kronos_post_immediate("ui", MSG_UI_WINDOW_DESTROY, map);
    if (response) {
      rectify_map_print(response);
      rectify_map_destroy(&response);
    }
    rectify_map_destroy(&map);
  }
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set_uint(map, "handle", ptr->colsel);
    RectifyMap *response = kronos_post_immediate("ui", MSG_UI_WINDOW_DESTROY, map);
    if (response) {
      rectify_map_print(response);
      rectify_map_destroy(&response);
    }
    rectify_map_destroy(&map);
  }
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set_uint(map, "handle", ptr->tilesel);
    RectifyMap *response = kronos_post_immediate("ui", MSG_UI_WINDOW_DESTROY, map);
    if (response) {
      rectify_map_print(response);
      rectify_map_destroy(&response);
    }
    rectify_map_destroy(&map);
  }

  surface_destroy(&ptr->surface);

  free(ptr);
  *scene = NULL;
}

void scene_ui_test_update(SceneUITest *scene, double delta) {
  assert(scene);
}

RectifyMap *scene_ui_test_message(SceneUITest *scene, uint32_t id, RectifyMap *const map) {
  assert(scene);

  switch (id) {
    case MSG_RENDER_SETTINGS_UPDATE: {
      scene_ui_test_internal_setup(scene);
      break;
    }

    case MSG_SYSTEM_RENDER: {
      surface_draw(scene->surface, *(AsciiBuffer **)rectify_map_get(map, "screen"));
      break;
    }
  }

  return NULL;
}

void scene_ui_test_internal_setup(SceneUITest *const scene) {
  assert(scene);

  Config *const config = config_get();

  if (scene->surface) {
    surface_destroy(&scene->surface);
  }
  scene->surface = surface_create(0, 0, config->ascii_width, config->ascii_height);
}
