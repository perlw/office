#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "kronos.h"

typedef struct {
  char *system;
  uint32_t id;
  RectifyMap *map;
} QueueItem;

typedef struct {
  KronosSystem *system;
  void *handle;
  bool running;
  double timing;
  double since_update;
} KronosState;

typedef struct {
  KronosState *systems;

  QueueItem *queue;
  QueueItem *active_queue;

  bool halt;
} Kronos;

Kronos *kronos = NULL;

void kronos_init(void) {
  assert(!kronos);

  kronos = calloc(1, sizeof(Kronos));
  *kronos = (Kronos){
    .systems = rectify_array_alloc(10, sizeof(KronosState)),
    .queue = rectify_array_alloc(10, sizeof(QueueItem)),
    .active_queue = rectify_array_alloc(10, sizeof(QueueItem)),
    .halt = false,
  };
}

void kronos_kill(void) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->queue); t++) {
    QueueItem *const item = &kronos->queue[t];
    if (item->system) {
      free(item->system);
    }
    rectify_map_destroy(&item->map);
  }
  rectify_array_free((void **)&kronos->queue);
  for (uintmax_t t = 0; t < rectify_array_size(kronos->active_queue); t++) {
    QueueItem *const item = &kronos->active_queue[t];
    if (item->system) {
      free(item->system);
    }
    rectify_map_destroy(&item->map);
  }
  rectify_array_free((void **)&kronos->active_queue);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosState *state = &kronos->systems[t];

    if (state->running) {
      state->system->stop(&state->handle);
      state->running = false;
    }

    free(state->system);
  }
  rectify_array_free((void **)&kronos->systems);

  free(kronos);
}

void kronos_halt(void) {
  assert(kronos);
  kronos->halt = true;
}

bool kronos_should_halt(void) {
  assert(kronos);
  return kronos->halt;
}

KronosResult kronos_register(KronosSystem *const system) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosState *state = &kronos->systems[t];

    if (strncmp(state->system->name, system->name, 128) == 0) {
      return KRONOS_SYSTEM_NAME_TAKEN;
    }
  }

  kronos->systems = rectify_array_push(kronos->systems, &(KronosState){
                                                          .system = rectify_memory_alloc_copy(system, sizeof(KronosSystem)),
                                                          .running = false,
                                                          .timing = 1.0 / (double)system->frames,
                                                          .since_update = 0.0,
                                                          .handle = NULL,
                                                        });

  if (system->autostart) {
    kronos_start_system(system->name);
  }

  return KRONOS_OK;
}

KronosResult kronos_start_system(const char *name) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosState *state = &kronos->systems[t];

    if (strncmp(state->system->name, name, 128) == 0) {
      state->handle = state->system->start();
      if (state->handle) {
        state->since_update = (state->system->frames == 0 ? 0.0 : 1.0 / (double)((rand() % state->system->frames) + 1));
        state->running = true;
        return KRONOS_OK;
      } else {
        state->running = false;
        return KRONOS_SYSTEM_FAILED_TO_START;
      }
    }
  }

  return KRONOS_SYSTEM_NOT_FOUND;
}

KronosResult kronos_stop_system(const char *name) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosState *state = &kronos->systems[t];

    if (strncmp(state->system->name, name, 128) == 0) {
      if (state->system->prevent_stop) {
        return KRONOS_SYSTEM_STOP_PREVENTED;
      } else {
        state->system->stop(&state->handle);
        state->running = false;
        return KRONOS_OK;
      }
    }
  }

  return KRONOS_SYSTEM_NOT_FOUND;
}

void kronos_post(const char *system, uint32_t id, RectifyMap *const map) {
  assert(kronos);
  kronos->active_queue = rectify_array_push(kronos->active_queue, &(QueueItem){
                                                                    .system = rectify_memory_alloc_copy(system, strnlen(system, 128) + 1),
                                                                    .id = id,
                                                                    .map = map,
                                                                  });
}

void kronos_emit(uint32_t id, RectifyMap *const map) {
  assert(kronos);
  kronos->active_queue = rectify_array_push(kronos->active_queue, &(QueueItem){
                                                                    .system = NULL,
                                                                    .id = id,
                                                                    .map = map,
                                                                  });
}

void kronos_update(double delta) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosState *state = &kronos->systems[t];

    if (!state->running || !state->system->update) {
      continue;
    }

    if (state->system->frames == 0) {
      state->system->update(state->handle, state->timing);
    } else {
      state->since_update += delta;
      while (state->since_update >= state->timing) {
        state->since_update -= state->timing;

        state->system->update(state->handle, state->timing);
      }
    }
  }

  QueueItem *swp = kronos->queue;
  kronos->queue = kronos->active_queue;
  kronos->active_queue = swp;

  uintmax_t queue_size = rectify_array_size(kronos->queue);
  for (uintmax_t t = 0; t < queue_size; t++) {
    QueueItem *const item = &kronos->queue[t];

    if (item->system) {
      for (uint32_t u = 0; u < rectify_array_size(kronos->systems); u++) {
        KronosState *state = &kronos->systems[u];

        if (!(state->running && state->system->message)) {
          continue;
        }

        if (strncmp(state->system->name, item->system, 128) == 0) {
          state->system->message(state->handle, item->id, item->map);
          break;
        }
      }
      free(item->system);
      item->system = NULL;
    } else {
      for (uint32_t u = 0; u < rectify_array_size(kronos->systems); u++) {
        KronosState *state = &kronos->systems[u];

        if (!(state->running && state->system->message)) {
          continue;
        }

        state->system->message(state->handle, item->id, item->map);
      }
    }

    rectify_map_destroy(&item->map);
  }

  rectify_array_free((void **)&kronos->queue);
  kronos->queue = rectify_array_alloc(10, sizeof(QueueItem));
}
