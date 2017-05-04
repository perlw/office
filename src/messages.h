#pragma once

typedef enum {
  MSG_GAME_INIT = 0x001,
  MSG_GAME_KILL,

  MSG_SCENE_PREV,
  MSG_SCENE_NEXT,

  MSG_SOUND_PLAY_TAP,

  MSG_INPUT_KEYBOARD,
} Messages;
