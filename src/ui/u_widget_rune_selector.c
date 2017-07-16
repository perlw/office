#include <assert.h>
#include <stdio.h>
#include <string.h>

#define UI_INTERNAL
#include "ui.h"

void ui_widget_rune_selector_internal_mouse_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  UIWidgetRuneSelector *widget = (UIWidgetRuneSelector *)subscriberdata;
  UIEventClick *event = (UIEventClick *)userdata;

  if (widget->parent != event->target) {
    return;
  }

  uint32_t rune = (event->y * 16) + event->x;
  widget->chosen_rune = rune;
  gossip_emit("widget:rune_selected", &rune);
}

void ui_widget_rune_selector_internal_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  UIWidgetRuneSelector *widget = (UIWidgetRuneSelector *)subscriberdata;
  UIWindow *window = (UIWindow *)userdata;

  if (widget->parent != window) {
    return;
  }

  if (strncmp(id, "paint", 128) == 0) {
    for (uint32_t y = 0; y < 16; y++) {
      for (uint32_t x = 0; x < 16; x++) {
        uint8_t rune = (y * 16) + x;
        Glyph glyph = {
          .rune = rune,
          .fore = (GlyphColor){ 128, 128, 128 },
          .back = (GlyphColor){ 0, 0, 0 },
        };

        if (rune == widget->chosen_rune) {
          glyph.fore = (GlyphColor){ 255, 255, 255 };
        } else if (rune / 16 == widget->chosen_rune / 16) {
          glyph.fore = (GlyphColor){ 200, 200, 200 };
        } else if (rune % 16 == widget->chosen_rune % 16) {
          glyph.fore = (GlyphColor){ 200, 200, 200 };
        }

        ui_window_glyph(window, x, y, glyph);
      }
    }
  }
}

UIWidgetRuneSelector *ui_widget_rune_selector_create(UIWindow *const parent) {
  UIWidgetRuneSelector *widget = calloc(1, sizeof(UIWidgetRuneSelector));

  *widget = (UIWidgetRuneSelector){
    .parent = parent,
    .chosen_rune = 1,
  };

  widget->event_handle = gossip_subscribe("widget:*", &ui_widget_rune_selector_internal_event, widget);
  widget->mouse_event_handle = gossip_subscribe("window:click", &ui_widget_rune_selector_internal_mouse_event, widget);

  return widget;
}

void ui_widget_rune_selector_destroy(UIWidgetRuneSelector *const widget) {
  assert(widget);

  gossip_unsubscribe(widget->mouse_event_handle);
  gossip_unsubscribe(widget->event_handle);

  free(widget);
}
