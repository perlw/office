#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "input.h"
#include "lua_bridge.h"

InputActionBinding *input_bindings = NULL;
InputActionRef *input_action_refs = NULL;

void input_init() {
  input_bindings = rectify_array_alloc(10, sizeof(InputActionBinding));
  input_action_refs = rectify_array_alloc(10, sizeof(InputActionRef));
}

void input_kill() {
  for (uint32_t t = 0; t < rectify_array_size(input_bindings); t++) {
    free(input_bindings[t].action);
  }
  rectify_array_free(input_bindings);

  for (uint32_t t = 0; t < rectify_array_size(input_action_refs); t++) {
    free(input_action_refs[t].action);
  }
  rectify_array_free(input_action_refs);
}

void input_keyboard_callback(const PicassoWindowKeyboardEvent *event) {
  gossip_emit("input:keyboard", (void *)event);

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
    for (uint32_t t = 0; t < rectify_array_size(input_action_refs); t++) {
      if (strncmp(input_action_refs[t].action, key_bind->action, 128) == 0) {
        gossip_emit("lua_bridge:action", &input_action_refs[t]);
      }
    }
  }
}

void input_mousemove_callback(const PicassoWindowMouseEvent *event) {
  const Config *const config = config_get();
  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);

  gossip_emit("input:mousemove", &(InputMouseMoveEvent){
                                   .x = m_x,
                                   .y = m_y,
                                 });
}

void input_click_callback(const PicassoWindowMouseEvent *event) {
  const Config *const config = config_get();
  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);

  gossip_emit("input:click", &(InputClickEvent){
                               .button = event->button,
                               .x = m_x,
                               .y = m_y,
                               .pressed = event->pressed,
                               .released = event->released,
                             });
}

void input_action_add_binding(InputActionBinding *binding) {
  assert(binding);

  uint32_t length = (uint32_t)strlen(binding->action) + 1;
  input_bindings = rectify_array_push(input_bindings, &(InputActionBinding){
                                                        .action = rectify_memory_alloc_copy(binding->action, sizeof(char) * length), .key = binding->key,
                                                      });
}

void input_action_add_action(InputActionRef *action_ref) {
  assert(action_ref);

  uint32_t length = (uint32_t)strlen(action_ref->action) + 1;
  input_action_refs = rectify_array_push(input_action_refs, &(InputActionRef){
                                                              .action = rectify_memory_alloc_copy(action_ref->action, sizeof(char) * length), .ref = action_ref->ref,
                                                            });
}
