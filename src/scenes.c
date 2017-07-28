#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "messages.h"
#include "scenes.h"
#include "screen.h"

struct Scenes {
  Scene *scenes;

  Scene *current_scene;
  void *current_scene_data;

  bool pause_updates;
};

void *scenes_dummy_create(void) {
  return NULL;
}

void scenes_dummy_destroy(void *const scene) {
}

void scenes_dummy_update(void *const scene, double delta) {
}

void scenes_dummy_draw(void *const scene, AsciiBuffer *const screen) {
}

Scene scenes_dummy = {
  .name = "dummy",
  .create = &scenes_dummy_create,
  .destroy = &scenes_dummy_destroy,
  .update = &scenes_dummy_update,
  .draw = &scenes_dummy_draw,
};

void scenes_internal_scene_event(const char *groupd_id, const char *id, void *const subscriberdata, void *const userdata);
void scenes_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

Scenes *scenes_create(void) {
  Scenes *scenes = calloc(1, sizeof(Scenes));

  scenes->scenes = rectify_array_alloc(10, sizeof(Scene));
  scenes->current_scene = &scenes_dummy;
  scenes->current_scene_data = NULL;
  scenes->pause_updates = false;

  screen_hook_render(&scenes_internal_render_hook, scenes, 0);

  return scenes;
}

void scenes_destroy(Scenes *scenes) {
  assert(scenes);

  screen_unhook_render(&scenes_internal_render_hook, scenes);

  if (scenes->current_scene) {
    scenes->current_scene->destroy(scenes->current_scene_data);
  }

  for (uintmax_t t = 0; t < rectify_array_size(scenes->scenes); t++) {
    free(scenes->scenes[t].name);
  }
  rectify_array_free(&scenes->scenes);
  free(scenes);
}

void scenes_register(Scenes *scenes, Scene *scene) {
  assert(scenes);

  Scene scene_cpy = {
    .name = rectify_memory_alloc_copy(scene->name, sizeof(char) * (strlen(scene->name) + 1)),
    .create = scene->create,
    .destroy = scene->destroy,
    .update = scene->update,
    .draw = scene->draw,
  };
  scenes->scenes = rectify_array_push(scenes->scenes, &scene_cpy);
}

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

void scenes_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  Scenes *scenes = (Scenes *)userdata;

  scenes->current_scene->draw(scenes->current_scene_data, screen);
}
