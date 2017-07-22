#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "system_debug.h"
#include "system_sound.h"

typedef struct {
  GossipHandle gossip_handle;
} Systems;

Systems *systems_internal;

void systems_internal_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);

void systems_init(void) {
  assert(!systems_internal);

  systems_internal = calloc(1, sizeof(Systems));

  systems_internal->gossip_handle = gossip_subscribe("system:*", &systems_internal_event, NULL);

  kronos_init();

  kronos_register(&system_debug);
  kronos_register(&system_sound);
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

void systems_internal_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  const char *system = (char *)userdata;

  if (strncmp(id, "start", 128) == 0) {
    printf("System: attempting to spin up \"%s\"...\n", system);
    KronosResult result = kronos_start_system(system);
    if (result == KRONOS_OK) {
      printf("System: \"%s\" spun up!\n", system);
    } else {
      printf("System: \"%s\" %s\n", system, (result == KRONOS_SYSTEM_NOT_FOUND ? "does not exist" : "failed to spin up"));
    }
  } else if (strncmp(id, "stop", 128) == 0) {
    printf("Systems: stopping \"%s\"\n", system);
    kronos_stop_system(system);
  }
}
