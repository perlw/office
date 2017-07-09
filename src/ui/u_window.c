#include <assert.h>
#include <stdint.h>

#include "ascii/ascii.h"
#include "messages.h"

#define UI_INTERNAL
#include "ui.h"

void ui_window_mouse_event(uint32_t id, void *const subscriberdata, void *const userdata) {
  UIWindow *window = (UIWindow *)subscriberdata;
  PicassoWindowMouseEvent *event = (PicassoWindowMouseEvent *)userdata;

  uint32_t m_x = (event->x / 8.0);
  uint32_t m_y = (event->y / 8.0);

  if (m_x > window->x && m_x < window->x + window->width - 1
      && m_y > window->y && m_y < window->y + window->width - 1) {
    gossip_emit(MSG_UI_WINDOW, UI_WINDOW_EVENT_MOUSEMOVE, &(UIEventMouseMove){
                                                            .target = window, .x = m_x - window->x - 1, .y = m_y - window->y - 1,
                                                          });
    if (event->pressed) {
      gossip_emit(MSG_UI_WINDOW, UI_WINDOW_EVENT_CLICK, &(UIEventClick){
                                                          .target = window, .x = m_x - window->x - 1, .y = m_y - window->y - 1,
                                                        });
    }
  }
}

UIWindow *ui_window_create(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
  UIWindow *window = calloc(1, sizeof(UIWindow));

  *window = (UIWindow){
    .timing = 1 / 30.0,
    .since_update = 1 / 30.0,
    .x = x,
    .y = y,
    .width = width,
    .height = height,
    .surface = surface_create(x, y, width, height),
  };

  SurfaceRectTiles rect_tiles = {
    201, 205, 187,
    186, 0, 186,
    200, 205, 188,
  };
  surface_rect(window->surface, 0, 0, window->surface->width, window->surface->height, rect_tiles, true, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 128, 0, 0 });
  window->surface->buffer[width - 2].rune = 198;
  window->surface->buffer[width - 3] = (Glyph){
    .rune = 254,
    .fore = (GlyphColor){ 255, 255, 0 },
    .back = (GlyphColor){ 0, 0, 0 },
  };
  window->surface->buffer[width - 4].rune = 181;

  window->mouse_handle = gossip_subscribe(MSG_INPUT, MSG_INPUT_MOUSE, &ui_window_mouse_event, window);

  return window;
}

void ui_window_destroy(UIWindow *const window) {
  assert(window);

  gossip_unsubscribe(window->mouse_handle);

  surface_destroy(window->surface);

  free(window);
}

void ui_window_glyph(UIWindow *const window, uint32_t x, uint32_t y, Glyph glyph) {
  assert(window);

  if (x > window->width - 2 || y > window->height - 2) {
    return;
  }

  uint32_t tx = x + 1;
  uint32_t ty = y + 1;

  window->surface->buffer[(ty * window->width) + tx] = glyph;
}

void ui_window_update(UIWindow *const window, double delta) {
  assert(window);

  window->since_update += delta;
  while (window->since_update >= window->timing) {
    window->since_update -= window->timing;

    gossip_emit(MSG_UI_WIDGET, UI_WIDGET_EVENT_PAINT, window);
  }
}

void ui_window_draw(UIWindow *const window, AsciiBuffer *const ascii) {
  assert(window);
  assert(ascii);

  surface_draw(window->surface, ascii);
}
