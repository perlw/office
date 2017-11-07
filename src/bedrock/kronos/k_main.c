#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "kronos.h"

typedef struct {
  char *system;
  char *caller;
  uint32_t id;
  uint32_t queue_id;
  RectifyMap *map;
} QueueItem;

typedef struct {
  QueueItem *back;
  QueueItem *front;
} Queue;

typedef struct {
  KronosSystem *system;
  void *handle;
  bool running;
  double timing;
  double since_update;
  uint32_t *queues;
} KronosState;

typedef struct {
  KronosState *systems;

  Queue queue;

  bool halt;
} Kronos;

Kronos *kronos = NULL;

Queue queue_setup(void) {
  return (Queue){
    .back = rectify_array_alloc(10, sizeof(QueueItem)),
    .front = rectify_array_alloc(10, sizeof(QueueItem)),
  };
}

void queue_cleanup(Queue *queue) {
  for (uintmax_t t = 0; t < rectify_array_size(queue->front); t++) {
    QueueItem *const item = &queue->front[t];
    if (item->system) {
      free(item->system);
      item->system = NULL;
    }
    if (item->caller) {
      free(item->caller);
      item->caller = NULL;
    }
    rectify_map_destroy(&item->map);
  }
  rectify_array_free((void **)&queue->front);
  for (uintmax_t t = 0; t < rectify_array_size(queue->back); t++) {
    QueueItem *const item = &queue->back[t];
    if (item->system) {
      free(item->system);
      item->system = NULL;
    }
    if (item->caller) {
      free(item->caller);
      item->caller = NULL;
    }
    rectify_map_destroy(&item->map);
  }
  rectify_array_free((void **)&queue->back);
}

void queue_update(Queue *queue) {
  QueueItem *swp = queue->front;
  queue->front = queue->back;
  queue->back = swp;

  uintmax_t queue_size = rectify_array_size(queue->front);
#ifdef KRONOS_DEBUG
  if (queue_size > 0) {
    printf("Kronos: Processing %d queued messages...\n", (uint32_t)queue_size);
  }
#endif

  for (uintmax_t t = 0; t < queue_size; t++) {
    QueueItem *const item = &queue->front[t];

    if (item->system) {
      for (uint32_t u = 0; u < rectify_array_size(kronos->systems); u++) {
        KronosState *state = &kronos->systems[u];

        if (!(state->running && state->system->message)) {
          continue;
        }

        if (item->queue_id != 0) {
          bool found = false;
          for (uint32_t v = 0; v < rectify_array_size(state->queues); v++) {
            if (item->queue_id == state->queues[v]) {
              found = true;
              break;
            }
          }
          if (!found) {
            continue;
          }
        }

        if (strncmp(state->system->name, item->system, 128) == 0) {
          RectifyMap *response = state->system->message(state->handle, item->id, item->map);
          if (response) {
            if (item->caller) {
              kronos_post(item->caller, item->id, response, item->system);
            } else {
              printf("Kronos: Post had no caller, deleted... ");
              rectify_map_print(response);
              rectify_map_destroy(&response);
            }
          }
          break;
        }
      }
    } else {
      for (uint32_t u = 0; u < rectify_array_size(kronos->systems); u++) {
        KronosState *state = &kronos->systems[u];

        if (!(state->running && state->system->message)) {
          continue;
        }

        if (item->queue_id != KRONOS_QUEUE) {
          bool found = false;
          for (uint32_t v = 0; v < rectify_array_size(state->queues); v++) {
            if (item->queue_id == state->queues[v]) {
              found = true;
              break;
            }
          }
          if (!found) {
            continue;
          }
        }

        RectifyMap *response = state->system->message(state->handle, item->id, item->map);
        if (response) {
          printf("Kronos: Emit returning a response, deleted... ");
          rectify_map_print(response);
          rectify_map_destroy(&response);
        }
      }
    }
    if (item->system) {
      free(item->system);
      item->system = NULL;
    }
    if (item->caller) {
      free(item->caller);
      item->caller = NULL;
    }

    rectify_map_destroy(&item->map);
  }

  rectify_array_free((void **)&queue->front);
  queue->front = rectify_array_alloc(10, sizeof(QueueItem));
}

void kronos_init(void) {
  assert(!kronos);

  kronos = calloc(1, sizeof(Kronos));
  *kronos = (Kronos){
    .systems = rectify_array_alloc(10, sizeof(KronosState)),
    .queue = queue_setup(),
    .halt = false,
  };
}

void kronos_kill(void) {
  assert(kronos);

  queue_cleanup(&kronos->queue);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosState *state = &kronos->systems[t];

    if (state->running) {
      state->system->stop(&state->handle);
      state->running = false;
    }

    rectify_array_free(&state->queues);

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
                                                          .queues = rectify_array_alloc(10, sizeof(uint32_t)),
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
#ifdef KRONOS_DEBUG
        printf("Kronos: System \"%s\" started\n", name);
#endif
        state->since_update = (state->system->frames == 0 ? 0.0 : 1.0 / (double)((rand() % state->system->frames) + 1));
        state->running = true;
        return KRONOS_OK;
      } else {
#ifdef KRONOS_DEBUG
        printf("Kronos: Failed to start system \"%s\"\n", name);
#endif
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
      if (!state->running) {
#ifdef KRONOS_DEBUG
        printf("Kronos: Failed to stop system \"%s\", not running\n", name);
#endif
        return KRONOS_SYSTEM_NOT_RUNNING;
      } else if (state->system->prevent_stop) {
#ifdef KRONOS_DEBUG
        printf("Kronos: Failed to stop system \"%s\", prevented\n", name);
#endif
        return KRONOS_SYSTEM_STOP_PREVENTED;
      } else {
        state->system->stop(&state->handle);
        state->running = false;
#ifdef KRONOS_DEBUG
        printf("Kronos: System \"%s\" stopped\n", name);
#endif
        return KRONOS_OK;
      }
    }
  }

  return KRONOS_SYSTEM_NOT_FOUND;
}

KronosResult kronos_hook_queue(const char *name, uint32_t queue) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosState *state = &kronos->systems[t];

    if (strncmp(state->system->name, name, 128) == 0) {
      for (uintmax_t u = 0; u < rectify_array_size(state->queues); u++) {
        if (state->queues[u] == queue) {
#ifdef KRONOS_DEBUG
          printf("Kronos: System \"%s\" already hooked to queue #%d\n", name, queue);
#endif
          return KRONOS_SYSTEM_ALREADY_HOOKED;
        }
      }

      state->queues = rectify_array_push(state->queues, &queue);
#ifdef KRONOS_DEBUG
      printf("Kronos: System \"%s\" hooked to queue #%d\n", name, queue);
#endif
      return KRONOS_OK;
    }
  }

  return KRONOS_SYSTEM_NOT_FOUND;
}

void kronos_post(const char *system, uint32_t id, RectifyMap *const map, const char *caller) {
  assert(kronos);

#ifdef KRONOS_DEBUG
  printf("Kronos: System \"%s\" posting id#%d to \"%s\" ->\n", (caller ? caller : "UNK"), id, system);
  rectify_map_print(map);
#endif

  kronos->queue.back = rectify_array_push(kronos->queue.back, &(QueueItem){
                                                                .system = rectify_memory_alloc_copy(system, strnlen(system, 128) + 1),
                                                                .caller = (caller ? rectify_memory_alloc_copy(caller, strnlen(caller, 128) + 1) : NULL),
                                                                .id = id,
                                                                .map = map,
                                                                .queue_id = KRONOS_QUEUE,
                                                              });
}

void kronos_emit(uint32_t id, RectifyMap *const map) {
  assert(kronos);

#ifdef KRONOS_DEBUG
  printf("Kronos: Emitting id#%d ->\n", id);
  rectify_map_print(map);
#endif

  kronos->queue.back = rectify_array_push(kronos->queue.back, &(QueueItem){
                                                                .system = NULL,
                                                                .caller = NULL,
                                                                .id = id,
                                                                .map = map,
                                                                .queue_id = KRONOS_QUEUE,
                                                              });
}

void kronos_post_queue(uint32_t queue, const char *system, uint32_t id, RectifyMap *const map, const char *caller) {
  assert(kronos);

#ifdef KRONOS_DEBUG
  printf("Kronos: System \"%s\" posting id#%d to \"%s\" on queue#%d ->\n", (caller ? caller : "UNK"), id, system, queue);
  rectify_map_print(map);
#endif

  kronos->queue.back = rectify_array_push(kronos->queue.back, &(QueueItem){
                                                                .system = rectify_memory_alloc_copy(system, strnlen(system, 128) + 1),
                                                                .caller = (caller ? rectify_memory_alloc_copy(caller, strnlen(caller, 128) + 1) : NULL),
                                                                .id = id,
                                                                .map = map,
                                                                .queue_id = queue,
                                                              });
}

void kronos_emit_queue(uint32_t queue, uint32_t id, RectifyMap *const map) {
  assert(kronos);

#ifdef KRONOS_DEBUG
  printf("Kronos: Emitting id#%d on queue#%d->\n", id, queue);
  rectify_map_print(map);
#endif

  kronos->queue.back = rectify_array_push(kronos->queue.back, &(QueueItem){
                                                                .system = NULL,
                                                                .caller = NULL,
                                                                .id = id,
                                                                .map = map,
                                                                .queue_id = queue,
                                                              });
}

RectifyMap *kronos_post_immediate(const char *system, uint32_t id, RectifyMap *const map) {
#ifdef KRONOS_DEBUG_IMMEDIATE
  printf("Kronos: Immediate posting id#%d to \"%s\" ->\n", id, system);
  rectify_map_print(map);
#endif

  for (uint32_t u = 0; u < rectify_array_size(kronos->systems); u++) {
    KronosState *state = &kronos->systems[u];

    if (!(state->running && state->system->message)) {
      continue;
    }

    if (strncmp(state->system->name, system, 128) == 0) {
      return state->system->message(state->handle, id, map);
    }
  }

  return NULL;
}

void kronos_emit_immediate(uint32_t id, RectifyMap *const map) {
#ifdef KRONOS_DEBUG_IMMEDIATE
  printf("Kronos: Immediate emitting id#%d ->\n", id);
  rectify_map_print(map);
#endif

  for (uint32_t u = 0; u < rectify_array_size(kronos->systems); u++) {
    KronosState *state = &kronos->systems[u];

    if (!(state->running && state->system->message)) {
      continue;
    }

    state->system->message(state->handle, id, map);
  }
}

RectifyMap *kronos_post_queue_immediate(uint32_t queue, const char *system, uint32_t id, RectifyMap *const map) {
#ifdef KRONOS_DEBUG_IMMEDIATE
  printf("Kronos: Immediate posting id#%d to \"%s\" on queue#%d ->\n", id, system, queue);
  rectify_map_print(map);
#endif

  for (uint32_t u = 0; u < rectify_array_size(kronos->systems); u++) {
    KronosState *state = &kronos->systems[u];

    if (!(state->running && state->system->message)) {
      continue;
    }

    bool found = false;
    for (uint32_t v = 0; v < rectify_array_size(state->queues); v++) {
      if (queue == state->queues[v]) {
        found = true;
        break;
      }
    }
    if (!found) {
      continue;
    }

    if (strncmp(state->system->name, system, 128) == 0) {
      return state->system->message(state->handle, id, map);
    }
  }

  return NULL;
}

void kronos_emit_queue_immediate(uint32_t queue, uint32_t id, RectifyMap *const map) {
#ifdef KRONOS_DEBUG_IMMEDIATE
  printf("Kronos: Immediate emitting id#%d on queue#%d ->\n", id, queue);
  rectify_map_print(map);
#endif

  for (uint32_t u = 0; u < rectify_array_size(kronos->systems); u++) {
    KronosState *state = &kronos->systems[u];

    if (!(state->running && state->system->message)) {
      continue;
    }

    bool found = false;
    for (uint32_t v = 0; v < rectify_array_size(state->queues); v++) {
      if (queue == state->queues[v]) {
        found = true;
        break;
      }
    }
    if (!found) {
      continue;
    }

    state->system->message(state->handle, id, map);
  }
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

  queue_update(&kronos->queue);
}
