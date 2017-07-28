#include <stdio.h>

#include "bedrock/bedrock.h"

#include "config.h"

bool system_input_start(void);
void system_input_stop(void);
void system_input_update(void);

KronosSystem system_input = {
  .name = "input",
  .frames = 1,
  .start = &system_input_start,
  .stop = &system_input_stop,
  .update = &system_input_update,
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

void system_input_update(void) {
  if (!system_input_internal) {
    return;
  }
}

void system_input_internal_keyboard_callback(const PicassoWindowKeyboardEvent *event) {
  if (event->key == PICASSO_KEY_F12) {
    printf("\n-=ABORT=-\n\n");
    gossip_emit("game:kill", 0, NULL);
    return;
  }

  //gossip_emit("input:keyboard", sizeof(PicassoWindowKeyboardEvent), (void *)event);
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
