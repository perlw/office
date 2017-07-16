#include <assert.h>
#include <string.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"

typedef struct {
  double timing;
  double since_update;

  Surface *world;
  Surface *overlay;

  uint8_t chosen_rune;
  uint32_t chosen_color;

  bool painting;

  GossipHandle mouse_handle;
  GossipHandle rune_handle;
  GossipHandle color_handle;
  uint32_t m_x, m_y;
  uint32_t o_x, o_y;
} SceneWorldEdit;

void scene_world_edit_mouse_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  SceneWorldEdit *scene = (SceneWorldEdit *)subscriberdata;
  PicassoWindowMouseEvent *event = (PicassoWindowMouseEvent *)userdata;
  const Config *const config = config_get();

  scene->o_x = scene->m_x;
  scene->o_y = scene->m_y;
  scene->m_x = (uint32_t)(event->x / config->grid_size_width);
  scene->m_y = (uint32_t)(event->y / config->grid_size_height);

  if (strncmp(id, "click", 128) == 0 && scene->m_x > 0 && scene->m_y > 0 && scene->m_x < scene->world->width - 1 && scene->m_y < scene->world->height - 1) {
    scene->painting = event->pressed;
  }
}

void scene_world_edit_rune_selected(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  SceneWorldEdit *scene = (SceneWorldEdit *)subscriberdata;
  scene->chosen_rune = *(uint32_t *)userdata;
}

void scene_world_edit_color_selected(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  SceneWorldEdit *scene = (SceneWorldEdit *)subscriberdata;
  scene->chosen_color = *(uint32_t *)userdata;
}

SceneWorldEdit *scene_world_edit_create(void) {
  SceneWorldEdit *scene = calloc(1, sizeof(SceneWorldEdit));

  const Config *const config = config_get();

  scene->timing = 1.0 / 30.0;
  scene->since_update = 1.0 / (double)((rand() % 29) + 1);
  scene->m_x = 0;
  scene->m_y = 0;

  scene->world = surface_create(0, 0, config->ascii_width - 20, config->ascii_height);
  scene->overlay = surface_clone(scene->world);

  surface_clear(scene->world, (Glyph){
                                .rune = ' ',
                                .fore = 0,
                                .back = 0,
                              });

  SurfaceRectTiles rect_tiles = {
    '+', '-', '+',
    '|', 0, '|',
    '+', '-', '+',
  };
  surface_rect(scene->world, 0, 0, scene->world->width, scene->world->height, rect_tiles, false, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });

  scene->mouse_handle = gossip_subscribe("input:*", &scene_world_edit_mouse_event, scene);
  scene->rune_handle = gossip_subscribe("widget:rune_selected", &scene_world_edit_rune_selected, scene);
  scene->color_handle = gossip_subscribe("widget:color_selected", &scene_world_edit_color_selected, scene);

  {
    scene->chosen_rune = 1;
    scene->chosen_color = 0xffffff;
  }

  return scene;
}

void scene_world_edit_destroy(SceneWorldEdit *const scene) {
  assert(scene);

  gossip_unsubscribe(scene->color_handle);
  gossip_unsubscribe(scene->rune_handle);
  gossip_unsubscribe(scene->mouse_handle);

  surface_destroy(scene->overlay);
  surface_destroy(scene->world);

  free(scene);
}

void scene_world_edit_update(SceneWorldEdit *const scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  if (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;
  }

  if (scene->m_x > 0 && scene->m_x < scene->overlay->width - 1
      && scene->m_y > 0 && scene->m_y < scene->overlay->height - 1) {
    if (scene->painting) {
      uint32_t index = (scene->m_y * scene->world->width) + scene->m_x;
      scene->world->buffer[index] = (Glyph){
        .rune = scene->chosen_rune,
        .fore = glyphcolor_from_int(scene->chosen_color),
        .back = (GlyphColor){ 0, 0, 0 },
      };
    }
  }
  {
    uint32_t index = (scene->o_y * scene->overlay->width) + scene->o_x;
    scene->overlay->buffer[index] = (Glyph){
      .rune = 0,
      .fore = 0,
      .back = 0,
    };
  }
  if (scene->m_x > 0 && scene->m_x < scene->overlay->width - 1
      && scene->m_y > 0 && scene->m_y < scene->overlay->height - 1) {
    uint32_t index = (scene->m_y * scene->overlay->width) + scene->m_x;
    scene->overlay->buffer[index] = (Glyph){
      .rune = scene->chosen_rune,
      .fore = glyphcolor_from_int(scene->chosen_color),
      .back = 0,
    };
  }
}

void scene_world_edit_draw(SceneWorldEdit *const scene, AsciiBuffer *const screen) {
  assert(scene);

  surface_draw(scene->world, screen);
  surface_draw(scene->overlay, screen);
}
