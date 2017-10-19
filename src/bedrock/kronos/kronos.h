/**
 * Kronos - system management and control
 */

#pragma once

#include "../rectify/rectify.h"

typedef enum {
  KRONOS_OK = 1,
  KRONOS_SYSTEM_NOT_FOUND,
  KRONOS_SYSTEM_NAME_TAKEN,
  KRONOS_SYSTEM_FAILED_TO_START,
  KRONOS_SYSTEM_STOP_PREVENTED,
} KronosResult;

typedef void *(*KronosSystemStart)(void);
typedef void (*KronosSystemMessage)(void *system, uint32_t id, RectifyMap *const map);
typedef void (*KronosSystemUpdate)(void *system, double delta);
typedef void (*KronosSystemStop)(void **system);

typedef struct {
  char *name;
  uint32_t frames;
  bool prevent_stop;
  bool autostart;
  KronosSystemStart start;
  KronosSystemStop stop;
  KronosSystemUpdate update;
  KronosSystemMessage message;
} KronosSystem;

void kronos_init(void);
void kronos_kill(void);
void kronos_halt(void);
bool kronos_should_halt(void);

KronosResult kronos_register(KronosSystem *const system);
KronosResult kronos_start_system(const char *name);
KronosResult kronos_stop_system(const char *name);

void kronos_post(const char *system, uint32_t id, RectifyMap *const map, const char *caller);
void kronos_emit(uint32_t id, RectifyMap *const map);
RectifyMap *kronos_post_immediate(const char *system, uint32_t id, RectifyMap *const map);

void kronos_update(double delta);
