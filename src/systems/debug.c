#include <assert.h>
#include <stdio.h>
#include <string.h>

#define USE_KRONOS
#include "bedrock/bedrock.h"

#define USE_ASCII
#define USE_CONFIG
#define USE_MESSAGES
#define USE_SCENES
#define USE_SCREEN
#include "main.h"

typedef struct {
  uint32_t frames;
  char fps_buffer[32];
  char scene_buffer[32];
  uintmax_t raw_mem[10];
  float mem_values[10];

  Surface *surface;
} DebugOverlay;

DebugOverlay *system_debug_start(void);
void system_debug_stop(void **system);
void system_debug_update(DebugOverlay *system, double delta);
void system_debug_message(DebugOverlay *system, uint32_t id, RectifyMap *const map);

KronosSystem system_debug = {
  .name = "debug",
  .frames = 1,
  .autostart = true,
  .start = &system_debug_start,
  .stop = &system_debug_stop,
  .update = &system_debug_update,
  .message = &system_debug_message,
};

void system_debug_internal_scene_changed(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);
void system_debug_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

DebugOverlay *system_debug_start(void) {
  DebugOverlay *system = calloc(1, sizeof(DebugOverlay));

  Config *const config = config_get();

  system->surface = surface_create(0, 0, config->ascii_width, config->ascii_height);

  system->frames = 0;
  snprintf(system->fps_buffer, 32, "FPS: 0 | MEM: 0.00kb");
  surface_text(system->surface, 0, config->ascii_height - 1, 31, system->fps_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });

  memset(system->raw_mem, 0, 10 * sizeof(uintmax_t));
  memset(system->mem_values, 0, 10 * sizeof(float));
  surface_graph(system->surface, config->ascii_width - 20, 0, 20, 6, 10, system->mem_values);

  memset(system->scene_buffer, 0, 32 * sizeof(char));
  snprintf(system->scene_buffer, 32, "SCENE: na");
  surface_text(system->surface, config->ascii_width - (uint32_t)strnlen(system->scene_buffer, 32), config->ascii_height - 1, 32, system->scene_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });

  screen_hook_render(&system_debug_internal_render_hook, system, 9999);

  return system;
}

void system_debug_stop(DebugOverlay **system) {
  DebugOverlay *ptr = *system;
  assert(system && ptr);

  screen_unhook_render(&system_debug_internal_render_hook, ptr);
  surface_destroy(&ptr->surface);

  free(ptr);
  *system = NULL;
}

void system_debug_update(DebugOverlay *system, double delta) {
  assert(system);

  Config *const config = config_get();

  surface_clear(system->surface, (Glyph){
                                   .rune = 0,
                                   .fore = 0,
                                   .back = 0,
                                 });

  snprintf(system->fps_buffer, 32, "FPS: %d | MEM: %.2fkb", system->frames, (double)occulus_current_usage() / 1024.0);
  surface_text(system->surface, 0, config->ascii_height - 1, 31, system->fps_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });
  surface_text(system->surface, config->ascii_width - (uint32_t)strnlen(system->scene_buffer, 32), config->ascii_height - 1, 32, system->scene_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });

  {
    uintmax_t raw_new_mem = occulus_current_usage();
    uintmax_t raw_mem_max = raw_new_mem;
    for (uint32_t t = 0; t < 9; t++) {
      system->raw_mem[t] = system->raw_mem[t + 1];
      if (system->raw_mem[t] > raw_mem_max) {
        raw_mem_max = system->raw_mem[t];
      }
    }
    system->raw_mem[9] = raw_new_mem;
    for (uint32_t t = 0; t < 10; t++) {
      system->mem_values[t] = (float)system->raw_mem[t] / (float)raw_mem_max;
    }

    surface_graph(system->surface, config->ascii_width - 20, 0, 20, 6, 10, system->mem_values);
  }

  system->frames = 0;
}

void system_debug_message(DebugOverlay *system, uint32_t id, RectifyMap *const map) {
  assert(system);

  switch (id) {
    case MSG_SCENE_CHANGED: {
      Config *const config = config_get();

      snprintf(system->scene_buffer, 32, "SCENE: %s", (char *)rectify_map_get(map, "scene"));
      break;
    }
  }
}

void system_debug_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  assert(userdata);
  DebugOverlay *system = userdata;

  surface_draw(system->surface, screen);
  system->frames++;
}
