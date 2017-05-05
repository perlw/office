#include <stdint.h>
#include <stdio.h>

#include "occulus/occulus.h"

#include "gossip.h"
#include "rectify/rectify.h"

typedef struct {
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

GossipResult gossip_subscribe(uint32_t id, GossipCallback callback, void *subscriberdata) {
  if (id < 1 || id > GOSSIP_ID_MAX) {
    return GOSSIP_RESULT_OUT_OF_RANGE;
  }

  Gossiper *g = &gossipers[id];
  if (!g->listeners) {
    g->listeners = rectify_array_alloc(4, sizeof(Listener));
  }
  g->listeners = rectify_array_push(g->listeners, &(Listener){
                                                    .subscriberdata = subscriberdata,
                                                    .callback = callback,
                                                  });

  return GOSSIP_RESULT_OK;
}

GossipResult gossip_emit(uint32_t id, void *userdata) {
  if (id < 1 || id > GOSSIP_ID_MAX) {
    return GOSSIP_RESULT_OUT_OF_RANGE;
  }

  Gossiper *g = &gossipers[id];

  for (size_t t = 0; t < rectify_array_size(g->listeners); t += 1) {
    GossipCallback callback = g->listeners[t].callback;
    callback(id, g->listeners[t].subscriberdata, userdata);
  }

  return GOSSIP_RESULT_OK;
}
