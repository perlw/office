#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "../bedrock.h"

uintmax_t num_bindings = 0;
NeglectBinding *input_bindings;
NeglectCallback main_callback = NULL;

void keyboard_event(void *userdata) {
  assert(userdata);

  BedrockKeyboardEvent event = *(BedrockKeyboardEvent*)userdata;
  printf("KEYBOARD %d %s\n", event.scancode, (event.press ? "down" : "up"));

  if (event.release) {
    return;
  }

  for (uintmax_t t = 0; t < num_bindings; t++) {
    if (input_bindings[t].scancode == event.scancode) {
      if (input_bindings[t].callback) {
        input_bindings[t].callback(&input_bindings[t]);
      }

      main_callback(&input_bindings[t]);
    }
  }
}

void neglect_init(void) {
  input_bindings = calloc(1, sizeof(NeglectBinding));

  gossip_subscribe(GOSSIP_ID_INPUT_KEY, &keyboard_event);
}

void neglect_kill(void) {
  free(input_bindings);
}

void neglect_action_callback(NeglectCallback callback) {
  assert(callback);

  main_callback = callback;
}

void neglect_add_binding(NeglectBinding *binding) {
  num_bindings++;
  input_bindings = realloc(input_bindings, num_bindings * sizeof(NeglectBinding));
  memcpy(&input_bindings[num_bindings - 1], binding, sizeof(NeglectBinding));
}