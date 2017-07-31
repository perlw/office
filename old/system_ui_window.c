#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "input.h"

#include "system_ui.h"

void ui_window_internal_draw_border(UIWindow *const window);
void ui_window_internal_mouse_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);

UIWindow *ui_window_create(const char *title, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
  UIWindow *window = calloc(1, sizeof(UIWindow));

  *window = (UIWindow){
    .title = rectify_memory_alloc_copy(title, sizeof(char) * (strlen(title) + 1)),
    .x = x,
    .y = y,
    .width = width,
    .height = height,
    .scroll_x = -1,
    .scroll_y = -1,
    .surface = surface_create(x, y, width, height),
  };
  window->handle = (UIWindowHandle)window;

  ui_window_internal_draw_border(window);

  window->mouse_handle = gossip_subscribe("input:*", &ui_window_internal_mouse_event, window);

  return window;
}

void ui_window_destroy(UIWindow *const window) {
  assert(window);

  gossip_unsubscribe(window->mouse_handle);
  surface_destroy(window->surface);

  free(window->title);
  free(window);
}

void ui_window_clear(UIWindow *const window, Glyph glyph) {
  assert(window);

  surface_clear(window->surface, glyph);
  ui_window_internal_draw_border(window);
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

void ui_window_scroll_x(UIWindow *const window, int32_t scroll) {
  assert(window);
  window->scroll_x = scroll;
}

void ui_window_scroll_y(UIWindow *const window, int32_t scroll) {
  assert(window);
  window->scroll_y = scroll;
}

void ui_window_update(UIWindow *const window, double delta) {
  assert(window);

  uintptr_t target = (uintptr_t)window;
  gossip_emit("widget:paint", sizeof(uintptr_t), &target);
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

  // Title
  uint32_t text_len = (uint32_t)strnlen(window->title, 32) + 1;
  window->surface->buffer[1].rune = 181;
  surface_text(window->surface, 2, 0, text_len, window->title, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 0, 0, 0 });
  window->surface->buffer[text_len + 1].rune = 198;

  // "Close" button
  window->surface->buffer[window->width - 2].rune = 198;
  window->surface->buffer[window->width - 3] = (Glyph){
    .rune = 'x',
    .fore = glyphcolor(255, 255, 0),
    .back = 0,
  };
  window->surface->buffer[window->width - 4].rune = 181;

  if (window->scroll_x >= 0) {
    // X scroll
    double scroll_mod = (int32_t)window->scroll_x / 100.0;
    double scroll_width = (double)(window->width - 5);
    uint32_t x_marker_pos = (uint32_t)((scroll_width * scroll_mod) + 0.5) + 2;
    uint32_t x_start = 1;
    uint32_t x_end = window->width - 2;
    for (uint32_t x = x_start; x < x_end + 1; x++) {
      uint32_t index = ((window->height - 1) * window->width) + x;

      if (x == x_start) {
        window->surface->buffer[index] = (Glyph){
          .rune = 181,
          .fore = glyphcolor(200, 200, 200),
          .back = glyphcolor(128, 0, 0),
        };
      } else if (x == x_end) {
        window->surface->buffer[index] = (Glyph){
          .rune = 198,
          .fore = glyphcolor(200, 200, 200),
          .back = glyphcolor(128, 0, 0),
        };
      } else if (x == x_marker_pos) {
        window->surface->buffer[index] = (Glyph){
          .rune = 254,
          .fore = glyphcolor(255, 255, 0),
          .back = glyphcolor(64, 0, 0),
        };
      } else {
        window->surface->buffer[index] = (Glyph){
          .rune = 196,
          .fore = glyphcolor(128, 128, 128),
          .back = glyphcolor(64, 0, 0),
        };
      }
    }
  }

  if (window->scroll_y >= 0) {
    // Y scroll
    double scroll_mod = (int32_t)window->scroll_y / 100.0;
    double scroll_height = (double)(window->height - 5);
    uint32_t y_marker_pos = (uint32_t)((scroll_height * scroll_mod) + 0.5) + 2;
    uint32_t y_start = 1;
    uint32_t y_end = window->height - 2;
    for (uint32_t y = y_start; y < y_end + 1; y++) {
      uint32_t index = (y * window->width) + window->width - 1;

      if (y == y_start) {
        window->surface->buffer[index] = (Glyph){
          .rune = 208,
          .fore = glyphcolor(200, 200, 200),
          .back = glyphcolor(128, 0, 0),
        };
      } else if (y == y_end) {
        window->surface->buffer[index] = (Glyph){
          .rune = 210,
          .fore = glyphcolor(200, 200, 200),
          .back = glyphcolor(128, 0, 0),
        };
      } else if (y == y_marker_pos) {
        window->surface->buffer[index] = (Glyph){
          .rune = 254,
          .fore = glyphcolor(255, 255, 0),
          .back = glyphcolor(64, 0, 0),
        };
      } else {
        window->surface->buffer[index] = (Glyph){
          .rune = 179,
          .fore = glyphcolor(128, 128, 128),
          .back = glyphcolor(64, 0, 0),
        };
      }
    }
  }
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
  } else if (strncmp(id, "mousescroll", 128) == 0) {
    InputMouseScrollEvent *event = (InputMouseScrollEvent *)userdata;

    if (event->x > window->x && event->x < window->x + window->width - 1
        && event->y > window->y && event->y < window->y + window->width - 1) {
      gossip_emit("window:scroll", sizeof(UIEventScroll), &(UIEventScroll){
                                                            .target = window, .scroll_x = event->scroll_x, .scroll_y = event->scroll_y, .x = event->x, .y = event->y,
                                                          });
    }
  }
}
