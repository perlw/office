#pragma once

#include "ascii/ascii.h"

typedef void *(*ScenesCreate)(void);
typedef void (*ScenesDestroy)(void *const scene);
typedef void (*ScenesUpdate)(void *const scene, double delta);
typedef void (*ScenesDraw)(void *const scene, AsciiBuffer *const screen);

typedef struct {
  char *name;
  ScenesCreate create;
  ScenesDestroy destroy;
  ScenesUpdate update;
  ScenesDraw draw;
} Scene;

typedef struct Scenes Scenes;

Scenes *scenes_create(void);
void scenes_destroy(Scenes *scenes);

void scenes_register(Scenes *scenes, Scene *scene);
void scenes_goto(Scenes *scenes, const char *name);

void scenes_update(Scenes *scenes, double delta);
void scenes_draw(Scenes *scenes, AsciiBuffer *const screen);
