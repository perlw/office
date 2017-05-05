#include <stdint.h>
#include <stdio.h>

#include "occulus/occulus.h"

#include "gossip.h"
#include "rectify/rectify.h"

typedef struct {
  GossipHandle handle;
  void *subscriberdata;
  GossipCallback callback;
} Listener;

typedef struct {
  Listener *listeners;
} Gossiper;
Gossiper gossipers[GOSSIP_ID_MAX];

void gossip_init(void) {
  for (uintmax_t t = 0; t < GOSSIP_ID_MAX; t += 1) {
    gossipers[t].listeners = NULL;
  }
}

void gossip_destroy(void) {
  for (uintmax_t t = 0; t < GOSSIP_ID_MAX; t += 1) {
    if (gossipers[t].listeners) {
      rectify_array_free(gossipers[t].listeners);
    }
  }
}

GossipHandle gossip_subscribe(uint32_t id, GossipCallback callback, void *subscriberdata) {
  if (id > GOSSIP_ID_MAX) {
    printf("GOSSIP: Warning, ignored subscribing to %d id, too large.\n", id);
    return 0;
  }

  Gossiper *g = &gossipers[id];
  if (!g->listeners) {
    g->listeners = rectify_array_alloc(4, sizeof(Listener));
  }

  Listener listener = (Listener){
    .subscriberdata = subscriberdata,
    .callback = callback,
  };
  listener.handle = (uintptr_t)&listener;
  g->listeners = rectify_array_push(g->listeners, &listener);

  return listener.handle;
}

void gossip_unsubscribe(uint32_t id, GossipHandle handle) {
  if (id > GOSSIP_ID_MAX) {
    printf("GOSSIP: Warning, ignored subscribing to %d id, too large.\n", id);
    return;
  }

  Gossiper *g = &gossipers[id];
  for (uintmax_t t = 0; t < rectify_array_size(g->listeners); t += 1) {
    if (g->listeners[t].handle == handle) {
      g->listeners = rectify_array_delete(g->listeners, t);
      break;
    }
  }
}

void gossip_emit(uint32_t id, void *userdata) {
  if (id > GOSSIP_ID_MAX) {
    printf("GOSSIP: Warning, ignored emitting %d id, too large.\n", id);
    return;
  }

  Gossiper *g = &gossipers[id];

  for (uintmax_t t = 0; t < rectify_array_size(g->listeners); t += 1) {
    GossipCallback callback = g->listeners[t].callback;
    callback(id, g->listeners[t].subscriberdata, userdata);
  }
}
