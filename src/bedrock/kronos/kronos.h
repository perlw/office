/**
 * Kronos - system management and control
 */

#pragma once

#include "../rectify/rectify.h"

#define KRONOS_QUEUE 0

typedef enum {
  KRONOS_OK = 1,
  KRONOS_SYSTEM_NOT_FOUND,
  KRONOS_SYSTEM_NAME_TAKEN,
  KRONOS_SYSTEM_FAILED_TO_START,
  KRONOS_SYSTEM_STOP_PREVENTED,
  KRONOS_SYSTEM_NOT_RUNNING,
  KRONOS_SYSTEM_ALREADY_HOOKED,
} KronosResult;

typedef void *(*KronosSystemStart)(void);
typedef RectifyMap *(*KronosSystemMessage)(void *system, uint32_t id, RectifyMap *const map);
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
KronosResult kronos_hook_queue(const char *name, uint32_t queue);

void kronos_post(const char *system, uint32_t id, RectifyMap *const map, const char *caller);
void kronos_emit(uint32_t id, RectifyMap *const map);
void kronos_post_queue(uint32_t queue, const char *system, uint32_t id, RectifyMap *const map, const char *caller);
void kronos_emit_queue(uint32_t queue, uint32_t id, RectifyMap *const map);
// Immediately posts to system and returns result synchronously.
// Note: Any used or returned map(s) needs to behandled by caller.
RectifyMap *kronos_post_immediate(const char *system, uint32_t id, RectifyMap *const map);
void kronos_emit_immediate(uint32_t id, RectifyMap *const map);
RectifyMap *kronos_post_queue_immediate(uint32_t queue, const char *system, uint32_t id, RectifyMap *const map);
void kronos_emit_queue_immediate(uint32_t queue, uint32_t id, RectifyMap *const map);

void kronos_update(double delta);
