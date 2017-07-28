#pragma once

typedef enum {
  MSG_GAME_INIT = 1,
  MSG_GAME_KILL,

  MSG_SCENE_SETUP,
  MSG_SCENE_TEARDOWN,
  MSG_SCENE_CHANGED,

  MSG_SOUND_PLAY,
  MSG_SOUND_SPECTRUM,
} Messages;
