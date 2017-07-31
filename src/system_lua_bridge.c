#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "messages.h"

bool system_lua_bridge_start(void);
void system_lua_bridge_stop(void);
void system_lua_bridge_message(uint32_t id, RectifyMap *const map);

KronosSystem system_lua_bridge = {
  .name = "lua_bridge",
  .frames = 1,
  .autostart = true,
  .start = &system_lua_bridge_start,
  .stop = &system_lua_bridge_stop,
  .update = NULL,
  .message = &system_lua_bridge_message,
};

typedef struct {
  int dummy;
} SystemLuaBridge;

SystemLuaBridge *system_lua_bridge_internal = NULL;
bool system_lua_bridge_start(void) {
  if (system_lua_bridge_internal) {
    return false;
  }

  system_lua_bridge_internal = calloc(1, sizeof(SystemLuaBridge));

  return true;
}

void system_lua_bridge_stop(void) {
  if (!system_lua_bridge_internal) {
    return;
  }

  free(system_lua_bridge_internal);
  system_lua_bridge_internal = NULL;
}

void system_lua_bridge_message(uint32_t id, RectifyMap *const map) {
  if (!system_lua_bridge_internal) {
    return;
  }

  /*
  switch (id) {
    case MSG_SOUND_PLAY: {
      break;
    }
  }
*/
}
