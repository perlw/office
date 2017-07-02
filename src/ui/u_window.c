#include <stdint.h>
#include <assert.h>

#include "ascii/ascii.h"
#include "messages.h"
#include "ui.h"

struct UIWindow {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;

  Surface *surface;

  GossipHandle mouse_handle;
  struct {
    UIWindowEventCallback callback;
    void *userdata;
  } events;
};

void ui_window_mouse_event(int32_t id, void *const subscriberdata, void *const userdata) {
  UIWindow *window = (UIWindow *)subscriberdata;
  PicassoWindowMouseEvent *event = (PicassoWindowMouseEvent *)userdata;

  if (!window->events.callback) {
    return;
  }

  uint32_t m_x = (event->x / 8.0);
  uint32_t m_y = (event->y / 8.0);

  if (m_x > window->x && m_x < window->x + window->width - 1
      && m_y > window->y && m_y < window->y + window->width - 1){
    window->events.callback(window, UI_WINDOW_EVENT_MOUSEMOVE, &(UIEventMouseMove){
      .x = m_x - window->x - 1,
      .y = m_y - window->y - 1,
    }, window->events.userdata);
    if (event->pressed) {
      window->events.callback(window, UI_WINDOW_EVENT_CLICK, &(UIEventClick){
      .x = m_x - window->x - 1,
      .y = m_y - window->y - 1,
      }, window->events.userdata);
    }
  }
}

UIWindow *ui_window_create(uint32_t x, uint32_t y, uint32_t width, uint32_t height, UIWindowEventCallback callback, void *const userdata) {
  UIWindow *window = calloc(1, sizeof(UIWindow));

  *window = (UIWindow){
    .x = x,
    .y = y,
    .width = width,
    .height = height,
    .surface = surface_create(x, y, width, height),
    .events = {
      .callback = callback,
      .userdata = userdata,
    },
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

  window->mouse_handle = gossip_subscribe(MSG_INPUT_MOUSE, &ui_window_mouse_event, window);

  return window;
}

void ui_window_destroy(UIWindow *const window) {
  assert(window);

  gossip_unsubscribe(MSG_INPUT_MOUSE, window->mouse_handle);

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
}

void ui_window_draw(UIWindow *const window, AsciiBuffer *const ascii) {
  assert(window);
  assert(ascii);

  window->events.callback(window, UI_WINDOW_EVENT_PAINT, NULL, window->events.userdata);

  surface_draw(window->surface, ascii);
}
