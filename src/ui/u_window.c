#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "input.h"

#define UI_INTERNAL
#include "ui.h"

void ui_window_internal_draw_border(UIWindow *const window);
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

  window->mouse_handle = gossip_subscribe("input:click", &ui_window_internal_mouse_event, window);

  return window;
}

void ui_window_destroy(UIWindow *const window) {
  assert(window);

  gossip_unsubscribe(window->mouse_handle);

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

void ui_window_update(UIWindow *const window, double delta) {
  assert(window);

  window->since_update += delta;
  while (window->since_update >= window->timing) {
    window->since_update -= window->timing;

    uintptr_t target = (uintptr_t)window;
    gossip_emit("widget:paint", sizeof(uintptr_t), &target);
  }
}

void ui_window_draw(UIWindow *const window, AsciiBuffer *const screen) {
  assert(window);

  surface_draw(window->surface, screen);
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

void ui_window_internal_mouse_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  UIWindow *window = (UIWindow *)subscriberdata;

  const Config *const config = config_get();

  if (strncmp(id, "mousemove", 128) == 0) {
    InputMouseMoveEvent *event = (InputMouseMoveEvent *)userdata;

    if (event->x > window->x && event->x < window->x + window->width - 1
        && event->y > window->y && event->y < window->y + window->width - 1) {
      gossip_emit("window:mousemove", sizeof(UIEventMouseMove), &(UIEventMouseMove){
                                                                  .target = window, .x = event->x - window->x - 1, .y = event->y - window->y - 1,
                                                                });
    }
  } else if (strncmp(id, "click", 128) == 0) {
    InputClickEvent *event = (InputClickEvent *)userdata;

    if (event->x > window->x && event->x < window->x + window->width - 1
        && event->y > window->y && event->y < window->y + window->width - 1) {
      if (event->pressed) {
        gossip_emit("window:click", sizeof(UIEventClick), &(UIEventClick){
                                                            .target = window, .x = event->x - window->x - 1, .y = event->y - window->y - 1,
                                                          });
      }
    }
  }
}
