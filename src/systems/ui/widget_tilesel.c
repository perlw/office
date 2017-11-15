#include <assert.h>

#define USE_KRONOS
#define USE_RECTIFY
#define USE_TOME
#include "bedrock/bedrock.h"

#include "internal.h"

TileSelWidget *tilesel_widget_create(void) {
  TileSelWidget *widget = calloc(1, sizeof(TileSelWidget));

  *widget = (TileSelWidget){
    .widget = (Widget){
      .destroy = &tilesel_widget_destroy,
      .draw = &tilesel_widget_draw,
      .event = &tilesel_widget_event,
    },
    .chosen_tile = 0,
    .tiledefs = (TileDef *)tome_fetch(ASSET_TILEDEFS, "tiledefs", "tiledefs.json"),
  };

  return widget;
}

void tilesel_widget_destroy(Widget **base_widget) {
  TileSelWidget *ptr = *(TileSelWidget **)base_widget;
  assert(ptr && base_widget);

  tome_release(ASSET_TILEDEFS, "tiledefs");

  free(ptr);
  *base_widget = NULL;
}

void tilesel_widget_draw(Widget *const base_widget, UIWindow *const window) {
  assert(base_widget && window);
  TileSelWidget *widget = (TileSelWidget *)base_widget;

  uint32_t max = rectify_array_size(widget->tiledefs);
  for (uint32_t y = 0; y < 16; y++) {
    for (uint32_t x = 0; x < 16; x++) {
      uint32_t i = (y * 16) + x;
      if (i >= max) {
        return;
      }

      Glyph glyph = widget->tiledefs[i].glyph;
      if (i == widget->chosen_tile) {
        glyph.back = glyphcolor_hex(0xff01ff);
      }

      surface_glyph(window->surface, x + 1, y + 1, glyph);
    }
  }
}

void tilesel_widget_event(Widget *const base_widget, uint32_t id, RectifyMap *const map) {
  assert(base_widget);
  TileSelWidget *widget = (TileSelWidget *)base_widget;

  switch (id) {
    case WIDGET_EVENT_CLICK: {
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");
      uint32_t i = (y * 16) + x;
      if (i >= rectify_array_size(widget->tiledefs)) {
        break;
      }

      widget->chosen_tile = i;
      {
        RectifyMap *map = rectify_map_create();
        rectify_map_set_byte(map, "rune", widget->tiledefs[widget->chosen_tile].glyph.rune);
        kronos_emit(MSG_WORLD_EDIT_RUNE_SELECTED, map);
      }
      {
        RectifyMap *map = rectify_map_create();
        rectify_map_set_uint(map, "color", glyphcolor_to_uint(widget->tiledefs[widget->chosen_tile].glyph.fore));
        kronos_emit(MSG_WORLD_EDIT_COLOR_SELECTED, map);
      }
      break;
    }
  }
}
