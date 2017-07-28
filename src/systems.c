#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "system_debug.h"
#include "system_input.h"
/*#include "system_lua_bridge.h"
#include "system_sound.h"
#include "system_ui.h"*/

typedef struct {
  GossipHandle gossip_handle;
} Systems;

Systems *systems_internal;

void systems_internal_start(const char *system);
void systems_internal_stop(const char *system);
void systems_internal_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);

void systems_init(void) {
  assert(!systems_internal);

  systems_internal = calloc(1, sizeof(Systems));

  systems_internal->gossip_handle = gossip_subscribe("system:*", &systems_internal_event, NULL);

  kronos_init();

  kronos_register(&system_debug);
  kronos_register(&system_input);
  /*kronos_register(&system_sound);
  kronos_register(&system_ui);
  kronos_register(&system_lua_bridge);*/

  systems_internal_start("debug");
  systems_internal_start("input");
}

void systems_kill(void) {
  assert(systems_internal);

  gossip_unsubscribe(systems_internal->gossip_handle);

  kronos_kill();
  free(systems_internal);
}

void systems_update(double delta) {
  assert(systems_internal);

  kronos_update(delta);
}

void systems_internal_start(const char *system) {
  printf("System: attempting to spin up \"%s\"...\n", system);
  KronosResult result = kronos_start_system(system);
  if (result == KRONOS_OK) {
    printf("System: \"%s\" spun up!\n", system);
  } else {
    printf("System: \"%s\" %s\n", system, (result == KRONOS_SYSTEM_NOT_FOUND ? "does not exist" : "failed to spin up"));
  }
}

void systems_internal_stop(const char *system) {
  printf("Systems: attempting to stop \"%s\"\n", system);
  KronosResult result = kronos_stop_system(system);
  if (result == KRONOS_OK) {
    printf("System: \"%s\" shut down!\n", system);
  } else {
    printf("System: \"%s\" %s\n", system, (result == KRONOS_SYSTEM_NOT_FOUND ? "does not exist" : "stop prevented"));
  }
}

void systems_internal_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  const char *system = (char *)userdata;

  if (strncmp(id, "start", 128) == 0) {
    systems_internal_start(system);
  } else if (strncmp(id, "stop", 128) == 0) {
    systems_internal_stop(system);
  }
}
