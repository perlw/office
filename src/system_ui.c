#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "screen.h"

bool system_ui_start(void);
void system_ui_stop(void);
void system_ui_update(double delta);
void system_ui_message(uint32_t id, RectifyMap *const map);

KronosSystem system_ui = {
  .name = "ui",
  .frames = 30,
  .start = &system_ui_start,
  .stop = &system_ui_stop,
  .update = &system_ui_update,
  .message = &system_ui_message,
};

typedef struct {
  char *title;
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  double handle;

  Surface *surface;
} UIWindow;

typedef struct {
  UIWindow *windows;
} SystemUI;

void system_ui_internal_window_draw_border(UIWindow *const window);
void system_ui_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

SystemUI *system_ui_internal = NULL;
bool system_ui_start(void) {
  if (system_ui_internal) {
    return false;
  }

  system_ui_internal = calloc(1, sizeof(SystemUI));
  system_ui_internal->windows = rectify_array_alloc(10, sizeof(UIWindow));

  screen_hook_render(&system_ui_internal_render_hook, NULL, 1);

  return true;
}

void system_ui_stop(void) {
  if (!system_ui_internal) {
    return;
  }

  screen_unhook_render(&system_ui_internal_render_hook, NULL);

  for (uint32_t t = 0; t < rectify_array_size(system_ui_internal->windows); t++) {
    free(system_ui_internal->windows[t].title);
    surface_destroy(&system_ui_internal->windows[t].surface);
  }
  rectify_array_free(&system_ui_internal->windows);
  free(system_ui_internal);
  system_ui_internal = NULL;
}

void system_ui_update(double delta) {
  if (!system_ui_internal) {
    return;
  }
}

void system_ui_message(uint32_t id, RectifyMap *const map) {
  if (!system_ui_internal) {
    return;
  }

  switch (id) {
    case MSG_UI_WINDOW_CREATE: {
      char *const title = rectify_map_get_string(map, "title");
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");
      uint32_t width = rectify_map_get_uint(map, "width");
      uint32_t height = rectify_map_get_uint(map, "height");
      double msg_id = rectify_map_get_double(map, "msg_id");

      if (!title) {
        printf("UI: Missing title when creating window\n");
        return;
      }

      UIWindow window = {
        .title = rectify_memory_alloc_copy(title, sizeof(char) * (strnlen(title, 128) + 1)),
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .handle = msg_id,
        .surface = surface_create(x, y, width, height),
      };
      system_ui_internal_window_draw_border(&window);

      system_ui_internal->windows = rectify_array_push(system_ui_internal->windows, &window);

      {
        RectifyMap *map = rectify_map_create();
        rectify_map_set_double(map, "handle", msg_id);
        gossip_emit(MSG_UI_WINDOW_CREATED, map);
      }

      break;
    }

    case MSG_UI_WINDOW_DESTROY: {
      printf("UI: Destroy window TBD\n");
      break;
    }

    case MSG_UI_WINDOW_GLYPH: {
      double handle = rectify_map_get_double(map, "handle");
      for (uint32_t t = 0; t < rectify_array_size(system_ui_internal->windows); t++) {
        UIWindow *window = &system_ui_internal->windows[t];

        if (window->handle == handle) {
          uint8_t rune = rectify_map_get_byte(map, "rune");
          uint32_t x = rectify_map_get_uint(map, "x");
          uint32_t y = rectify_map_get_uint(map, "y");
          GlyphColor fore = glyphcolor_hex(rectify_map_get_uint(map, "fore_color"));
          GlyphColor back = glyphcolor_hex(rectify_map_get_uint(map, "back_color"));

          if (x > window->width - 3 || y > window->height - 3) {
            break;
          }

          uint32_t tx = x + 1;
          uint32_t ty = y + 1;

          window->surface->buffer[(ty * window->width) + tx] = (Glyph){
            .rune = rune,
            .fore = fore,
            .back = back,
          };

          break;
        }
      }

      break;
    }
  }
}

void system_ui_internal_window_draw_border(UIWindow *const window) {
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

void system_ui_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  if (!system_ui_internal) {
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(system_ui_internal->windows); t++) {
    surface_draw(system_ui_internal->windows[t].surface, screen);
  }
}
