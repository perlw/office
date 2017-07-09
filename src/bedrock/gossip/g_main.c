#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "occulus/occulus.h"

#include "gossip.h"
#include "rectify/rectify.h"

typedef struct {
  uint32_t id;
  GossipHandle handle;
  void *subscriberdata;
  GossipCallback callback;
} Listener;

typedef struct {
  uint32_t id;
  Listener *listeners;
} Group;

typedef struct {
  Group *groups;
} Gossip;
Gossip *gossip = NULL;

void gossip_init(void) {
  if (gossip) {
    return;
  }

  gossip = calloc(1, sizeof(Gossip));
  *gossip = (Gossip){
    .groups = rectify_array_alloc(4, sizeof(Group)),
  };
}

void gossip_destroy(void) {
  assert(gossip);

  for (uintmax_t t = 0; t < rectify_array_size(gossip->groups); t++) {
    rectify_array_free(gossip->groups[t].listeners);
  }
  rectify_array_free(gossip->groups);

  free(gossip);
}

GossipHandle gossip_subscribe(uint32_t group_id, uint32_t id, GossipCallback callback, void *const subscriberdata) {
  assert(gossip);

  Listener listener = (Listener){
    .id = id,
    .subscriberdata = subscriberdata,
    .callback = callback,
  };
  listener.handle = (uintptr_t)&listener;

  printf("Gossip: Subscribing to %d:%d... ", group_id, id);

  Group *group = NULL;
  for (uintmax_t t = 0; t < rectify_array_size(gossip->groups); t++) {
    Group *const g = &gossip->groups[t];

    if (group_id != GOSSIP_GROUP_ALL && g->id != group_id) {
      continue;
    }

    group = g;

    if (group_id != GOSSIP_GROUP_ALL) {
      break;
    }
  }

  if (!group) {
    group = &(Group){
      .id = group_id,
      .listeners = rectify_array_alloc(10, sizeof(Listener)),
    };
    gossip->groups = rectify_array_push(gossip->groups, group);
  }

  group->listeners = rectify_array_push(group->listeners, &listener);
  printf("subscribed, 0x%" PRIuPTR ".\n", listener.handle);

  return listener.handle;
}

void gossip_unsubscribe(GossipHandle handle) {
  assert(gossip);

  printf("Gossip: Unsubscribing 0x%" PRIuPTR "... ", handle);
  for (uintmax_t t = 0; t < rectify_array_size(gossip->groups); t++) {
    Group *const group = &gossip->groups[t];

    for (uintmax_t u = 0; u < rectify_array_size(group->listeners); u++) {
      Listener *const listener = &group->listeners[u];

      if (listener->handle == handle) {
        group->listeners = rectify_array_delete(group->listeners, u);
        printf("unsubscribed.\n");
        return;
      }
    }
  }
  printf("handle not found.\n");
}

void gossip_emit(uint32_t group_id, uint32_t id, void *const userdata) {
  assert(gossip);

  //printf("Gossip: Emitting to %d:%d... ", group_id, id);
  for (uintmax_t t = 0; t < rectify_array_size(gossip->groups); t++) {
    Group *const group = &gossip->groups[t];

    if (group_id != GOSSIP_GROUP_ALL && group->id != GOSSIP_GROUP_ALL && group->id != group_id) {
      continue;
    }

    for (uintmax_t u = 0; u < rectify_array_size(group->listeners); u++) {
      Listener *const listener = &group->listeners[u];

      if (id != GOSSIP_ID_ALL && listener->id != GOSSIP_ID_ALL && listener->id != id) {
        continue;
      }

      GossipCallback callback = listener->callback;
      callback(id, listener->subscriberdata, userdata);
    }

    if (group_id != GOSSIP_GROUP_ALL) {
      break;
    }
  }
  //printf("emitted.\n");
}
