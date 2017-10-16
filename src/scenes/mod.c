#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define USE_KRONOS
#include "bedrock/bedrock.h"

#define USE_MESSAGES
#define USE_SCENES
#include "main.h"

void *scenes_start(void);
void scenes_stop(void **system);
void scenes_update(void *system, double delta);
void scenes_message(void *system, uint32_t id, RectifyMap *const map);

KronosSystem scenes = {
  .name = "scenes",
  .frames = 30,
  .prevent_stop = true,
  .autostart = true,
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

void scenes_internal_goto(void *system, uint32_t index);

void *scenes_start(void) {
  Scenes *scenes_internal = calloc(1, sizeof(Scenes));

  scenes_internal->current = -1;
  scenes_internal->systems = rectify_array_alloc(10, sizeof(KronosSystem));
  scenes_internal->systems = rectify_array_push(scenes_internal->systems, &scene_test);
  scenes_internal->systems = rectify_array_push(scenes_internal->systems, &scene_drips);
  scenes_internal->systems = rectify_array_push(scenes_internal->systems, &scene_sound_test);
  scenes_internal->systems = rectify_array_push(scenes_internal->systems, &scene_game);
  scenes_internal->systems = rectify_array_push(scenes_internal->systems, &scene_world_edit);

  return scenes_internal;
}

void scenes_stop(void **system) {
  Scenes *scenes_internal = *system;
  assert(system && scenes_internal);

  rectify_array_free((void **)&scenes_internal->systems);
  free(scenes_internal);
  *system = NULL;
}

void scenes_update(void *system, double delta) {
  assert(system);
}

void scenes_message(void *system, uint32_t id, RectifyMap *const map) {
  assert(system);
  Scenes *scenes_internal = system;

  switch (id) {
    case MSG_GAME_INIT:
      for (uint32_t t = 0; t < rectify_array_size(scenes_internal->systems); t++) {
        kronos_register(&scenes_internal->systems[t]);
      }
      break;

    case MSG_SCENE_GOTO: {
      char *const scene = (char *const)rectify_map_get(map, "scene");
      if (!scene) {
        return;
      }

      for (uint32_t t = 0; t < rectify_array_size(scenes_internal->systems); t++) {
        if (strncmp(scenes_internal->systems[t].name, scene, 128) == 0) {
          scenes_internal_goto(scenes_internal, t);
          break;
        }
      }
      break;
    }

    case MSG_SCENE_PREV:
      if (scenes_internal->current > 0) {
        scenes_internal_goto(scenes_internal, (uint32_t)scenes_internal->current - 1);
      }
      break;

    case MSG_SCENE_NEXT:
      if (scenes_internal->current < rectify_array_size(scenes_internal->systems) - 1) {
        scenes_internal_goto(scenes_internal, (uint32_t)scenes_internal->current + 1);
      }
      break;
  }
}

void scenes_internal_goto(void *system, uint32_t index) {
  assert(system);
  Scenes *scenes_internal = system;

  KronosSystem *const current = (scenes_internal->current < 0 ? NULL : &scenes_internal->systems[scenes_internal->current]);
  KronosSystem *const target = &scenes_internal->systems[index];

  if (current) {
    {
      RectifyMap *map = rectify_map_create();
      rectify_map_set(map, "scene", RECTIFY_MAP_TYPE_STRING, sizeof(char) * (strnlen(current->name, 128) + 1), current->name);
      kronos_emit(MSG_SCENE_TEARDOWN, map);
    }
    {
      RectifyMap *map = rectify_map_create();
      rectify_map_set(map, "system", RECTIFY_MAP_TYPE_STRING, sizeof(char) * (strnlen(current->name, 128) + 1), current->name);
      kronos_post("systems", MSG_SYSTEM_STOP, map);
    }
  }

  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "system", RECTIFY_MAP_TYPE_STRING, sizeof(char) * (strnlen(target->name, 128) + 1), target->name);
    kronos_post("systems", MSG_SYSTEM_START, map);
  }
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "scene", RECTIFY_MAP_TYPE_STRING, sizeof(char) * (strnlen(target->name, 128) + 1), target->name);
    kronos_emit(MSG_SCENE_SETUP, map);
  }
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "scene", RECTIFY_MAP_TYPE_STRING, sizeof(char) * (strnlen(target->name, 128) + 1), target->name);
    kronos_emit(MSG_SCENE_CHANGED, map);
  }

  scenes_internal->current = (int32_t)index;
}