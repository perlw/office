/**
 * Kronos - system management and control
 */

#pragma once

typedef enum {
  KRONOS_OK = 1,
  KRONOS_SYSTEM_NOT_FOUND,
  KRONOS_SYSTEM_NAME_TAKEN,
  KRONOS_SYSTEM_FAILED_TO_START,
  KRONOS_SYSTEM_STOP_PREVENTED,
} KronosResult;

typedef bool (*KronosSystemStart)(void);
typedef void (*KronosSystemFunc)(void);

typedef struct {
  char *name;
  uint32_t frames;
  bool prevent_stop;
  KronosSystemStart start;
  KronosSystemFunc stop;
  KronosSystemFunc update;
} KronosSystem;

void kronos_init(void);
void kronos_kill(void);

KronosResult kronos_register(KronosSystem *const system);
KronosResult kronos_start_system(const char *name);
KronosResult kronos_stop_system(const char *name);

void kronos_update(double delta);
