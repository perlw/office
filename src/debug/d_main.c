#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "scenes.h"

typedef struct {
  uint32_t frames;
  double current_second;
  char fps_buffer[32];
  char scene_buffer[32];
  uintmax_t raw_mem[10];
  float mem_values[10];

  Surface *surface;

  GossipHandle scene_handle;
} DebugOverlay;

void debugoverlay_internal_scene_changed(const char *message, void *const subscriberdata, void *const userdata) {
  DebugOverlay *overlay = (DebugOverlay *)subscriberdata;
  Scene *scene = (Scene *)userdata;

  snprintf(overlay->scene_buffer, 32, "SCENE: %s", scene->name);
  printf("%s\n", overlay->scene_buffer);
  surface_text(overlay->surface, 80 - (uint32_t)strnlen(overlay->scene_buffer, 32), 59, 32, overlay->scene_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });
}

DebugOverlay *debugoverlay_create(void) {
  DebugOverlay *overlay = calloc(1, sizeof(DebugOverlay));

  const Config *const config = config_get();

  overlay->surface = surface_create(0, 0, config->ascii_width, config->ascii_height);

  overlay->frames = 0;
  overlay->current_second = 0.0;
  snprintf(overlay->fps_buffer, 32, "FPS: 0 | MEM: 0.00kb");
  surface_text(overlay->surface, 0, 59, 31, overlay->fps_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });

  memset(overlay->raw_mem, 0, 10 * sizeof(uintmax_t));
  memset(overlay->mem_values, 0, 10 * sizeof(float));
  surface_graph(overlay->surface, 60, 0, 20, 6, 10, overlay->mem_values);

  memset(overlay->scene_buffer, 0, 32 * sizeof(char));
  snprintf(overlay->scene_buffer, 32, "SCENE: na");
  surface_text(overlay->surface, 80 - (uint32_t)strnlen(overlay->scene_buffer, 32), 59, 32, overlay->scene_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });

  overlay->scene_handle = gossip_subscribe("scene:changed", &debugoverlay_internal_scene_changed, overlay);

  return overlay;
}

void debugoverlay_destroy(DebugOverlay *overlay) {
  assert(overlay);

  gossip_unsubscribe(overlay->scene_handle);

  surface_destroy(overlay->surface);

  free(overlay);
}

void debugoverlay_update(DebugOverlay *overlay, double dt) {
  assert(overlay);

  overlay->current_second += dt;
  if (overlay->current_second >= 1) {
    snprintf(overlay->fps_buffer, 32, "FPS: %d | MEM: %.2fkb", overlay->frames, (double)occulus_current_allocated() / 1024.0);
    surface_text(overlay->surface, 0, 59, 31, overlay->fps_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });

    {
      uintmax_t raw_new_mem = occulus_current_allocated();
      uintmax_t raw_mem_max = raw_new_mem;
      for (uint32_t t = 0; t < 9; t++) {
        overlay->raw_mem[t] = overlay->raw_mem[t + 1];
        if (overlay->raw_mem[t] > raw_mem_max) {
          raw_mem_max = overlay->raw_mem[t];
        }
      }
      overlay->raw_mem[9] = raw_new_mem;
      for (uint32_t t = 0; t < 10; t++) {
        overlay->mem_values[t] = (float)overlay->raw_mem[t] / (float)raw_mem_max;
      }

      surface_graph(overlay->surface, 60, 0, 20, 6, 10, overlay->mem_values);
    }

    overlay->current_second = 0;
    overlay->frames = 0;
  }
}

void debugoverlay_draw(DebugOverlay *overlay, AsciiBuffer *const screen) {
  assert(overlay);

  surface_draw(overlay->surface, screen);
  overlay->frames++;
}
