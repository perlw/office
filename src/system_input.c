#include <stdio.h>

#include "bedrock/bedrock.h"

#include "config.h"
#include "messages.h"

bool system_input_start(void);
void system_input_stop(void);

KronosSystem system_input = {
  .name = "input",
  .frames = 1,
  .prevent_stop = true,
  .autostart = true,
  .start = &system_input_start,
  .stop = &system_input_stop,
  .update = NULL,
  .message = NULL,
};

void system_input_internal_keyboard_callback(const PicassoWindowKeyboardEvent *event);
void system_input_internal_mousemove_callback(const PicassoWindowMouseEvent *event);
void system_input_internal_click_callback(const PicassoWindowMouseEvent *event);
void system_input_internal_mousescroll_callback(const PicassoWindowMouseScrollEvent *event);

typedef struct {
  int dummy;
} SystemInput;

SystemInput *system_input_internal = NULL;
bool system_input_start(void) {
  if (system_input_internal) {
    return false;
  }

  system_input_internal = calloc(1, sizeof(SystemInput));

  picasso_window_keyboard_callback(&system_input_internal_keyboard_callback);
  picasso_window_mouse_move_callback(&system_input_internal_mousemove_callback);
  picasso_window_mouse_button_callback(&system_input_internal_click_callback);
  picasso_window_mouse_scroll_callback(&system_input_internal_mousescroll_callback);

  return true;
}

void system_input_stop(void) {
  if (!system_input_internal) {
    return;
  }

  free(system_input_internal);
  system_input_internal = NULL;
}

void system_input_internal_keyboard_callback(const PicassoWindowKeyboardEvent *event) {
  // Temp
  RectifyMap *map = rectify_map_create();
  rectify_map_set(map, "key", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t * const) & event->key);
  rectify_map_set(map, "pressed", RECTIFY_MAP_TYPE_BOOL, sizeof(bool), (bool *const) & event->pressed);
  rectify_map_set(map, "released", RECTIFY_MAP_TYPE_BOOL, sizeof(bool), (bool *const) & event->released);
  gossip_emit(MSG_INPUT_KEY, map);

  if (event->released) {
    return;
  }

  switch (event->key) {
    case PICASSO_KEY_ESCAPE: {
      printf("\n-=ABORT=-\n\n");
      gossip_emit(MSG_GAME_KILL, NULL);
      return;
    }

    case PICASSO_KEY_LEFT:
      gossip_emit(MSG_SCENE_PREV, NULL);
      return;

    case PICASSO_KEY_RIGHT:
      gossip_emit(MSG_SCENE_NEXT, NULL);
      return;

    case PICASSO_KEY_KP_1:
      gossip_emit(MSG_PLAYER_MOVE_DOWN_LEFT, NULL);
      return;

    case PICASSO_KEY_KP_2:
      gossip_emit(MSG_PLAYER_MOVE_DOWN, NULL);
      return;

    case PICASSO_KEY_KP_3:
      gossip_emit(MSG_PLAYER_MOVE_DOWN_RIGHT, NULL);
      return;

    case PICASSO_KEY_KP_4:
      gossip_emit(MSG_PLAYER_MOVE_LEFT, NULL);
      return;

    case PICASSO_KEY_KP_5:
      printf("SYSTEM_INPUT: Something should happen?\n");
      break;

    case PICASSO_KEY_KP_6:
      gossip_emit(MSG_PLAYER_MOVE_RIGHT, NULL);
      return;

    case PICASSO_KEY_KP_7:
      gossip_emit(MSG_PLAYER_MOVE_UP_LEFT, NULL);
      return;

    case PICASSO_KEY_KP_8:
      gossip_emit(MSG_PLAYER_MOVE_UP, NULL);
      return;

    case PICASSO_KEY_KP_9:
      gossip_emit(MSG_PLAYER_MOVE_UP_RIGHT, NULL);
      return;
  }
}

void system_input_internal_mousemove_callback(const PicassoWindowMouseEvent *event) {
  const Config *const config = config_get();

  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  static uint32_t o_x = 0;
  static uint32_t o_y = 0;

  if (m_x != o_x || m_y != o_y) {
    /*gossip_emit("input:mousemove", sizeof(InputMouseMoveEvent), &(InputMouseMoveEvent){
                                                                  .x = m_x, .y = m_y,
                                                                });*/
    o_x = m_x;
    o_y = m_y;
  }
}

void system_input_internal_click_callback(const PicassoWindowMouseEvent *event) {
  const Config *const config = config_get();
  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  static uint32_t o_x = 0;
  static uint32_t o_y = 0;

  if (m_x != o_x || m_y != o_y) {
    /*gossip_emit("input:click", sizeof(InputClickEvent), &(InputClickEvent){
                                                          .button = event->button, .x = m_x, .y = m_y, .pressed = event->pressed, .released = event->released,
                                                        });*/
    o_x = m_x;
    o_y = m_y;
  }
}

void system_input_internal_mousescroll_callback(const PicassoWindowMouseScrollEvent *event) {
  const Config *const config = config_get();
  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  int32_t o_x = (int32_t)(event->offset_x < 0.0 ? event->offset_x - 0.5 : event->offset_x + 0.5);
  int32_t o_y = (int32_t)(event->offset_y < 0.0 ? event->offset_y - 0.5 : event->offset_y + 0.5);
  /*gossip_emit("input:mousescroll", sizeof(InputMouseScrollEvent), &(InputMouseScrollEvent){
                                                                    .scroll_x = o_x, .scroll_y = o_y, .x = m_x, .y = m_y,
                                                                  });*/
}
