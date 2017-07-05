#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "ui/ui.h"

typedef enum {
  FONT_EVENT_RUNE_SELECTED = GOSSIP_ID_ALL + 1,
} FontWindowEvent;

typedef struct {
  double timing;
  double since_update;

  AsciiBuffer *ascii;

  Surface *world;
  Surface *overlay;

  struct {
    uint32_t x, y;
    float radius;
    bool rolling;
  } wave;

  uint8_t chosen_rune;
  UIWindow *font_window;
  GossipHandle font_window_event_handle;

  GossipHandle mouse_handle;
  GossipHandle rune_handle;
  uint32_t m_x, m_y;
  uint32_t o_x, o_y;
} SceneWorldEdit;

void scene_world_edit_mouse_event(uint32_t id, void *const subscriberdata, void *const userdata) {
  SceneWorldEdit *scene = (SceneWorldEdit *)subscriberdata;
  PicassoWindowMouseEvent *event = (PicassoWindowMouseEvent *)userdata;

  scene->o_x = scene->m_x;
  scene->o_y = scene->m_y;
  scene->m_x = (uint32_t)(event->x / 8.0);
  scene->m_y = (uint32_t)(event->y / 8.0);

  if (scene->m_x > 0 && scene->m_y > 0 && scene->m_x < scene->world->width - 1 && scene->m_y < scene->world->height - 1) {
    if (event->pressed) {
      gossip_emit(MSG_SOUND, MSG_SOUND_PLAY_BOOM, NULL);

      uint32_t index = (scene->m_y * scene->world->width) + scene->m_x;
      scene->world->buffer[index] = (Glyph){
        .rune = scene->chosen_rune,
        .fore = (GlyphColor){ 255, 255, 255 },
        .back = (GlyphColor){ 0, 0, 0 },
      };

      scene->wave.rolling = true;
      scene->wave.x = scene->m_x;
      scene->wave.y = scene->m_y;
      scene->wave.radius = 10.0f;
    }
  }
}

void font_window_events(uint32_t id, void *const subscriberdata, void *const userdata) {
  UIWindow *window = (UIWindow *)subscriberdata;

  switch (id) {
    case UI_WINDOW_EVENT_CLICK: {
      UIEventClick *event = (UIEventClick *)userdata;
      uint32_t rune = (event->y * 16) + event->x;
      gossip_emit(MSG_SCENE_EVENT, FONT_EVENT_RUNE_SELECTED, &rune);
      break;
    }
  }
}

void scene_world_edit_rune_selected(uint32_t id, void *const subscriberdata, void *const userdata) {
  SceneWorldEdit *scene = (SceneWorldEdit *)subscriberdata;
  scene->chosen_rune = *(uint32_t *)userdata;

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
}

SceneWorldEdit *scene_world_edit_create(const Config *config) {
  SceneWorldEdit *scene = calloc(1, sizeof(SceneWorldEdit));

  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;
  scene->m_x = 0;
  scene->m_y = 0;

  scene->ascii = ascii_buffer_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);

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

  scene->mouse_handle = gossip_subscribe(MSG_INPUT, MSG_INPUT_MOUSE, &scene_world_edit_mouse_event, scene);
  scene->rune_handle = gossip_subscribe(MSG_SCENE_EVENT, FONT_EVENT_RUNE_SELECTED, &scene_world_edit_rune_selected, scene);

  {
    scene->chosen_rune = 1;
    scene->font_window = ui_window_create(config->ascii_width - 20, 20, 18, 18);
    scene->font_window_event_handle = gossip_subscribe(MSG_UI_WINDOW, GOSSIP_ID_ALL, &font_window_events, scene->font_window);

    gossip_emit(MSG_SCENE_EVENT, FONT_EVENT_RUNE_SELECTED, &scene->chosen_rune);
  }

  return scene;
}

void scene_world_edit_destroy(SceneWorldEdit *scene) {
  assert(scene);

  gossip_unsubscribe(MSG_UI_WINDOW, GOSSIP_ID_ALL, scene->font_window_event_handle);
  gossip_unsubscribe(MSG_SCENE_EVENT, FONT_EVENT_RUNE_SELECTED, scene->rune_handle);
  gossip_unsubscribe(MSG_INPUT, MSG_INPUT_MOUSE, scene->mouse_handle);

  ui_window_destroy(scene->font_window);

  surface_destroy(scene->overlay);
  surface_destroy(scene->world);
  ascii_buffer_destroy(scene->ascii);

  free(scene);
}

void scene_world_edit_update(SceneWorldEdit *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    for (uint32_t t = 0; t < scene->overlay->size; t++) {
      scene->overlay->buffer[t].fore = glyphcolor_muls(scene->overlay->buffer[t].fore, 0.75);
      if (scene->overlay->buffer[t].fore.r < 0.9) {
        scene->overlay->buffer[t].rune = 0;
      }
    }

    if (scene->wave.rolling) {
      bool did_paint = false;
      int32_t x = (int32_t)(scene->wave.radius + 0.5f);
      int32_t y = 0;
      int32_t err = 0;
      while (x >= y) {
        uint32_t ax[] = {
          scene->wave.x + x,
          scene->wave.x + y,
          scene->wave.x - y,
          scene->wave.x - x,
          scene->wave.x - x,
          scene->wave.x - y,
          scene->wave.x + y,
          scene->wave.x + x,
        };
        uint32_t ay[] = {
          scene->wave.y + y,
          scene->wave.y + x,
          scene->wave.y + x,
          scene->wave.y + y,
          scene->wave.y - y,
          scene->wave.y - x,
          scene->wave.y - x,
          scene->wave.y - y,
        };
        for (uint32_t t = 0; t < 8; t++) {
          if (ax[t] > 0 && ax[t] < scene->overlay->width - 1
              && ay[t] > 0 && ay[t] < scene->overlay->height - 1) {
            uint32_t index = (ay[t] * scene->overlay->width) + ax[t];
            scene->overlay->buffer[index] = (Glyph){
              .rune = 2,
              .fore = (GlyphColor){ 255, 255, 0 },
              .back = 0,
            };
            did_paint = true;
          }
        }

        y++;
        if (err <= 0) {
          err += (2 * y) + 1;
        } else {
          x--;
          err += (2 * (y - x)) + 1;
        }
      }

      scene->wave.radius += 1.0f;
      scene->wave.rolling = did_paint;
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
      .fore = (GlyphColor){ 255, 255, 0 },
      .back = 0,
    };
  }

  ui_window_update(scene->font_window, delta);
}

void scene_world_edit_draw(SceneWorldEdit *scene) {
  assert(scene);

  surface_draw(scene->world, scene->ascii);
  surface_draw(scene->overlay, scene->ascii);
  ui_window_draw(scene->font_window, scene->ascii);

  ascii_buffer_draw(scene->ascii);
}
