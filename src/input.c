#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lua.h"

#include "config.h"
#include "input.h"
#include "messages.h"

InputActionBinding *input_bindings = NULL;
InputActionRef *input_action_refs = NULL;
InputActionCallback main_callback = NULL;
void *main_callback_userdata = NULL;

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
  gossip_emit(MSG_INPUT, MSG_INPUT_KEYBOARD, NULL, (void *)event);

  if (event->released) {
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(input_bindings); t++) {
    if (input_bindings[t].key == event->key) {
      main_callback(&input_bindings[t], main_callback_userdata);
    }
  }
}

void input_mouse_callback(const PicassoWindowMouseEvent *event) {
  gossip_emit(MSG_INPUT, MSG_INPUT_MOUSE, NULL, (void *)event);
}

void input_action(InputActionBinding *binding, void *userdata) {
  printf("INPUT: %s\n", binding->action);

  if (strncmp(binding->action, "close", 128) == 0) {
    gossip_emit(MSG_GAME, MSG_GAME_KILL, NULL, NULL);
    return;
  }
  if (strncmp(binding->action, "prev_scene", 128) == 0) {
    gossip_emit(MSG_SCENE, MSG_SCENE_PREV, NULL, NULL);
    return;
  }
  if (strncmp(binding->action, "next_scene", 128) == 0) {
    gossip_emit(MSG_SCENE, MSG_SCENE_NEXT, NULL, NULL);
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(input_action_refs); t++) {
    if (strncmp(input_action_refs[t].action, binding->action, 128) == 0) {
      lua_State *state = (lua_State *)userdata;
      lua_rawgeti(state, LUA_REGISTRYINDEX, input_action_refs[t].ref);
      int result = lua_pcall(state, 0, 0, 0);
      if (result != LUA_OK) {
        const char *message = lua_tostring(state, -1);
        printf("LUA: %s: %s\n", __func__, message);
        lua_pop(state, 1);
      }
    }
  }
}

void input_action_callback(InputActionCallback callback, void *userdata) {
  assert(callback);

  main_callback = callback;
  main_callback_userdata = userdata;
}

void input_action_add_binding(InputActionBinding *binding) {
  assert(binding);

  uint32_t length = strlen(binding->action) + 1;
  input_bindings = rectify_array_push(input_bindings, &(InputActionBinding){
                                                        .action = rectify_memory_alloc_copy(binding->action, sizeof(char) * length), .key = binding->key,
                                                      });
}

void input_action_add_action(InputActionRef *action_ref) {
  assert(action_ref);

  uint32_t length = strlen(action_ref->action) + 1;
  input_action_refs = rectify_array_push(input_action_refs, &(InputActionRef){
                                                              .action = rectify_memory_alloc_copy(action_ref->action, sizeof(char) * length), .ref = action_ref->ref,
                                                            });
}
