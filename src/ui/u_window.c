#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "config.h"

#define UI_INTERNAL
#include "ui.h"

void ui_window_internal_draw_border(UIWindow *const window);
void ui_window_internal_window_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);
void ui_window_internal_mouse_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);

UIWindow *ui_window_create(const char *title, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
  UIWindow *window = calloc(1, sizeof(UIWindow));

  *window = (UIWindow){
    .timing = 1.0 / 30.0,
    .since_update = 1.0 / (double)((rand() % 29) + 1),
    .title = rectify_memory_alloc_copy(title, sizeof(char) * (strlen(title) + 1)),
    .x = x,
    .y = y,
    .width = width,
    .height = height,
    .surface = surface_create(x, y, width, height),
  };

  ui_window_internal_draw_border(window);

  window->window_handle = gossip_subscribe("window:*", &ui_window_internal_window_event, window);
  window->mouse_handle = gossip_subscribe("input:click", &ui_window_internal_mouse_event, window);

  return window;
}

void ui_window_destroy(UIWindow *const window) {
  assert(window);

  gossip_unsubscribe(window->mouse_handle);
  gossip_unsubscribe(window->window_handle);

  surface_destroy(window->surface);

  free(window->title);

  free(window);
}

void ui_window_glyph(UIWindow *const window, uint32_t x, uint32_t y, Glyph glyph) {
  assert(window);

  if (x > window->width - 3 || y > window->height - 3) {
    return;
  }

  uint32_t tx = x + 1;
  uint32_t ty = y + 1;

  window->surface->buffer[(ty * window->width) + tx] = glyph;
}

void ui_window_internal_draw_border(UIWindow *const window) {
  SurfaceRectTiles rect_tiles = {
    201, 205, 187,
    186, 0, 186,
    200, 205, 188,
  };
  surface_rect(window->surface, 0, 0, window->width, window->height, rect_tiles, true, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 128, 0, 0 });

  uint32_t text_len = (uint32_t)strnlen(window->title, 32) + 1;
  window->surface->buffer[1].rune = 181;
  surface_text(window->surface, 2, 0, text_len, window->title, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 0, 0, 0 });
  window->surface->buffer[text_len + 1].rune = 198;

  window->surface->buffer[window->width - 2].rune = 198;
  window->surface->buffer[window->width - 3] = (Glyph){
    .rune = 254,
    .fore = (GlyphColor){ 255, 255, 0 },
    .back = (GlyphColor){ 0, 0, 0 },
  };
  window->surface->buffer[window->width - 4].rune = 181;
}

void ui_window_internal_window_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  UIWindow *window = (UIWindow *)subscriberdata;

  if (strncmp(id, "update", 128) == 0) {
    gossip_emit("widget:paint", window);
  } else if (strncmp(id, "draw", 128) == 0) {
    AsciiBuffer *const screen = (AsciiBuffer * const)userdata;
    surface_draw(window->surface, screen);
  }
}

void ui_window_internal_mouse_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  UIWindow *window = (UIWindow *)subscriberdata;
  PicassoWindowMouseEvent *event = (PicassoWindowMouseEvent *)userdata;

  const Config *const config = config_get();

  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);

  if (m_x > window->x && m_x < window->x + window->width - 1
      && m_y > window->y && m_y < window->y + window->width - 1) {
    gossip_emit("window:mousemove", &(UIEventMouseMove){
                                      .target = window, .x = m_x - window->x - 1, .y = m_y - window->y - 1,
                                    });
    if (event->pressed) {
      gossip_emit("window:click", &(UIEventClick){
                                    .target = window, .x = m_x - window->x - 1, .y = m_y - window->y - 1,
                                  });
    }
  }
}
