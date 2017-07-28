#include <stdio.h>

#include "bedrock/bedrock.h"

#include "config.h"
#include "messages.h"

bool system_game_start(void);
void system_game_stop(void);
void system_game_update(void);
void system_game_message(uint32_t id, RectifyMap *const map);

KronosSystem system_game = {
  .name = "game",
  .frames = 1,
  .prevent_stop = true,
  .start = &system_game_start,
  .stop = &system_game_stop,
  .update = &system_game_update,
  .message = &system_game_message,
};

typedef struct {
  bool should_kill;
} SystemGame;

SystemGame *system_game_internal = NULL;
bool system_game_start(void) {
  if (system_game_internal) {
    return false;
  }

  system_game_internal = calloc(1, sizeof(SystemGame));

  return true;
}

void system_game_stop(void) {
  if (!system_game_internal) {
    return;
  }

  free(system_game_internal);
  system_game_internal = NULL;
}

void system_game_update(void) {
  if (!system_game_internal) {
    return;
  }
}

void system_game_message(uint32_t id, RectifyMap *const map) {
  if (!system_game_internal) {
    return;
  }

  switch (id) {
    case MSG_GAME_KILL:
      system_game_internal->should_kill = true;
      break;
  }
}

bool system_game_should_kill(void) {
  if (!system_game_internal) {
    return true;
  }

  return system_game_internal->should_kill;
}
