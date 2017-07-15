#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "occulus/occulus.h"

#include "g_types.h"
#include "g_words.h"
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

  Group *group = NULL;
  for (uintmax_t t = 0; t < rectify_array_size(gossip->groups); t++) {
    Group *const g = &gossip->groups[t];

    if (g->id != group_id) {
      continue;
    }

    group = g;
  }

  if (!group) {
    group = &(Group){
      .id = group_id,
      .listeners = rectify_array_alloc(10, sizeof(Listener)),
    };
    gossip->groups = rectify_array_push(gossip->groups, group);
  }

  group->listeners = rectify_array_push(group->listeners, &listener);

  Listener *pushed_listener = &group->listeners[rectify_array_size(group->listeners) - 1];
  pushed_listener->handle = (uintptr_t)pushed_listener;

  return pushed_listener->handle;
}

bool gossip_unsubscribe(GossipHandle handle) {
  assert(gossip);

  for (uintmax_t t = 0; t < rectify_array_size(gossip->groups); t++) {
    Group *const group = &gossip->groups[t];

    for (uintmax_t u = 0; u < rectify_array_size(group->listeners); u++) {
      Listener *const listener = &group->listeners[u];

      if (listener->handle == handle) {
        group->listeners = rectify_array_delete(group->listeners, u);
        return true;
      }
    }
  }
  return false;
}

void gossip_emit(uint32_t group_id, uint32_t id, void *const userdata) {
  assert(gossip);

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
      callback(group_id, id, listener->subscriberdata, userdata);
    }
  }
}

void handle_to_word(GossipHandle handle, uintmax_t max_len, char *buffer) {
  uint32_t a = (handle >> 24) & 0xff;
  uint32_t b = (handle >> 16) & 0xff;
  uint32_t c = (handle >> 8) & 0xff;
  uint32_t d = handle & 0xff;
  snprintf(buffer, max_len, "%d %s %s %s", a, COLORS[b % NUM_COLORS], ADJECTIVES[c % NUM_ADJECTIVES], SUBJECTS[d % NUM_SUBJECTS]);
}

GossipHandle gossip_subscribe_debug(uint32_t group_id, uint32_t id, GossipCallback callback, void *const subscriberdata, const char *filepath, uintmax_t line, const char *function) {
  GossipHandle handle = gossip_subscribe(group_id, id, callback, subscriberdata);

  char buffer[256];
  handle_to_word(handle, 256, buffer);
  printf("Gossip:(%s:%" PRIuPTR "/%s) Subscribing to %d:%d, handle is [%s] (%p)\n", filepath, line, function, group_id, id, buffer, (void *)handle);

  return handle;
}

bool gossip_unsubscribe_debug(GossipHandle handle, const char *filepath, uintmax_t line, const char *function) {
  char buffer[256];
  handle_to_word(handle, 256, buffer);
  printf("Gossip:(%s:%" PRIuPTR "/%s) Unsubscribing [%s] (%p)... ", filepath, line, function, buffer, (void *)handle);

  bool result = gossip_unsubscribe(handle);
  if (result) {
    printf("unsubscribed.\n");
  } else {
    printf("handle not found.\n");
  }
  return result;
}
