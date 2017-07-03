#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "ui/ui.h"

typedef struct {
  double timing;
  double since_update;

  AsciiBuffer *ascii;

  Surface *world;

  uint8_t chosen_rune;
  UIWindow *font_window;

  GossipHandle mouse_handle;
  uint32_t m_x, m_y;
} SceneWorldEdit;

void scene_world_edit_mouse_event(uint32_t id, void *const subscriberdata, void *const userdata) {
  SceneWorldEdit *scene = (SceneWorldEdit *)subscriberdata;
  PicassoWindowMouseEvent *event = (PicassoWindowMouseEvent *)userdata;

  scene->m_x = (uint32_t)(event->x / 8.0);
  scene->m_y = (uint32_t)(event->y / 8.0);

  if (scene->m_x > 0 && scene->m_y > 0 && scene->m_x < scene->world->width - 1 && scene->m_y < scene->world->height - 1) {
    if (event->pressed) {
      gossip_emit(MSG_SOUND, MSG_SOUND_PLAY_TAP, NULL);

      uint32_t index = (scene->m_y * scene->world->width) + scene->m_x;
      scene->world->buffer[index] = (Glyph){
        .rune = scene->chosen_rune,
        .fore = (GlyphColor){ 255, 255, 255 },
        .back = (GlyphColor){ 0, 0, 0 },
      };
    }
  }
}

void font_window_events(UIWindow *const window, UIWindowEvent event, void *const eventdata, void *const userdata) {
  SceneWorldEdit *scene = (SceneWorldEdit *)userdata;

  switch (event) {
    case UI_WINDOW_EVENT_CLICK: {
      UIEventClick *event = (UIEventClick *)eventdata;
      scene->chosen_rune = (event->y * 16) + event->x;
      break;
    }

    case UI_WINDOW_EVENT_PAINT: {
      for (uint32_t y = 0; y < 16; y++) {
        for (uint32_t x = 0; x < 16; x++) {
          uint8_t rune = (y * 16) + x;
          Glyph glyph = {
            .rune = rune,
            .fore = (GlyphColor){ 128, 128, 128 },
            .back = (GlyphColor){ 0, 0, 0 },
          };

          if (rune == scene->chosen_rune) {
            glyph.fore = (GlyphColor){ 255, 255, 255 };
          } else if (rune / 16 == scene->chosen_rune / 16) {
            glyph.fore = (GlyphColor){ 200, 200, 200 };
          } else if (rune % 16 == scene->chosen_rune % 16) {
            glyph.fore = (GlyphColor){ 200, 200, 200 };
          }

          ui_window_glyph(scene->font_window, x, y, glyph);
        }
      }
      break;
    }
  }
}

SceneWorldEdit *scene_world_edit_create(const Config *config) {
  SceneWorldEdit *scene = calloc(1, sizeof(SceneWorldEdit));

  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;
  scene->m_x = 0;
  scene->m_y = 0;

  scene->ascii = ascii_buffer_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);

  scene->world = surface_create(0, 0, config->ascii_width - 20, config->ascii_height);
  SurfaceRectTiles rect_tiles = {
    '+', '-', '+',
    '|', 0, '|',
    '+', '-', '+',
  };
  surface_rect(scene->world, 0, 0, scene->world->width, scene->world->height, rect_tiles, false, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });

  scene->mouse_handle = gossip_subscribe(MSG_INPUT, MSG_INPUT_MOUSE, &scene_world_edit_mouse_event, scene);

  {
    scene->chosen_rune = 1;
    scene->font_window = ui_window_create(config->ascii_width - 20, 20, 18, 18, &font_window_events, scene);
  }

  return scene;
}

void scene_world_edit_destroy(SceneWorldEdit *scene) {
  assert(scene);

  gossip_unsubscribe(MSG_INPUT, MSG_INPUT_MOUSE, scene->mouse_handle);

  ui_window_destroy(scene->font_window);

  surface_destroy(scene->world);
  ascii_buffer_destroy(scene->ascii);

  free(scene);
}

void scene_world_edit_update(SceneWorldEdit *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;
  }

  ui_window_update(scene->font_window, delta);
}

void scene_world_edit_draw(SceneWorldEdit *scene) {
  assert(scene);

  surface_draw(scene->world, scene->ascii);
  ui_window_draw(scene->font_window, scene->ascii);

  ascii_buffer_draw(scene->ascii);
}
