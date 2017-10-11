#include <stdio.h>
#include <string.h>

#define USE_KRONOS
#define USE_PICASSO
#include "bedrock/bedrock.h"

#define USE_CONFIG
#define USE_MESSAGES
#include "main.h"

bool system_input_start(void);
void system_input_stop(void);
void system_input_message(uint32_t id, RectifyMap *const map);

KronosSystem system_input = {
  .name = "input",
  .frames = 1,
  .prevent_stop = true,
  .autostart = true,
  .start = &system_input_start,
  .stop = &system_input_stop,
  .update = NULL,
  .message = &system_input_message,
};

void system_input_internal_keyboard_callback(const PicassoWindowKeyboardEvent *event);
void system_input_internal_mousemove_callback(const PicassoWindowMouseEvent *event);
void system_input_internal_click_callback(const PicassoWindowMouseEvent *event);
void system_input_internal_mousescroll_callback(const PicassoWindowMouseScrollEvent *event);

typedef struct {
  uint32_t key;
  char *action;
} SystemInputKeyBind;

typedef struct {
  SystemInputKeyBind *keybinds;
} SystemInput;

SystemInput *system_input_internal = NULL;
bool system_input_start(void) {
  if (system_input_internal) {
    return false;
  }

  system_input_internal = calloc(1, sizeof(SystemInput));
  system_input_internal->keybinds = rectify_array_alloc(10, sizeof(SystemInputKeyBind));

  return true;
}

void system_input_stop(void) {
  if (!system_input_internal) {
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(system_input_internal->keybinds); t++) {
    free(system_input_internal->keybinds[t].action);
  }
  rectify_array_free((void **)&system_input_internal->keybinds);

  free(system_input_internal);
  system_input_internal = NULL;
}

void system_input_message(uint32_t id, RectifyMap *const map) {
  if (!system_input_internal) {
    return;
  }

  switch (id) {
    case MSG_INPUT_BIND: {
      char *const action = rectify_map_get_string(map, "action");
      uint32_t key = rectify_map_get_uint(map, "key");

      system_input_internal->keybinds = rectify_array_push(system_input_internal->keybinds, &(SystemInputKeyBind){
                                                                                              .key = key,
                                                                                              .action = rectify_memory_alloc_copy(action, sizeof(char) * (strnlen(action, 128) + 1)),
                                                                                            });
      break;
    }
  }
}

void system_input_bind_input(PicassoWindow *window) {
  if (!system_input_internal) {
    return;
  }

  picasso_window_keyboard_callback(window, &system_input_internal_keyboard_callback);
  picasso_window_mouse_move_callback(window, &system_input_internal_mousemove_callback);
  picasso_window_mouse_button_callback(window, &system_input_internal_click_callback);
  picasso_window_mouse_scroll_callback(window, &system_input_internal_mousescroll_callback);
}

void system_input_internal_keyboard_callback(const PicassoWindowKeyboardEvent *event) {
  for (uint32_t t = 0; t < rectify_array_size(system_input_internal->keybinds); t++) {
    if (system_input_internal->keybinds[t].key == event->key) {
      printf("Input: Triggering bind %s\n", system_input_internal->keybinds[t].action);

      RectifyMap *map = rectify_map_create();
      rectify_map_set_string(map, "action", system_input_internal->keybinds[t].action);
      rectify_map_set_bool(map, "pressed", event->pressed);
      rectify_map_set_bool(map, "released", event->released);
      kronos_emit(MSG_INPUT_ACTION, map);
      return;
    }
  }

  RectifyMap *map = rectify_map_create();
  rectify_map_set_uint(map, "key", event->key);
  rectify_map_set_bool(map, "pressed", event->pressed);
  rectify_map_set_bool(map, "released", event->released);
  kronos_emit(MSG_INPUT_KEY, map);

  if (event->released) {
    return;
  }

  switch (event->key) {
    case PICASSO_KEY_ESCAPE: {
      printf("\n-=ABORT=-\n\n");
      kronos_emit(MSG_GAME_KILL, NULL);
      return;
    }

    case PICASSO_KEY_LEFT:
      kronos_emit(MSG_SCENE_PREV, NULL);
      return;

    case PICASSO_KEY_RIGHT:
      kronos_emit(MSG_SCENE_NEXT, NULL);
      return;
  }
}

void system_input_internal_mousemove_callback(const PicassoWindowMouseEvent *event) {
  Config *const config = config_get();

  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  static uint32_t o_x = 0;
  static uint32_t o_y = 0;

  if (m_x != o_x || m_y != o_y) {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "x", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_x);
    rectify_map_set(map, "y", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_y);
    kronos_emit(MSG_INPUT_MOUSEMOVE, map);

    o_x = m_x;
    o_y = m_y;
  }
}

void system_input_internal_click_callback(const PicassoWindowMouseEvent *event) {
  Config *const config = config_get();

  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  static uint32_t o_x = 0;
  static uint32_t o_y = 0;

  if (m_x != o_x || m_y != o_y) {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "button", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & event->button);
    rectify_map_set(map, "x", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_x);
    rectify_map_set(map, "y", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_y);
    rectify_map_set(map, "pressed", RECTIFY_MAP_TYPE_BOOL, sizeof(bool), (bool *const) & event->pressed);
    rectify_map_set(map, "released", RECTIFY_MAP_TYPE_BOOL, sizeof(bool), (bool *const) & event->released);
    kronos_emit(MSG_INPUT_CLICK, map);

    o_x = m_x;
    o_y = m_y;
  }
}

void system_input_internal_mousescroll_callback(const PicassoWindowMouseScrollEvent *event) {
  Config *const config = config_get();

  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  int32_t scroll_x = (int32_t)(event->offset_x < 0.0 ? event->offset_x - 0.5 : event->offset_x + 0.5);
  int32_t scroll_y = (int32_t)(event->offset_y < 0.0 ? event->offset_y - 0.5 : event->offset_y + 0.5);

  RectifyMap *map = rectify_map_create();
  rectify_map_set(map, "x", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_x);
  rectify_map_set(map, "y", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_y);
  rectify_map_set(map, "scroll_x", RECTIFY_MAP_TYPE_INT, sizeof(int32_t), (int32_t *const) & scroll_x);
  rectify_map_set(map, "scroll_y", RECTIFY_MAP_TYPE_INT, sizeof(int32_t), (int32_t *const) & scroll_y);
  kronos_emit(MSG_INPUT_SCROLL, map);
}
