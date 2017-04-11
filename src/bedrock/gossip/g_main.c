#include <stdio.h>
#include <stdint.h>

#include "gossip.h"
#include "occulus/occulus.h"

typedef struct {
  void *subscriberdata;
  GossipCallback callback;
} Listener;

typedef struct {
  size_t max_listeners;
  size_t num_listeners;
  Listener *listeners;
} Gossiper;
Gossiper gossipers[GOSSIP_ID_MAX] = { { 0, 0, NULL } };

#define LISTENER_CHUNK 4

GossipResult gossip_subscribe(GossipID id, GossipCallback callback, void *subscriberdata) {
  if (id < 1 || id > GOSSIP_ID_MAX) {
    return GOSSIP_RESULT_OUT_OF_RANGE;
  }

  Gossiper *g = &gossipers[id];

  if (!g->listeners) {
    g->max_listeners = LISTENER_CHUNK;
    g->listeners = calloc(g->max_listeners, sizeof(Listener));
  } else if (g->num_listeners == g->max_listeners) {
    g->max_listeners += LISTENER_CHUNK;
    g->listeners = realloc(g->listeners, sizeof(Listener) * g->max_listeners);
  }

  g->listeners[g->num_listeners].subscriberdata = subscriberdata;
  g->listeners[g->num_listeners].callback = callback;
  g->num_listeners += 1;

  return GOSSIP_RESULT_OK;
}

GossipResult gossip_emit(GossipID id, void *userdata) {
  if (id < 1 || id > GOSSIP_ID_MAX) {
    return GOSSIP_RESULT_OUT_OF_RANGE;
  }

  Gossiper *g = &gossipers[id];

  for (size_t t = 0; t < g->num_listeners; t += 1) {
    GossipCallback callback = g->listeners[t].callback;
    callback(id, g->listeners[t].subscriberdata, userdata);
  }

  return GOSSIP_RESULT_OK;
}

void gossip_cleanup() {
  for (size_t t = 0; t < GOSSIP_ID_MAX; t += 1) {
    if (gossipers[t].listeners) {
      free(gossipers[t].listeners);
    }
  }
}
