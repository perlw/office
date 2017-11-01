#include <assert.h>
#include <string.h>

#define USE_KRONOS
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#define USE_ASCII
#define USE_CONFIG
#define USE_MESSAGES
#include "main.h"

typedef struct {
  char *title;
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  uint32_t handle;

  Surface *surface;
} UIWindow;

typedef struct {
  UIWindow *windows;
} SystemUI;

SystemUI *system_ui_start(void);
void system_ui_stop(void **system);
void system_ui_update(SystemUI *system, double delta);
RectifyMap *system_ui_message(SystemUI *system, uint32_t id, RectifyMap *const map);

KronosSystem system_ui = {
  .name = "ui",
  .frames = 30,
  .start = &system_ui_start,
  .stop = &system_ui_stop,
  .update = &system_ui_update,
  .message = &system_ui_message,
};

void system_ui_internal_window_draw_border(UIWindow *const window);

SystemUI *system_ui_start(void) {
  SystemUI *system = calloc(1, sizeof(SystemUI));
  system->windows = rectify_array_alloc(10, sizeof(UIWindow));

  return system;
}

void system_ui_stop(void **system) {
  SystemUI *ptr = *system;
  assert(ptr && system);

  for (uint32_t t = 0; t < rectify_array_size(ptr->windows); t++) {
    free(ptr->windows[t].title);
    surface_destroy(&ptr->windows[t].surface);
  }
  rectify_array_free((void **)&ptr->windows);
  free(ptr);
  *system = NULL;
}

void system_ui_update(SystemUI *system, double delta) {
  assert(system);
}

RectifyMap *system_ui_message(SystemUI *system, uint32_t id, RectifyMap *const map) {
  assert(system);

  switch (id) {
    case MSG_UI_WINDOW_CREATE: {
      char *const title = rectify_map_get_string(map, "title");
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");
      uint32_t width = rectify_map_get_uint(map, "width");
      uint32_t height = rectify_map_get_uint(map, "height");
      uint32_t handle = rectify_map_get_uint(map, "handle");

      if (!title) {
        printf("UI: Missing title when creating window\n");
        break;
      }

      UIWindow window = {
        .title = rectify_memory_alloc_copy(title, sizeof(char) * (strnlen(title, 128) + 1)),
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .handle = handle,
        .surface = surface_create(x, y, width, height),
      };
      system_ui_internal_window_draw_border(&window);

      system->windows = rectify_array_push(system->windows, &window);

      {
        RectifyMap *map = rectify_map_create();
        rectify_map_set_uint(map, "handle", handle);
        kronos_emit(MSG_UI_WINDOW_CREATED, map);
      }

      break;
    }

    case MSG_UI_WINDOW_DESTROY: {
      printf("UI: Destroy window TBD\n");
      break;
    }

    case MSG_UI_WINDOW_GLYPH: {
      uint32_t handle = rectify_map_get_uint(map, "handle");
      for (uint32_t t = 0; t < rectify_array_size(system->windows); t++) {
        UIWindow *window = &system->windows[t];

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

    case MSG_UI_WINDOW_GLYPHS: {
      uint32_t handle = rectify_map_get_uint(map, "handle");
      for (uint32_t t = 0; t < rectify_array_size(system->windows); t++) {
        UIWindow *window = &system->windows[t];

        if (window->handle == handle) {
          RectifyMapIter iter = rectify_map_iter(rectify_map_get_map(map, "glyphs"));
          for (RectifyMapItem item; rectify_map_iter_next(&iter, &item);) {
            if (item.type == RECTIFY_MAP_TYPE_MAP) {
              RectifyMap *item_map = (RectifyMap *)item.val;
              uint8_t rune = rectify_map_get_byte(item_map, "rune");
              uint32_t x = rectify_map_get_uint(item_map, "x");
              uint32_t y = rectify_map_get_uint(item_map, "y");
              GlyphColor fore = glyphcolor_hex(rectify_map_get_uint(item_map, "fore_color"));
              GlyphColor back = glyphcolor_hex(rectify_map_get_uint(item_map, "back_color"));

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
            }
          }

          break;
        }
      }

      break;
    }

    case MSG_INPUT_MOUSEMOVE: {
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");

      for (uint32_t t = 0; t < rectify_array_size(system->windows); t++) {
        UIWindow *window = &system->windows[t];

        if (x > window->x && x < window->x + window->width - 1
            && y > window->y && y < window->y + window->width - 1) {
          RectifyMap *map = rectify_map_create();
          rectify_map_set_uint(map, "handle", window->handle);
          rectify_map_set_uint(map, "x", x - window->x - 1);
          rectify_map_set_uint(map, "y", y - window->y - 1);
          kronos_emit(MSG_UI_WINDOW_MOUSEMOVE, map);
        }
      }

      break;
    }

    case MSG_INPUT_CLICK: {
      uint32_t button = rectify_map_get_uint(map, "button");
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");
      bool pressed = rectify_map_get_bool(map, "pressed");
      bool released = rectify_map_get_bool(map, "released");

      for (uint32_t t = 0; t < rectify_array_size(system->windows); t++) {
        UIWindow *window = &system->windows[t];

        if (x > window->x && x < window->x + window->width - 1
            && y > window->y && y < window->y + window->width - 1) {
          RectifyMap *map = rectify_map_create();
          rectify_map_set_uint(map, "handle", window->handle);
          rectify_map_set_uint(map, "button", button);
          rectify_map_set_uint(map, "x", x - window->x - 1);
          rectify_map_set_uint(map, "y", y - window->y - 1);
          rectify_map_set_bool(map, "pressed", pressed);
          rectify_map_set_bool(map, "released", released);
          kronos_emit(MSG_UI_WINDOW_CLICK, map);
        }
      }

      break;
    }

    case MSG_INPUT_SCROLL: {
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");
      int32_t scroll_x = rectify_map_get_int(map, "scroll_x");
      int32_t scroll_y = rectify_map_get_int(map, "scroll_y");

      for (uint32_t t = 0; t < rectify_array_size(system->windows); t++) {
        UIWindow *window = &system->windows[t];

        if (x > window->x && x < window->x + window->width - 1
            && y > window->y && y < window->y + window->width - 1) {
          RectifyMap *map = rectify_map_create();
          rectify_map_set_uint(map, "handle", window->handle);
          rectify_map_set_uint(map, "x", x - window->x - 1);
          rectify_map_set_uint(map, "y", y - window->y - 1);
          rectify_map_set_uint(map, "scroll_x", scroll_x);
          rectify_map_set_uint(map, "scroll_y", scroll_y);
          kronos_emit(MSG_UI_WINDOW_SCROLL, map);
        }
      }

      break;
    }

    case MSG_SYSTEM_RENDER: {
      AsciiBuffer *screen = *(AsciiBuffer **)rectify_map_get(map, "screen");
      for (uint32_t t = 0; t < rectify_array_size(system->windows); t++) {
        surface_draw(system->windows[t].surface, screen);
      }
      break;
    }
  }

  return NULL;
}

void system_ui_internal_window_draw_border(UIWindow *const window) {
  SurfaceRectTiles rect_tiles = {
    201,
    205,
    187,
    186,
    0,
    186,
    200,
    205,
    188,
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
