#include <assert.h>

#include "bedrock/bedrock.h"

#include "config.h"
#include "messages.h"

bool system_ui_start(void);
void system_ui_stop(void);
void system_ui_update(double delta);
void system_ui_message(uint32_t id, RectifyMap *const map);

KronosSystem system_ui = {
  .name = "ui",
  .frames = 30,
  .start = &system_ui_start,
  .stop = &system_ui_stop,
  .update = &system_ui_update,
  .message = &system_ui_message,
};

typedef struct {
  int dummy;
} SystemUI;

SystemUI *system_ui_internal = NULL;
bool system_ui_start(void) {
  if (system_ui_internal) {
    return false;
  }

  system_ui_internal = calloc(1, sizeof(SystemUI));

  return true;
}

void system_ui_stop(void) {
  if (!system_ui_internal) {
    return;
  }

  free(system_ui_internal);
  system_ui_internal = NULL;
}

void system_ui_update(double delta) {
  if (!system_ui_internal) {
    return;
  }
}

void system_ui_message(uint32_t id, RectifyMap *const map) {
  if (!system_ui_internal) {
    return;
  }

  /*
  switch (id) {
    case MSG_SOUND_PLAY:
      break;
  }
  */
}
