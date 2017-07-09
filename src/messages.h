#pragma once

typedef enum {
  MSG_GAME = GOSSIP_GROUP_ALL + 1,
  MSG_SYSTEM,
  MSG_SCENE,
  MSG_SCENE_EVENT,
  MSG_SOUND,
  MSG_INPUT,
  MSG_UI_WINDOW,
  MSG_UI_WIDGET,
} MessageGroups;

typedef enum {
  MSG_GAME_INIT = 0x001,
  MSG_GAME_KILL,

  MSG_SYSTEM_UPDATE,
  MSG_SYSTEM_DRAW_LAYER0,
  MSG_SYSTEM_DRAW,

  MSG_SCENE_PREV,
  MSG_SCENE_NEXT,
  MSG_SCENE_CHANGED,

  MSG_SOUND_PLAY_TAP,
  MSG_SOUND_PLAY_BOOM,
  MSG_SOUND_PLAY_SONG,
  MSG_SOUND_STOP_SONG,
  MSG_SOUND_SPECTRUM,

  MSG_INPUT_KEYBOARD,
  MSG_INPUT_MOUSE,
} Messages;
