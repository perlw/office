#include <assert.h>
#include <stdio.h>
#include <string.h>

#define USE_KRONOS
#include "bedrock/bedrock.h"

#define USE_MESSAGES
#define USE_SYSTEMS
#include "main.h"

typedef struct {
  int dummy;
} Systems;

Systems *systems_start(void);
void systems_stop(void **systems);
void systems_message(Systems *systems, uint32_t id, RectifyMap *const map);

KronosSystem systems = {
  .name = "systems",
  .frames = 0,
  .prevent_stop = true,
  .autostart = true,
  .start = &systems_start,
  .stop = &systems_stop,
  .update = NULL,
  .message = &systems_message,
};

void systems_internal_start(const char *name);
void systems_internal_stop(const char *name);

Systems *systems_start(void) {
  Systems *systems = calloc(1, sizeof(Systems));

  kronos_register(&system_debug);
  //kronos_register(&system_input);
  //kronos_register(&system_game);
  kronos_register(&system_window);
  kronos_register(&system_sound);
  kronos_register(&system_lua_bridge);
  kronos_register(&system_ui);

  return systems;
}

void systems_stop(Systems **systems) {
  Systems *ptr = *systems;
  assert(ptr && systems);

  free(ptr);
  *systems = NULL;
}

void systems_message(Systems *systems, uint32_t id, RectifyMap *const map) {
  assert(systems);

  switch (id) {
    case MSG_SYSTEM_START: {
      char *const name = rectify_map_get_string(map, "system");
      if (!name) {
        return;
      }
      systems_internal_start(name);
      break;
    }

    case MSG_SYSTEM_STOP: {
      char *const name = rectify_map_get_string(map, "system");
      if (!name) {
        return;
      }
      systems_internal_stop(name);
      break;
    }
  }
}

void systems_internal_start(const char *system) {
  printf("System: attempting to spin up \"%s\"...\n", system);
  KronosResult result = kronos_start_system(system);
  if (result == KRONOS_OK) {
    RectifyMap *map = rectify_map_create();
    rectify_map_set_string(map, "system", (char *const)system);
    kronos_emit(MSG_SYSTEM_SPUN_UP, map);

    printf("System: \"%s\" spun up!\n", system);
  } else {
    printf("System: \"%s\" %s\n", system, (result == KRONOS_SYSTEM_NOT_FOUND ? "does not exist" : "failed to spin up"));
  }
}

void systems_internal_stop(const char *system) {
  printf("Systems: attempting to stop \"%s\"\n", system);
  KronosResult result = kronos_stop_system(system);
  if (result == KRONOS_OK) {
    RectifyMap *map = rectify_map_create();
    rectify_map_set_string(map, "system", (char *const)system);
    kronos_emit(MSG_SYSTEM_SHUT_DOWN, map);

    printf("System: \"%s\" shut down!\n", system);
  } else {
    printf("System: \"%s\" %s\n", system, (result == KRONOS_SYSTEM_NOT_FOUND ? "does not exist" : "stop prevented"));
  }
}
