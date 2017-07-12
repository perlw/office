#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
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
  GossipHandle system_handle;
} DebugOverlay;

void debugoverlay_internal_update(DebugOverlay *overlay, double dt);

void debugoverlay_internal_scene_changed(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata) {
  DebugOverlay *overlay = (DebugOverlay *)subscriberdata;
  Scene *scene = (Scene *)userdata;

  snprintf(overlay->scene_buffer, 32, "SCENE: %s", scene->name);
  printf("%s\n", overlay->scene_buffer);
  surface_text(overlay->surface, 80 - (uint32_t)strnlen(overlay->scene_buffer, 32), 59, 32, overlay->scene_buffer, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 128, 0, 0 });
}

void debugoverlay_internal_system_event(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata) {
  DebugOverlay *overlay = (DebugOverlay *)subscriberdata;

  switch (id) {
    case MSG_SYSTEM_UPDATE:
      debugoverlay_internal_update(overlay, *(double *)userdata);
      break;

    case MSG_SYSTEM_DRAW_TOP: {
      AsciiBuffer *screen = (AsciiBuffer *)userdata;
      surface_draw(overlay->surface, screen);
      overlay->frames++;
      break;
    }
  }
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

  overlay->scene_handle = gossip_subscribe(MSG_SCENE, MSG_SCENE_CHANGED, &debugoverlay_internal_scene_changed, overlay, NULL);
  overlay->system_handle = gossip_subscribe(MSG_SYSTEM, GOSSIP_ID_ALL, &debugoverlay_internal_system_event, overlay, NULL);

  return overlay;
}

void debugoverlay_destroy(DebugOverlay *overlay) {
  assert(overlay);

  gossip_unsubscribe(overlay->system_handle);
  gossip_unsubscribe(overlay->scene_handle);

  surface_destroy(overlay->surface);

  free(overlay);
}

void debugoverlay_internal_update(DebugOverlay *overlay, double dt) {
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
