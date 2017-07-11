#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "input.h"
#include "messages.h"

ActionRef *action_refs;

uintmax_t num_bindings = 0;
InputActionBinding *input_bindings = NULL;
InputActionCallback main_callback = NULL;
void *main_callback_userdata = NULL;

void input_init() {
  action_refs = rectify_array_alloc(10, sizeof(ActionRef));
}

void input_kill() {
  for (uintmax_t t = 0; t < rectify_array_size(action_refs); t++) {
    if (action_refs[t].action) {
      free(action_refs[t].action);
    }
  }
  rectify_array_free(action_refs);

  for (uintmax_t t = 0; t < num_bindings; t++) {
    free(input_bindings[t].action);
  }
  if (input_bindings) {
    free(input_bindings);
  }
}

void input_keyboard_callback(const PicassoWindowKeyboardEvent *event) {
  gossip_emit(MSG_INPUT, MSG_INPUT_KEYBOARD, NULL, (void *)event);

  if (event->released) {
    return;
  }

  for (uintmax_t t = 0; t < num_bindings; t++) {
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

  if (strcmp(binding->action, "close") == 0) {
    gossip_emit(MSG_GAME, MSG_GAME_KILL, NULL, NULL);
    return;
  }
  if (strcmp(binding->action, "prev_scene") == 0) {
    gossip_emit(MSG_SCENE, MSG_SCENE_PREV, NULL, NULL);
    return;
  }
  if (strcmp(binding->action, "next_scene") == 0) {
    gossip_emit(MSG_SCENE, MSG_SCENE_NEXT, NULL, NULL);
    return;
  }

  /*for (uintmax_t t = 0; t < rectify_array_size(action_refs); t++) {
    if (strcmp(action_refs[t].action, binding->action) == 0) {
      muse_call_funcref((Muse *)userdata, action_refs[t].ref, 0, NULL, 0, NULL);
    }
  }*/
}

void lua_action(Muse *const muse, uintmax_t num_arguments, const MuseArgument *const arguments, const void *const userdata) {
  char *action = (char *)arguments[0].argument;
  MuseFunctionRef ref = *(MuseFunctionRef *)arguments[1].argument;

  ActionRef action_ref = {
    .action = rectify_memory_alloc_copy(action, sizeof(char) * (strlen(action) + 1)),
    .ref = ref,
  };
  action_refs = rectify_array_push(action_refs, &action_ref);
}

void input_action_callback(InputActionCallback callback, void *userdata) {
  assert(callback);

  main_callback = callback;
  main_callback_userdata = userdata;
}

void input_action_add_binding(InputActionBinding *binding) {
  assert(binding);
  if (!input_bindings) {
    input_bindings = calloc(1, sizeof(InputActionBinding));
  }

  uintmax_t length = strlen(binding->action) + 1;
  num_bindings++;
  input_bindings = realloc(input_bindings, num_bindings * sizeof(InputActionBinding));
  input_bindings[num_bindings - 1] = (InputActionBinding){
    .action = rectify_memory_alloc_copy(binding->action, sizeof(char) * length),
    .key = binding->key,
  };
}
