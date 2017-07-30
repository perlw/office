#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "kronos.h"

typedef struct {
  KronosSystem *system;
  bool running;
  double timing;
  double since_update;
} KronosState;

typedef struct {
  KronosState *systems;
} Kronos;

Kronos *kronos = NULL;

void kronos_init(void) {
  assert(!kronos);

  kronos = calloc(1, sizeof(Kronos));
  kronos->systems = rectify_array_alloc(10, sizeof(KronosState));
}

void kronos_kill(void) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosState *state = &kronos->systems[t];

    if (state->running) {
      gossip_unregister_system(state->system);
      state->system->stop();
      state->running = false;
    }

    free(state->system);
  }
  rectify_array_free(&kronos->systems);

  free(kronos);
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
                                                        });

  return KRONOS_OK;
}

KronosResult kronos_start_system(const char *name) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosState *state = &kronos->systems[t];

    if (strncmp(state->system->name, name, 128) == 0) {
      if (state->system->start()) {
        gossip_register_system(state->system);

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
        gossip_unregister_system(state->system);
        state->system->stop();
        state->running = false;
        return KRONOS_OK;
      }
    }
  }

  return KRONOS_SYSTEM_NOT_FOUND;
}

void kronos_update(double delta) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosState *state = &kronos->systems[t];

    if (!state->running) {
      continue;
    }

    if (state->system->frames == 0) {
      state->system->update();
    } else {
      state->since_update += delta;
      while (state->since_update >= state->timing) {
        state->since_update -= state->timing;

        state->system->update();
      }
    }
  }
}
