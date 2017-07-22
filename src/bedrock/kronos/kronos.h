/**
 * Kronos - system management and control
 */

#pragma once

typedef enum {
  KRONOS_OK = 1,
  KRONOS_SYSTEM_NOT_FOUND,
  KRONOS_NAME_TAKEN,
} KronosResult;

typedef void (*KronosSystemFunc)(void);

typedef struct {
  char *name;
  double timing;
  double since_update;
  KronosSystemFunc start;
  KronosSystemFunc stop;
  KronosSystemFunc update;
} KronosSystem;

void kronos_init(void);
void kronos_kill(void);

KronosResult kronos_register(KronosSystem *const system);
KronosResult kronos_start_system(const char *name);
KronosResult kronos_stop_system(const char *name);

void kronos_update(double delta);
