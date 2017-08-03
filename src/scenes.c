#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "messages.h"
#include "scene_drips.h"
#include "scene_game.h"
#include "scene_sound-test.h"
#include "scene_test.h"
#include "scene_world-edit.h"

bool scenes_start(void);
void scenes_stop(void);
void scenes_update(double delta);
void scenes_message(uint32_t id, RectifyMap *const map);

KronosSystem scenes = {
  .name = "scenes",
  .frames = 30,
  .prevent_stop = true,
  .start = &scenes_start,
  .stop = &scenes_stop,
  .update = &scenes_update,
  .message = &scenes_message,
};

typedef struct {
  uint32_t num_scenes;
  int32_t current;
  KronosSystem *systems;
} Scenes;

void scenes_internal_goto(uint32_t index);

Scenes *scenes_internal = NULL;
bool scenes_start(void) {
  if (scenes_internal) {
    return false;
  }

  scenes_internal = calloc(1, sizeof(Scenes));

  scenes_internal->current = -1;
  scenes_internal->systems = rectify_array_alloc(10, sizeof(KronosSystem));
  scenes_internal->systems = rectify_array_push(scenes_internal->systems, &scene_test);
  scenes_internal->systems = rectify_array_push(scenes_internal->systems, &scene_drips);
  scenes_internal->systems = rectify_array_push(scenes_internal->systems, &scene_sound_test);
  scenes_internal->systems = rectify_array_push(scenes_internal->systems, &scene_game);
  scenes_internal->systems = rectify_array_push(scenes_internal->systems, &scene_world_edit);

  return true;
}

void scenes_stop(void) {
  if (!scenes_internal) {
    return;
  }

  rectify_array_free(&scenes_internal->systems);
  free(scenes_internal);
  scenes_internal = NULL;
}

void scenes_update(double delta) {
  if (!scenes_internal) {
    return;
  }
}

void scenes_message(uint32_t id, RectifyMap *const map) {
  if (!scenes_internal) {
    return;
  }

  switch (id) {
    case MSG_GAME_INIT:
      for (uint32_t t = 0; t < rectify_array_size(scenes_internal->systems); t++) {
        kronos_register(&scenes_internal->systems[t]);
      }
      break;

    case MSG_SCENE_GOTO:
      char *const scene = (char *const)rectify_map_get(map, "scene");
      if (!scene) {
        return;
      }

      for (uint32_t t = 0; t < rectify_array_size(scenes_internal->systems); t++) {
        if (strncmp(scenes_internal->systems[t].name, scene, 128) == 0) {
          scenes_internal_goto(t);
          break;
        }
      }
      break;

    case MSG_SCENE_PREV:
      if (scenes_internal->current > 0) {
        scenes_internal_goto((uint32_t)scenes_internal->current - 1);
      }
      break;

    case MSG_SCENE_NEXT:
      if (scenes_internal->current < rectify_array_size(scenes_internal->systems) - 1) {
        scenes_internal_goto((uint32_t)scenes_internal->current + 1);
      }
      break;
  }
}

void scenes_internal_goto(uint32_t index) {
  if (!scenes_internal) {
    return;
  }

  KronosSystem *const current = (scenes_internal->current < 0 ? NULL : &scenes_internal->systems[scenes_internal->current]);
  KronosSystem *const target = &scenes_internal->systems[index];

  if (current) {
    {
      RectifyMap *map = rectify_map_create();
      rectify_map_set(map, "system", RECTIFY_MAP_TYPE_STRING, sizeof(char) * (strnlen(current->name, 128) + 1), current->name);
      gossip_post("systems", MSG_SCENE_TEARDOWN, map);
    }
    {
      RectifyMap *map = rectify_map_create();
      rectify_map_set(map, "system", RECTIFY_MAP_TYPE_STRING, sizeof(char) * (strnlen(current->name, 128) + 1), current->name);
      gossip_post("systems", MSG_SYSTEM_STOP, map);
    }
  }

  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "system", RECTIFY_MAP_TYPE_STRING, sizeof(char) * (strnlen(target->name, 128) + 1), target->name);
    gossip_post("systems", MSG_SYSTEM_START, map);
  }
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "scene", RECTIFY_MAP_TYPE_STRING, sizeof(char) * (strnlen(target->name, 128) + 1), target->name);
    gossip_emit(MSG_SCENE_SETUP, map);
  }
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "scene", RECTIFY_MAP_TYPE_STRING, sizeof(char) * (strnlen(target->name, 128) + 1), target->name);
    gossip_emit(MSG_SCENE_CHANGED, map);
  }

  scenes_internal->current = (int32_t)index;
}
