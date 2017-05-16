#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "scenes.h"

struct Scenes {
  Scene *scenes;
  Config *config;

  Scene *current_scene;
  void *current_scene_data;
};

void *scenes_dummy_create(const Config *config) {
}

void scenes_dummy_destroy(void *scene) {
}

void scenes_dummy_update(void *scene, double delta) {
}

void scenes_dummy_draw(void *scene) {
}

Scene scene_dummy = {
  .name = "dummy",
  .create = &scenes_dummy_create,
  .destroy = &scenes_dummy_destroy,
  .update = &scenes_dummy_update,
  .draw = &scenes_dummy_draw,
};

Scenes *scenes_create(Config *config) {
  Scenes *scenes = calloc(1, sizeof(Scenes));

  scenes->scenes = rectify_array_alloc(10, sizeof(Scene));
  scenes->config = config;
  scenes->current_scene = &scene_dummy;
  scenes->current_scene_data = NULL;

  return scenes;
}

void scenes_destroy(Scenes *scenes) {
  assert(scenes);

  if (scenes->current_scene) {
    scenes->current_scene->destroy(scenes->current_scene_data);
  }

  for (uintmax_t t = 0; t < rectify_array_size(scenes->scenes); t++) {
    free(scenes->scenes[t].name);
  }
  rectify_array_free(scenes->scenes);
  free(scenes);
}

void scenes_update(Scenes *scenes, double delta) {
  assert(scenes);

  if (scenes->current_scene) {
    scenes->current_scene->update(scenes->current_scene_data, delta);
  }
}

void scenes_draw(Scenes *scenes) {
  assert(scenes);

  if (scenes->current_scene) {
    scenes->current_scene->draw(scenes->current_scene_data);
  }
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

  scenes->current_scene_data = scenes->current_scene->create(scenes->config);

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
        scenes->current_scene_data = scenes->current_scene->create(scenes->config);

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
