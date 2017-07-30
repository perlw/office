#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "messages.h"
#include "scene_drips.h"
#include "scene_test.h"
//#include "scene_test.h"
//#include "scene_game.h"
//#include "scene_sound-test.h"
//#include "scene_world-edit.h"

bool scenes_start(void);
void scenes_stop(void);
void scenes_update(void);
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

bool scenes_running = false;
bool scenes_start(void) {
  if (scenes_running) {
    return false;
  }

  scenes_running = true;

  kronos_register(&scene_test);
  kronos_register(&scene_drips);

  return true;
}

void scenes_stop(void) {
  if (!scenes_running) {
    return;
  }
  scenes_running = false;
}

void scenes_update(void) {
  if (!scenes_running) {
    return;
  }
}

void scenes_message(uint32_t id, RectifyMap *const map) {
  if (!scenes_running) {
    return;
  }

  // TODO: Next/prev, stopping
  switch (id) {
    case MSG_SCENE_GOTO:
      char *const scene = (char *const)rectify_map_get(map, "scene");
      if (!scene) {
        return;
      }

      {
        RectifyMap *map = rectify_map_create();
        rectify_map_set(map, "system", sizeof(char) * (strnlen(scene, 128) + 1), scene);
        gossip_post("systems", MSG_SYSTEM_START, map);
      }
      {
        RectifyMap *map = rectify_map_create();
        rectify_map_set(map, "scene", sizeof(char) * (strnlen(scene, 128) + 1), scene);
        gossip_emit(MSG_SCENE_SETUP, map);
      }
      {
        RectifyMap *map = rectify_map_create();
        rectify_map_set(map, "scene", sizeof(char) * (strnlen(scene, 128) + 1), scene);
        gossip_emit(MSG_SCENE_CHANGED, map);
      }

      break;
    case MSG_SCENE_PREV:
      break;
    case MSG_SCENE_NEXT:
      break;
  }
}
/*
void scenes_internal_go(Scenes *scenes, uint32_t index) {
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "scene", sizeof(char) * (strnlen(scenes->current_scene->name, 128) + 1), scenes->current_scene->name);
    gossip_emit(MSG_SCENE_TEARDOWN, map);
  }
  scenes->current_scene->destroy(scenes->current_scene_data);
  scenes->current_scene = NULL;

  scenes->current_scene = &scenes->scenes[index];
  scenes->current_scene_data = scenes->current_scene->create();
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "scene", sizeof(char) * (strnlen(scenes->current_scene->name, 128) + 1), scenes->current_scene->name);
    gossip_emit(MSG_SCENE_SETUP, map);
  }

  printf("SCENES: Switched to scene \"%s\"\n", scenes->current_scene->name);
  {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "scene", sizeof(char) * (strnlen(scenes->current_scene->name, 128) + 1), scenes->current_scene->name);
    gossip_emit(MSG_SCENE_CHANGED, map);
  }
}

void scenes_internal_move(Scenes *scenes, int32_t move) {
  assert(scenes);

  for (uintmax_t t = 0; t < rectify_array_size(scenes->scenes); t++) {
    if (&scenes->scenes[t] == scenes->current_scene) {
      if ((t > 0 && move < 0) || (t < rectify_array_size(scenes->scenes) - 1 && move > 0)) {
        scenes_internal_go(scenes, (uint32_t)t + move);
      }
      break;
    }
  }
}

void scenes_goto(Scenes *scenes, const char *name) {
  assert(scenes);

  for (uintmax_t t = 0; t < rectify_array_size(scenes->scenes); t++) {
    if (strncmp(scenes->scenes[t].name, name, 128) == 0) {
      scenes_internal_go(scenes, (uint32_t)t);
      return;
    }
  }

  printf("SCENES: No such scene, \"%s\"\n", name);
  scenes->current_scene = &scenes_dummy;
  scenes->current_scene_data = NULL;
}

void scenes_update(Scenes *scenes, double delta) {
  assert(scenes);

  if (!scenes->pause_updates) {
    scenes->current_scene->update(scenes->current_scene_data, delta);
  }
}

void scenes_internal_scene_event(const char *groupd_id, const char *id, void *const subscriberdata, void *const userdata) {
  Scenes *scenes = (Scenes *)subscriberdata;

  if (strncmp(id, "prev", 128) == 0) {
    scenes_internal_move(scenes, -1);
  } else if (strncmp(id, "next", 128) == 0) {
    scenes_internal_move(scenes, 1);
  } else if (strncmp(id, "pause_updates", 128) == 0) {
    scenes->pause_updates = !scenes->pause_updates;
  }
}
*/
