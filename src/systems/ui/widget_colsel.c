#include <assert.h>

#define USE_KRONOS
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#include "internal.h"

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
      kronos_emit(MSG_UI_COLSEL_CHANGED, map);
      break;
    }
  }
}
