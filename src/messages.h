#pragma once

#include <stdio.h>

#define FOREACH_MESSAGE(MSG)      \
  MSG(MSG_SYSTEM_START)           \
  MSG(MSG_SYSTEM_STOP)            \
                                  \
  MSG(MSG_GAME_INIT)              \
  MSG(MSG_GAME_KILL)              \
                                  \
  MSG(MSG_INPUT_BIND)             \
  MSG(MSG_INPUT_ACTION)           \
  MSG(MSG_INPUT_KEY)              \
  MSG(MSG_INPUT_MOUSEMOVE)        \
  MSG(MSG_INPUT_CLICK)            \
  MSG(MSG_INPUT_SCROLL)           \
                                  \
  MSG(MSG_SCENE_GOTO)             \
  MSG(MSG_SCENE_PREV)             \
  MSG(MSG_SCENE_NEXT)             \
  MSG(MSG_SCENE_SETUP)            \
  MSG(MSG_SCENE_TEARDOWN)         \
  MSG(MSG_SCENE_CHANGED)          \
                                  \
  MSG(MSG_SOUND_PLAY)             \
  MSG(MSG_SOUND_PLAY_SONG)        \
  MSG(MSG_SOUND_STOP_SONG)        \
  MSG(MSG_SOUND_SPECTRUM)         \
                                  \
  MSG(MSG_PLAYER_MOVE_UP_LEFT)    \
  MSG(MSG_PLAYER_MOVE_UP)         \
  MSG(MSG_PLAYER_MOVE_UP_RIGHT)   \
  MSG(MSG_PLAYER_MOVE_LEFT)       \
  MSG(MSG_PLAYER_MOVE_RIGHT)      \
  MSG(MSG_PLAYER_MOVE_DOWN_LEFT)  \
  MSG(MSG_PLAYER_MOVE_DOWN)       \
  MSG(MSG_PLAYER_MOVE_DOWN_RIGHT) \
                                  \
  MSG(MSG_MATERIALS_LOAD)         \
  MSG(MSG_MATERIALS_LOADED)       \
  MSG(MSG_MATERIAL_REGISTER)      \
                                  \
  MSG(MSG_DEBUG_TEST)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum {
  FOREACH_MESSAGE(GENERATE_ENUM)
} Messages;

static char *MSG_NAMES[] = {
  FOREACH_MESSAGE(GENERATE_STRING)
    NULL,
};
