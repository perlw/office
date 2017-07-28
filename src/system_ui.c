#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "screen.h"
#include "system_ui.h"

bool system_ui_start(void);
void system_ui_stop(void);
void system_ui_update(void);

KronosSystem system_ui = {
  .name = "ui",
  .frames = 30,
  .start = &system_ui_start,
  .stop = &system_ui_stop,
  .update = &system_ui_update,
};

typedef struct {
  UIWindow **windows;

  GossipHandle gossip_handle;
} SystemUI;

void system_ui_internal_gossip_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);
void system_ui_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

// +UIWindow
UIWindow *ui_window_create(const char *title, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void ui_window_destroy(UIWindow *const window);

void ui_window_clear(UIWindow *const window, Glyph glyph);
void ui_window_glyph(UIWindow *const window, uint32_t x, uint32_t y, Glyph glyph);
void ui_window_scroll_x(UIWindow *const window, int32_t scroll);
void ui_window_scroll_y(UIWindow *const window, int32_t scroll);

void ui_window_update(UIWindow *const window, double delta);
void ui_window_draw(UIWindow *const window, AsciiBuffer *const screen);
// -UIWindow

SystemUI *system_ui_internal = NULL;
bool system_ui_start(void) {
  if (system_ui_internal) {
    return false;
  }

  system_ui_internal = calloc(1, sizeof(SystemUI));
  *system_ui_internal = (SystemUI){
    .windows = rectify_array_alloc(10, sizeof(UIWindow *)),
    .gossip_handle = gossip_subscribe("ui:*", &system_ui_internal_gossip_event, system_ui_internal),
  };

  screen_hook_render(&system_ui_internal_render_hook, system_ui_internal);

  return true;
}

void system_ui_stop(void) {
  if (!system_ui_internal) {
    return;
  }

  gossip_unsubscribe(system_ui_internal->gossip_handle);

  screen_unhook_render(&system_ui_internal_render_hook, system_ui_internal);

  for (uint32_t t = 0; t < rectify_array_size(system_ui_internal->windows); t++) {
    ui_window_destroy(system_ui_internal->windows[t]);
  }
  rectify_array_free(system_ui_internal->windows);

  free(system_ui_internal);
  system_ui_internal = NULL;
}

void system_ui_update(void) {
  if (!system_ui_internal) {
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(system_ui_internal->windows); t++) {
    ui_window_update(system_ui_internal->windows[t], 1.0 / (double)system_ui.frames);
  }
}

void system_ui_internal_gossip_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  if (strncmp(id, "window_create", 128) == 0) {
    UIEventCreateWindow *event = (UIEventCreateWindow *)userdata;

    UIWindow *window = ui_window_create(event->title, event->x, event->y, event->width, event->height);
    system_ui_internal->windows = rectify_array_push(system_ui_internal->windows, (void *)&window);

    gossip_emit("ui:window_created", sizeof(uintptr_t), &(uintptr_t)window);
  } else if (strncmp(id, "window_destroy", 128) == 0) {
    UIWindow *window = (UIWindow *)*(uintptr_t *)userdata;

    for (uint32_t t = 0; t < rectify_array_size(system_ui_internal->windows); t++) {
      if (system_ui_internal->windows[t]->handle == window->handle) {
        system_ui_internal->windows = rectify_array_delete(system_ui_internal->windows, t);
        break;
      }
    }

    ui_window_destroy(window);
  } else if (strncmp(id, "window_glyph", 128) == 0) {
    UIEventWindowGlyph *event = (UIEventWindowGlyph *)userdata;
    ui_window_glyph(event->window, event->x, event->y, event->glyph);
  }
}

void system_ui_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  for (uint32_t t = 0; t < rectify_array_size(system_ui_internal->windows); t++) {
    ui_window_draw(system_ui_internal->windows[t], screen);
  }
}
