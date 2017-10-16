#include <assert.h>
#include <stdio.h>
#include <string.h>

#define USE_KRONOS
#include "bedrock/bedrock.h"

#define USE_CONFIG
#define USE_MESSAGES
#include "main.h"

typedef struct {
  uint32_t key;
  char *action;
} SystemInputKeyBind;

typedef struct {
  SystemInputKeyBind *keybinds;
} SystemInput;

SystemInput *system_input_start(void);
void system_input_stop(void **system);
void system_input_update(SystemInput *system, double delta);
void system_input_message(SystemInput *system, uint32_t id, RectifyMap *const map);

KronosSystem system_input = {
  .name = "input",
  .frames = 0,
  .autostart = true,
  .start = &system_input_start,
  .stop = &system_input_stop,
  .update = &system_input_update,
  .message = &system_input_message,
};

SystemInput *system_input_start(void) {
  Config *const config = config_get();

  SystemInput *system = calloc(1, sizeof(SystemInput));
  *system = (SystemInput){
    .keybinds = rectify_array_alloc(10, sizeof(SystemInputKeyBind)),
  };

  return system;
}

void system_input_stop(void **system) {
  SystemInput *ptr = *system;
  assert(ptr && system);

  for (uint32_t t = 0; t < rectify_array_size(ptr->keybinds); t++) {
    free(ptr->keybinds[t].action);
  }
  rectify_array_free((void **)&ptr->keybinds);

  free(ptr);
  *system = NULL;
}

void system_input_update(SystemInput *system, double delta) {
  assert(system);
}

void system_input_message(SystemInput *system, uint32_t id, RectifyMap *const map) {
  assert(system);

  switch (id) {
    case MSG_INPUT_KEY: {
      uint32_t key = rectify_map_get_uint(map, "key");
      bool pressed = rectify_map_get_bool(map, "pressed");
      bool released = rectify_map_get_bool(map, "released");

      for (uint32_t t = 0; t < rectify_array_size(system->keybinds); t++) {
        if (system->keybinds[t].key == key) {
          printf("Input: Triggering bind %s\n", system->keybinds[t].action);

          RectifyMap *map = rectify_map_create();
          rectify_map_set_string(map, "action", system->keybinds[t].action);
          rectify_map_set_bool(map, "pressed", pressed);
          rectify_map_set_bool(map, "released", released);
          kronos_emit(MSG_INPUT_ACTION, map);
          return;
        }
      }
      break;
    }

    case MSG_INPUT_BIND: {
      char *const action = rectify_map_get_string(map, "action");
      uint32_t key = rectify_map_get_uint(map, "key");

      system->keybinds = rectify_array_push(system->keybinds, &(SystemInputKeyBind){
                                                                .key = key,
                                                                .action = rectify_memory_alloc_copy(action, sizeof(char) * (strnlen(action, 128) + 1)),
                                                              });
      break;
    }
  }
}
