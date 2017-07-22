#include <assert.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "kronos.h"

typedef struct {
  KronosSystem *systems;
} Kronos;

Kronos *kronos = NULL;

void kronos_init(void) {
  assert(!kronos);

  kronos = calloc(1, sizeof(Kronos));
  kronos->systems = rectify_array_alloc(10, sizeof(KronosSystem));
}

void kronos_kill(void) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    kronos->systems[t].stop();
  }
  rectify_array_free(kronos->systems);

  free(kronos);
}

KronosResult kronos_register(KronosSystem *const system) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosSystem *system = &kronos->systems[t];

    if (strncmp(system->name, system->name, 128) == 0) {
      return KRONOS_NAME_TAKEN;
    }
  }

  kronos->systems = rectify_array_push(kronos->systems, system);

  return KRONOS_OK;
}

KronosResult kronos_start_system(const char *name) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosSystem *system = &kronos->systems[t];

    if (strncmp(system->name, name, 128) == 0) {
      system->start();
      return KRONOS_OK;
    }
  }

  return KRONOS_SYSTEM_NOT_FOUND;
}

KronosResult kronos_stop_system(const char *name) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosSystem *system = &kronos->systems[t];

    if (strncmp(system->name, name, 128) == 0) {
      system->stop();
      return KRONOS_OK;
    }
  }

  return KRONOS_SYSTEM_NOT_FOUND;
}

void kronos_update(double delta) {
  assert(kronos);

  for (uintmax_t t = 0; t < rectify_array_size(kronos->systems); t++) {
    KronosSystem *system = &kronos->systems[t];

    system->since_update += delta;
    while (system->since_update >= system->timing) {
      system->since_update -= system->timing;
      system->update();
    }
  }
}
