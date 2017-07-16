#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "scenes.h"

struct Scenes {
  Scene *scenes;

  Scene *current_scene;
  void *current_scene_data;

  GossipHandle scene_handle;
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

void scenes_internal_scene_event(const char *groupd_id, const char *id, void *const subscriberdata, void *const userdata);

Scenes *scenes_create(void) {
  Scenes *scenes = calloc(1, sizeof(Scenes));

  scenes->scenes = rectify_array_alloc(10, sizeof(Scene));
  scenes->current_scene = &scene_dummy;
  scenes->current_scene_data = NULL;

  scenes->scene_handle = gossip_subscribe("scene:*", &scenes_internal_scene_event, scenes);

  return scenes;
}

void scenes_destroy(Scenes *scenes) {
  assert(scenes);

  gossip_unsubscribe(scenes->scene_handle);

  if (scenes->current_scene) {
    gossip_emit("scene:teardown", scenes->current_scene);
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

void scenes_internal_go(Scenes *scenes, uint32_t index) {
  gossip_emit("scene:teardown", scenes->current_scene);
  scenes->current_scene->destroy(scenes->current_scene_data);
  scenes->current_scene = NULL;

  scenes->current_scene = &scenes->scenes[index];
  scenes->current_scene_data = scenes->current_scene->create();
  gossip_emit("scene:setup", scenes->current_scene);

  printf("SCENES: Switched to scene \"%s\"\n", scenes->current_scene->name);
  gossip_emit("scene:changed", scenes->current_scene);
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
  scenes->current_scene = &scene_dummy;
  scenes->current_scene_data = NULL;
}

void scenes_update(Scenes *scenes, double delta) {
  assert(scenes);

  scenes->current_scene->update(scenes->current_scene_data, delta);
}

void scenes_draw(Scenes *scenes, AsciiBuffer *const screen) {
  assert(scenes);

  scenes->current_scene->draw(scenes->current_scene_data, screen);
}

void scenes_internal_scene_event(const char *groupd_id, const char *id, void *const subscriberdata, void *const userdata) {
  Scenes *scenes = (Scenes *)subscriberdata;

  if (strncmp(id, "prev", 128) == 0) {
    scenes_internal_move(scenes, -1);
  } else if (strncmp(id, "next", 128) == 0) {
    scenes_internal_move(scenes, 1);
  }
}
