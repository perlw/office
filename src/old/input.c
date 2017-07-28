#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "input.h"

InputActionBinding *input_bindings = NULL;

void input_init(void) {
  input_bindings = rectify_array_alloc(10, sizeof(InputActionBinding));
}

void input_kill(void) {
  for (uint32_t t = 0; t < rectify_array_size(input_bindings); t++) {
    free(input_bindings[t].action);
  }
  rectify_array_free(input_bindings);
}

void input_keyboard_callback(const PicassoWindowKeyboardEvent *event) {
  gossip_emit("input:keyboard", sizeof(PicassoWindowKeyboardEvent), (void *)event);

  if (event->key == PICASSO_KEY_F12) {
    printf("\n-=ABORT=-\n\n");
    gossip_emit("game:kill", 0, NULL);
    return;
  }

  if (event->released) {
    return;
  }

  {
    InputActionBinding *key_bind = NULL;
    for (uint32_t t = 0; t < rectify_array_size(input_bindings); t++) {
      if (input_bindings[t].key == event->key) {
        key_bind = &input_bindings[t];
      }
    }
    if (!key_bind) {
      return;
    }

    printf("INPUT: %s\n", key_bind->action);
    gossip_emit(key_bind->action, 0, NULL);
  }
}

void input_mousemove_callback(const PicassoWindowMouseEvent *event) {
  const Config *const config = config_get();
  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  static uint32_t o_x = 0;
  static uint32_t o_y = 0;

  if (m_x != o_x || m_y != o_y) {
    gossip_emit("input:mousemove", sizeof(InputMouseMoveEvent), &(InputMouseMoveEvent){
                                                                  .x = m_x, .y = m_y,
                                                                });
    o_x = m_x;
    o_y = m_y;
  }
}

void input_click_callback(const PicassoWindowMouseEvent *event) {
  const Config *const config = config_get();
  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  static uint32_t o_x = 0;
  static uint32_t o_y = 0;

  if (m_x != o_x || m_y != o_y) {
    gossip_emit("input:click", sizeof(InputClickEvent), &(InputClickEvent){
                                                          .button = event->button, .x = m_x, .y = m_y, .pressed = event->pressed, .released = event->released,
                                                        });
    o_x = m_x;
    o_y = m_y;
  }
}

void input_mousescroll_callback(const PicassoWindowMouseScrollEvent *event) {
  const Config *const config = config_get();
  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  int32_t o_x = (int32_t)(event->offset_x < 0.0 ? event->offset_x - 0.5 : event->offset_x + 0.5);
  int32_t o_y = (int32_t)(event->offset_y < 0.0 ? event->offset_y - 0.5 : event->offset_y + 0.5);
  gossip_emit("input:mousescroll", sizeof(InputMouseScrollEvent), &(InputMouseScrollEvent){
                                                                    .scroll_x = o_x, .scroll_y = o_y, .x = m_x, .y = m_y,
                                                                  });
}

void input_action_add_binding(InputActionBinding *binding) {
  assert(binding);

  uint32_t length = (uint32_t)strlen(binding->action) + 1;
  input_bindings = rectify_array_push(input_bindings, &(InputActionBinding){
                                                        .action = rectify_memory_alloc_copy(binding->action, sizeof(char) * length), .key = binding->key,
                                                      });
}
