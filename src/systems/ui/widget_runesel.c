#include <assert.h>

#define USE_KRONOS
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#include "internal.h"

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
      kronos_emit(MSG_UI_RUNESEL_CHANGED, map);
      break;
    }
  }
}
