#include <assert.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "screen.h"

bool scene_secret_start(void);
void scene_secret_stop(void);
void scene_secret_update(double delta);
void scene_secret_message(uint32_t id, RectifyMap *const map);

KronosSystem scene_secret = {
  .name = "scene_secret",
  .frames = 30,
  .start = &scene_secret_start,
  .stop = &scene_secret_stop,
  .update = &scene_secret_update,
  .message = &scene_secret_message,
};

typedef struct {
  Surface *surface;
} SceneSecret;

void scene_secret_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

SceneSecret *scene_secret_internal = NULL;
bool scene_secret_start(void) {
  if (scene_secret_internal) {
    return false;
  }

  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set_string(map, "system", "debug");
    gossip_post("systems", MSG_SYSTEM_STOP, map);
  }

  Config *const config = config_get();

  scene_secret_internal = calloc(1, sizeof(SceneSecret));
  scene_secret_internal->surface = surface_create(0, 0, config->ascii_width, config->ascii_height);
  screen_hook_render(&scene_secret_internal_render_hook, NULL, 0);

  surface_clear(scene_secret_internal->surface, (Glyph){
                                                  .rune = ' ',
                                                  .fore = 0,
                                                  .back = 0,
                                                });
  // Initial screen
  {
    surface_textc(scene_secret_internal->surface, 1, 1, 0, "TITBIOS (C)1980 Taiwanese Integrated Tat,");
    surface_textc(scene_secret_internal->surface, 1, 2, 0, "(4XXAUS33) MUC880 For 486 PCI");
    surface_textc(scene_secret_internal->surface, 1, 3, 0, "CPU : TIT(R) Pusy(R) 486 CPU 33MHz");
    surface_textc(scene_secret_internal->surface, 1, 5, 0, "65152KB OK");
    surface_textc(scene_secret_internal->surface, 1, 7, 0, "WAIT...");
    surface_textc(scene_secret_internal->surface, 1, 9, 0, "CMOS battery state low");
    surface_textc(scene_secret_internal->surface, 1, 10, 0, "Keyboard Interface error");
    surface_textc(scene_secret_internal->surface, 1, 12, 0, "Press F1 to DIAGNOSE");
    surface_textc(scene_secret_internal->surface, 1, config->ascii_height - 3, 0, "(C) Taiwanese Integrated Tat,");
    surface_textc(scene_secret_internal->surface, 1, config->ascii_height - 2, 0, "41-PUSY-80085-00101111-TIT-UB0S9-H");
  }

  return true;
}

void scene_secret_stop(void) {
  if (!scene_secret_internal) {
    return;
  }
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set_string(map, "system", "debug");
    gossip_post("systems", MSG_SYSTEM_START, map);
  }

  screen_unhook_render(&scene_secret_internal_render_hook, NULL);
  surface_destroy(&scene_secret_internal->surface);

  free(scene_secret_internal);
  scene_secret_internal = NULL;
}

void scene_secret_update(double delta) {
  if (!scene_secret_internal) {
    return;
  }

  /*surface_clear(scene_secret_internal->surface, (Glyph){
                                                .rune = ' ',
                                                .fore = 0,
                                                .back = 0,
                                              });*/
}

void scene_secret_message(uint32_t id, RectifyMap *const map) {
  if (!scene_secret_internal) {
    return;
  }

  switch (id) {
    default:
      break;
  }
}

void scene_secret_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  if (!scene_secret_internal) {
    return;
  }
  surface_draw(scene_secret_internal->surface, screen);
}
