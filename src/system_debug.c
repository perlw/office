#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "scenes.h"
#include "screen.h"

bool system_debug_start(void);
void system_debug_stop(void);
void system_debug_update(double delta);
void system_debug_message(uint32_t id, RectifyMap *const map);

KronosSystem system_debug = {
  .name = "debug",
  .frames = 1,
  .autostart = true,
  .start = &system_debug_start,
  .stop = &system_debug_stop,
  .update = &system_debug_update,
  .message = &system_debug_message,
};

typedef struct {
  uint32_t frames;
  char fps_buffer[32];
  char scene_buffer[32];
  uintmax_t raw_mem[10];
  float mem_values[10];

  Surface *surface;
} DebugOverlay;

void system_debug_internal_scene_changed(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);
void system_debug_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

DebugOverlay *debugoverlay = NULL;
bool system_debug_start(void) {
  if (debugoverlay) {
    return false;
  }

  debugoverlay = calloc(1, sizeof(DebugOverlay));

  Config *const config = config_get();

  debugoverlay->surface = surface_create(0, 0, config->ascii_width, config->ascii_height);

  debugoverlay->frames = 0;
  snprintf(debugoverlay->fps_buffer, 32, "FPS: 0 | MEM: 0.00kb");
  surface_text(debugoverlay->surface, 0, config->ascii_height - 1, 31, debugoverlay->fps_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });

  memset(debugoverlay->raw_mem, 0, 10 * sizeof(uintmax_t));
  memset(debugoverlay->mem_values, 0, 10 * sizeof(float));
  surface_graph(debugoverlay->surface, config->ascii_width - 20, 0, 20, 6, 10, debugoverlay->mem_values);

  memset(debugoverlay->scene_buffer, 0, 32 * sizeof(char));
  snprintf(debugoverlay->scene_buffer, 32, "SCENE: na");
  surface_text(debugoverlay->surface, config->ascii_width - (uint32_t)strnlen(debugoverlay->scene_buffer, 32), config->ascii_height - 1, 32, debugoverlay->scene_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });

  screen_hook_render(&system_debug_internal_render_hook, debugoverlay, 9999);

  return true;
}

void system_debug_stop(void) {
  if (!debugoverlay) {
    return;
  }

  screen_unhook_render(&system_debug_internal_render_hook, debugoverlay);
  surface_destroy(&debugoverlay->surface);

  free(debugoverlay);
  debugoverlay = NULL;
}

void system_debug_update(double delta) {
  if (!debugoverlay) {
    return;
  }

  Config *const config = config_get();

  surface_clear(debugoverlay->surface, (Glyph){
                                         .rune = 0,
                                         .fore = 0,
                                         .back = 0,
                                       });

  snprintf(debugoverlay->fps_buffer, 32, "FPS: %d | MEM: %.2fkb", debugoverlay->frames, (double)occulus_current_usage() / 1024.0);
  surface_text(debugoverlay->surface, 0, config->ascii_height - 1, 31, debugoverlay->fps_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });
  surface_text(debugoverlay->surface, config->ascii_width - (uint32_t)strnlen(debugoverlay->scene_buffer, 32), config->ascii_height - 1, 32, debugoverlay->scene_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });

  {
    uintmax_t raw_new_mem = occulus_current_usage();
    uintmax_t raw_mem_max = raw_new_mem;
    for (uint32_t t = 0; t < 9; t++) {
      debugoverlay->raw_mem[t] = debugoverlay->raw_mem[t + 1];
      if (debugoverlay->raw_mem[t] > raw_mem_max) {
        raw_mem_max = debugoverlay->raw_mem[t];
      }
    }
    debugoverlay->raw_mem[9] = raw_new_mem;
    for (uint32_t t = 0; t < 10; t++) {
      debugoverlay->mem_values[t] = (float)debugoverlay->raw_mem[t] / (float)raw_mem_max;
    }

    surface_graph(debugoverlay->surface, config->ascii_width - 20, 0, 20, 6, 10, debugoverlay->mem_values);
  }

  debugoverlay->frames = 0;
}

void system_debug_message(uint32_t id, RectifyMap *const map) {
  if (!debugoverlay) {
    return;
  }

  switch (id) {
    case MSG_SCENE_CHANGED: {
      Config *const config = config_get();

      snprintf(debugoverlay->scene_buffer, 32, "SCENE: %s", (char *)rectify_map_get(map, "scene"));
      break;
    }
  }
}

void system_debug_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  if (!debugoverlay) {
    return;
  }

  surface_draw(debugoverlay->surface, screen);
  debugoverlay->frames++;
}
