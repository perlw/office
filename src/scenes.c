#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "messages.h"
#include "scenes.h"

struct Scenes {
  Scene *scenes;

  Scene *current_scene;
  void *current_scene_data;

  GossipHandle system_handle;
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

Scene scene_dummy = {
  .name = "dummy",
  .create = &scenes_dummy_create,
  .destroy = &scenes_dummy_destroy,
  .update = &scenes_dummy_update,
  .draw = &scenes_dummy_draw,
};

void scenes_internal_system_event(uint32_t id, void *const subscriberdata, void *const userdata);

Scenes *scenes_create(void) {
  Scenes *scenes = calloc(1, sizeof(Scenes));

  scenes->scenes = rectify_array_alloc(10, sizeof(Scene));
  scenes->current_scene = &scene_dummy;
  scenes->current_scene_data = NULL;

  scenes->system_handle = gossip_subscribe(MSG_SYSTEM, GOSSIP_ID_ALL, &scenes_internal_system_event, scenes, NULL);

  return scenes;
}

void scenes_destroy(Scenes *scenes) {
  assert(scenes);

  gossip_unsubscribe(scenes->system_handle);

  if (scenes->current_scene) {
    scenes->current_scene->destroy(scenes->current_scene_data);
  }

  for (uintmax_t t = 0; t < rectify_array_size(scenes->scenes); t++) {
    free(scenes->scenes[t].name);
  }
  rectify_array_free(scenes->scenes);
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

Scene *scenes_goto(Scenes *scenes, const char *name) {
  assert(scenes);

  if (scenes->current_scene) {
    scenes->current_scene->destroy(scenes->current_scene_data);
    scenes->current_scene = NULL;
  }
  for (uintmax_t t = 0; t < rectify_array_size(scenes->scenes); t++) {
    if (strncmp(scenes->scenes[t].name, name, 128) == 0) {
      scenes->current_scene = &scenes->scenes[t];
    }
  }

  if (!scenes->current_scene) {
    printf("SCENES: No such scene, \"%s\"\n", name);
    scenes->current_scene = &scene_dummy;
    scenes->current_scene_data = NULL;
  }

  scenes->current_scene_data = scenes->current_scene->create();

  printf("SCENES: Switched to scene \"%s\"\n", name);

  return scenes->current_scene;
}

void scenes_go(Scenes *scenes, int32_t move) {
  assert(scenes);

  if (!scenes->current_scene) {
    return;
  }

  for (uintmax_t t = 0; t < rectify_array_size(scenes->scenes); t++) {
    if (&scenes->scenes[t] == scenes->current_scene) {
      if ((t > 0 && move < 0) || (t < rectify_array_size(scenes->scenes) - 1 && move > 0)) {
        scenes->current_scene->destroy(scenes->current_scene_data);
        scenes->current_scene = NULL;

        scenes->current_scene = &scenes->scenes[t + move];
        scenes->current_scene_data = scenes->current_scene->create();

        printf("SCENES: Switched to scene \"%s\"\n", scenes->current_scene->name);
      }

      break;
    }
  }
}

Scene *scenes_prev(Scenes *scenes) {
  scenes_go(scenes, -1);
  return scenes->current_scene;
}

Scene *scenes_next(Scenes *scenes) {
  scenes_go(scenes, 1);
  return scenes->current_scene;
}

void scenes_internal_system_event(uint32_t id, void *const subscriberdata, void *const userdata) {
  Scenes *scenes = (Scenes *)subscriberdata;

  switch (id) {
    case MSG_SYSTEM_UPDATE:
      scenes->current_scene->update(scenes->current_scene_data, *(double *)userdata);
      break;

    case MSG_SYSTEM_DRAW: {
      AsciiBuffer *screen = (AsciiBuffer *)userdata;
      scenes->current_scene->draw(scenes->current_scene_data, screen);
      break;
    }
  }
}
