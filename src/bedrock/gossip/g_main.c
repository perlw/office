#include <stdio.h>
#include <stdint.h>

#include "gossip.h"
#include "occulus/occulus.h"

typedef struct {
	size_t max_listeners;
	size_t num_listeners;
	GossipCallback *listeners;
} Gossiper;
Gossiper gossipers[GOSSIP_ID_MAX] = { { 0, 0, NULL } };

#define LISTENER_CHUNK 4

void gossip_subscribe(GossipID id, GossipCallback callback) {
	Gossiper *g = &gossipers[id];

	if (!g->listeners) {
		g->max_listeners = LISTENER_CHUNK;
		g->listeners = malloc(sizeof(GossipCallback) * g->max_listeners);
	} else if (g->num_listeners == g->max_listeners) {
		g->max_listeners += LISTENER_CHUNK;
		g->listeners = realloc(g->listeners, sizeof(GossipCallback) * g->max_listeners);
	}

	g->listeners[g->num_listeners] = callback;
	g->num_listeners += 1;
}

void gossip_emit(GossipID id) {
	Gossiper *g = &gossipers[id];

	for (size_t t = 0; t < g->num_listeners; t += 1) {
		g->listeners[t]();
	}
}

void gossip_cleanup() {
	for (size_t t = 0; t < GOSSIP_ID_MAX; t += 1) {
		if (gossipers[t].listeners) {
			free(gossipers[t].listeners);
		}
	}
}
