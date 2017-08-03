#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "messages.h"
#include "system_debug.h"
#include "system_game.h"
#include "system_input.h"
#include "system_lua_bridge.h"
#include "system_sound.h"
//#include "system_ui.h"

bool systems_start(void);
void systems_stop(void);
void systems_message(uint32_t id, RectifyMap *const map);

KronosSystem systems = {
  .name = "systems",
  .frames = 0,
  .prevent_stop = true,
  .start = &systems_start,
  .stop = &systems_stop,
  .update = NULL,
  .message = &systems_message,
};

typedef struct {
  int dummy;
} Systems;

Systems *systems_internal;

void systems_internal_start(const char *system);
void systems_internal_stop(const char *system);

bool systems_start(void) {
  if (systems_internal) {
    return false;
  }

  systems_internal = calloc(1, sizeof(Systems));

  kronos_register(&system_debug);
  kronos_register(&system_input);
  kronos_register(&system_game);
  kronos_register(&system_sound);
  kronos_register(&system_lua_bridge);
  //kronos_register(&system_ui);

  return true;
}

void systems_stop(void) {
  if (!systems_internal) {
    return;
  }
  free(systems_internal);
}

void systems_message(uint32_t id, RectifyMap *const map) {
  if (!systems_internal) {
    return;
  }

  char *const system = (char *const)rectify_map_get(map, "system");
  if (!system) {
    return;
  }
  switch (id) {
    case MSG_SYSTEM_START:
      systems_internal_start(system);
      break;

    case MSG_SYSTEM_STOP:
      systems_internal_stop(system);
      break;
  }
}

void systems_internal_start(const char *system) {
  printf("System: attempting to spin up \"%s\"...\n", system);
  KronosResult result = kronos_start_system(system);
  if (result == KRONOS_OK) {
    printf("System: \"%s\" spun up!\n", system);
  } else {
    printf("System: \"%s\" %s\n", system, (result == KRONOS_SYSTEM_NOT_FOUND ? "does not exist" : "failed to spin up"));
  }
}

void systems_internal_stop(const char *system) {
  printf("Systems: attempting to stop \"%s\"\n", system);
  KronosResult result = kronos_stop_system(system);
  if (result == KRONOS_OK) {
    printf("System: \"%s\" shut down!\n", system);
  } else {
    printf("System: \"%s\" %s\n", system, (result == KRONOS_SYSTEM_NOT_FOUND ? "does not exist" : "stop prevented"));
  }
}
