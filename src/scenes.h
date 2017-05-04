#pragma once

#include "config.h"

typedef void *(*ScenesCreate)(const Config *config);
typedef void (*ScenesDestroy)(void *scene);
typedef void (*ScenesUpdate)(void *scene, double delta);
typedef void (*ScenesDraw)(void *scene);

typedef struct {
  char *name;
  ScenesCreate create;
  ScenesDestroy destroy;
  ScenesUpdate update;
  ScenesDraw draw;
} Scene;

typedef struct Scenes Scenes;

Scenes *scenes_create(Config *config);
void scenes_destroy(Scenes *scenes);
void scenes_update(Scenes *scenes, double delta);
void scenes_draw(Scenes *scenes);

void scenes_register(Scenes *scenes, Scene *scene);
void scenes_goto(Scenes *scenes, const char *name);
void scenes_prev(Scenes *scenes);
void scenes_next(Scenes *scenes);
