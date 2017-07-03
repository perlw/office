#pragma once

typedef enum {
  MSG_GAME = GOSSIP_GROUP_ALL + 1,
  MSG_SCENE,
  MSG_SOUND,
  MSG_INPUT,
} MessageGroups;

typedef enum {
  MSG_GAME_INIT = 0x001,
  MSG_GAME_KILL,

  MSG_SCENE_PREV,
  MSG_SCENE_NEXT,
  MSG_SCENE_CHANGED,

  MSG_SOUND_PLAY_TAP,
  MSG_SOUND_PLAY_SONG,
  MSG_SOUND_STOP_SONG,
  MSG_SOUND_SPECTRUM,

  MSG_INPUT_KEYBOARD,
  MSG_INPUT_MOUSE,
} Messages;
