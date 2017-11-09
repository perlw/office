#include <assert.h>
#include <string.h>

#define USE_KRONOS
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#define USE_ASCII
#define USE_CONFIG
#define USE_MESSAGES
#include "main.h"

typedef struct Widget Widget;

typedef struct {
  char *title;
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  uint32_t handle;

  Widget *widget;

  Surface *surface;
} UIWindow;

typedef void (*WidgetDestroy)(Widget **base_widget);
typedef void (*WidgetEvent)(Widget *const base_widget, uint32_t id, RectifyMap *const map);
typedef void (*WidgetDraw)(Widget *const base_widget, UIWindow *const window);

struct Widget {
  WidgetDestroy destroy;
  WidgetDraw draw;
  WidgetEvent event;
};

#define FOREACH_WIDGET(WIDGET) \
  WIDGET(WIDGET_EVENT_CLICK)

typedef enum {
  FOREACH_WIDGET(GENERATE_ENUM)
} WidgetEvents;

// +RuneSelWidget
typedef struct {
  Widget widget;
  uint32_t chosen_rune;
} RuneSelWidget;

void runesel_widget_destroy(Widget **base_widget);
void runesel_widget_draw(Widget *const base_widget, UIWindow *const window);
void runesel_widget_event(Widget *const base_widget, uint32_t id, RectifyMap *const map);

RuneSelWidget *runesel_widget_create(void) {
  RuneSelWidget *widget = calloc(1, sizeof(RuneSelWidget));

  *widget = (RuneSelWidget){
    .widget = (Widget){
      .destroy = &runesel_widget_destroy,
      .draw = &runesel_widget_draw,
      .event = &runesel_widget_event,
    },
    .chosen_rune = 1,
  };

  return widget;
}

void runesel_widget_destroy(Widget **base_widget) {
  RuneSelWidget *ptr = *(RuneSelWidget **)base_widget;
  assert(ptr && base_widget);

  free(ptr);
  *base_widget = NULL;
}

void runesel_widget_draw(Widget *const base_widget, UIWindow *const window) {
  assert(base_widget && window);
  RuneSelWidget *widget = (RuneSelWidget *)base_widget;

  for (uint32_t y = 0; y < 16; y++) {
    for (uint32_t x = 0; x < 16; x++) {
      Glyph glyph = {
        .rune = (y * 16) + x,
        .fore = glyphcolor_hex(0x808080),
        .back = glyphcolor_hex(0x0),
      };
      if (glyph.rune == widget->chosen_rune) {
        glyph.fore = glyphcolor_hex(0xffffff);
        glyph.back = glyphcolor_hex(0x999999);
      } else if ((glyph.rune / 16 == widget->chosen_rune / 16) || (glyph.rune % 16 == widget->chosen_rune % 16)) {
        glyph.fore = glyphcolor_hex(0x8c8c8c);
        glyph.back = glyphcolor_hex(0x666666);
      }

      surface_glyph(window->surface, x + 1, y + 1, glyph);
    }
  }
}

void runesel_widget_event(Widget *const base_widget, uint32_t id, RectifyMap *const map) {
  assert(base_widget);
  RuneSelWidget *widget = (RuneSelWidget *)base_widget;

  switch (id) {
    case WIDGET_EVENT_CLICK: {
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");
      widget->chosen_rune = (y * 16) + x;
      RectifyMap *map = rectify_map_create();
      rectify_map_set_byte(map, "rune", widget->chosen_rune);
      kronos_emit(MSG_WORLD_EDIT_RUNE_SELECTED, map);
      break;
    }
  }
}
// -RuneSelWidget

// +ColSelWidget
typedef struct {
  Widget widget;
  uint32_t chosen_color;
} ColSelWidget;

void colsel_widget_destroy(Widget **base_widget);
void colsel_widget_draw(Widget *const base_widget, UIWindow *const window);
void colsel_widget_event(Widget *const base_widget, uint32_t id, RectifyMap *const map);

ColSelWidget *colsel_widget_create(void) {
  ColSelWidget *widget = calloc(1, sizeof(ColSelWidget));

  *widget = (ColSelWidget){
    .widget = (Widget){
      .destroy = &colsel_widget_destroy,
      .draw = &colsel_widget_draw,
      .event = &colsel_widget_event,
    },
    .chosen_color = 15,
  };

  return widget;
}

void colsel_widget_destroy(Widget **base_widget) {
  ColSelWidget *ptr = *(ColSelWidget **)base_widget;
  assert(ptr && base_widget);

  free(ptr);
  *base_widget = NULL;
}

void colsel_widget_draw(Widget *const base_widget, UIWindow *const window) {
  assert(base_widget && window);
  ColSelWidget *widget = (ColSelWidget *)base_widget;

  for (uint32_t y = 0; y < 16; y++) {
    for (uint32_t x = 0; x < 16; x++) {
      Glyph glyph = {
        .rune = ((y * 16) + x == widget->chosen_color ? '*' : 219),
        .fore = glyphcolor_hsl((double)y / 16.0, 1.0, (double)x / 16.0),
        .back = glyphcolor_hex(0x0),
      };

      surface_glyph(window->surface, x + 1, y + 1, glyph);
    }
  }
}

void colsel_widget_event(Widget *const base_widget, uint32_t id, RectifyMap *const map) {
  assert(base_widget);
  ColSelWidget *widget = (ColSelWidget *)base_widget;

  switch (id) {
    case WIDGET_EVENT_CLICK: {
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");
      widget->chosen_color = (y * 16) + x;
      RectifyMap *map = rectify_map_create();
      GlyphColor color = glyphcolor_hsl((double)(widget->chosen_color / 16.0) / 16.0, 1.0, (double)(widget->chosen_color % 16) / 16.0);
      rectify_map_set_uint(map, "color", glyphcolor_to_uint(color));
      kronos_emit(MSG_WORLD_EDIT_COLOR_SELECTED, map);
      break;
    }
  }
}
// -ColSelWidget

typedef struct {
  UIWindow *windows;
  uint32_t next_handle;
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
  system->next_handle = 0;

  return system;
}

void system_ui_stop(void **system) {
  SystemUI *ptr = *system;
  assert(ptr && system);

  for (uint32_t t = 0; t < rectify_array_size(ptr->windows); t++) {
    if (ptr->windows[t].widget) {
      ptr->windows[t].widget->destroy(&ptr->windows[t].widget);
    }

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
      char *const widget = rectify_map_get_string(map, "widget");

      if (!title) {
        printf("UI: Missing title when creating window\n");
        break;
      }

      Widget *widget_ptr = NULL;
      if (widget) {
        if (strncmp(widget, "runesel", 128) == 0) {
          widget_ptr = (Widget *)runesel_widget_create();
        } else if (strncmp(widget, "colsel", 128) == 0) {
          widget_ptr = (Widget *)colsel_widget_create();
        }
      }

      uint32_t handle = system->next_handle++;
      UIWindow window = {
        .title = rectify_memory_alloc_copy(title, sizeof(char) * (strnlen(title, 128) + 1)),
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .handle = handle,
        .widget = widget_ptr,
        .surface = surface_create(x, y, width, height),
      };
      system_ui_internal_window_draw_border(&window);

      system->windows = rectify_array_push(system->windows, &window);

      RectifyMap *map = rectify_map_create();
      rectify_map_set_uint(map, "handle", handle);
      return map;
    }

    case MSG_UI_WINDOW_DESTROY: {
      printf("UI: Destroy window TBD\n");
      break;
    }

    /*case MSG_UI_WINDOW_GLYPH: {
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
    }*/

    /*case MSG_INPUT_MOUSEMOVE: {
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
    }*/

    case MSG_INPUT_CLICK: {
      uint32_t button = rectify_map_get_uint(map, "button");
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");
      bool pressed = rectify_map_get_bool(map, "pressed");
      bool released = rectify_map_get_bool(map, "released");

      for (uint32_t t = 0; t < rectify_array_size(system->windows); t++) {
        UIWindow *window = &system->windows[t];

        if (!window->widget) {
          continue;
        }

        if (x > window->x && x < window->x + window->width - 1
            && y > window->y && y < window->y + window->width - 1) {
          RectifyMap *map = rectify_map_create();
          rectify_map_set_uint(map, "x", x - window->x - 1);
          rectify_map_set_uint(map, "y", y - window->y - 1);
          window->widget->event(window->widget, WIDGET_EVENT_CLICK, map);
          rectify_map_destroy(&map);
        }
      }

      break;
    }

    /*case MSG_INPUT_SCROLL: {
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
    }*/

    case MSG_SYSTEM_RENDER: {
      AsciiBuffer *screen = *(AsciiBuffer **)rectify_map_get(map, "screen");
      for (uint32_t t = 0; t < rectify_array_size(system->windows); t++) {
        if (system->windows[t].widget) {
          system->windows[t].widget->draw(system->windows[t].widget, &system->windows[t]);
        }

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
