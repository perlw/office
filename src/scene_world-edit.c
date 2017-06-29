#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"

typedef struct {
  double timing;
  double since_update;

  AsciiBuffer *ascii;

  GossipHandle mouse_handle;
  uint32_t m_x, m_y;
} SceneWorldEdit;

void scene_world_edit_mouse_event(int32_t id, void *subscriberdata, void *userdata) {
  SceneWorldEdit *scene = (SceneWorldEdit *)subscriberdata;
  PicassoWindowMouseEvent *event = (PicassoWindowMouseEvent *)userdata;

  scene->m_x = (uint32_t)(event->x / 8.0);
  scene->m_y = (uint32_t)(event->y / 8.0);
}

SceneWorldEdit *scene_world_edit_create(const Config *config) {
  SceneWorldEdit *scene = calloc(1, sizeof(SceneWorldEdit));

  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;
  scene->m_x = 0;
  scene->m_y = 0;

  scene->ascii = ascii_buffer_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);

  scene->mouse_handle = gossip_subscribe(MSG_INPUT_MOUSE, &scene_world_edit_mouse_event, scene);

  return scene;
}

void scene_world_edit_destroy(SceneWorldEdit *scene) {
  assert(scene);

  gossip_unsubscribe(MSG_INPUT_MOUSE, scene->mouse_handle);

  ascii_buffer_destroy(scene->ascii);

  free(scene);
}

void scene_world_edit_update(SceneWorldEdit *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    {
      for (uint32_t t = 0; t < scene->ascii->size; t++) {
        scene->ascii->buffer[t].fore = glyphcolor_muls(scene->ascii->buffer[t].fore, 0.95);
        scene->ascii->buffer[t].back = glyphcolor_muls(scene->ascii->buffer[t].back, 0.95);
      }
    }
  }

  uint32_t index = (scene->m_y * scene->ascii->width) + scene->m_x;
  scene->ascii->buffer[index].rune = 1;
  scene->ascii->buffer[index].fore = (GlyphColor){ 255, 255, 0 };
  scene->ascii->buffer[index].back = (GlyphColor){ 0, 0, 0 };
}

void scene_world_edit_draw(SceneWorldEdit *scene) {
  assert(scene);

  ascii_buffer_draw(scene->ascii);
}
