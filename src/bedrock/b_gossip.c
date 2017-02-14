#include "bedrock.h"

#include <stdio.h>
#include <stdint.h>

typedef struct {
	size_t max_listeners;
	size_t num_listeners;
	BedrockGossipCallbackType* listeners;
} Gossiper;
Gossiper gossipers[BEDROCK_GOSSIP_ID_MAX] = { { 0, 0, NULL } };

#define LISTENER_CHUNK 4

void bedrock_gossip_subscribe(BedrockGossipID id, BedrockGossipCallbackType callback) {
	Gossiper* g = &gossipers[id];

	if (!g->listeners) {
		g->max_listeners = LISTENER_CHUNK;
		g->listeners = malloc(sizeof(BedrockGossipCallbackType) * g->max_listeners);
	} else if (g->num_listeners == g->max_listeners) {
		g->max_listeners += LISTENER_CHUNK;
		g->listeners = realloc(g->listeners, sizeof(BedrockGossipCallbackType) * g->max_listeners);
	}

	g->listeners[g->num_listeners] = callback;
	g->num_listeners += 1;
}

void bedrock_gossip_emit(BedrockGossipID id) {
	Gossiper* g = &gossipers[id];

	for (size_t t = 0; t < g->num_listeners; t += 1) {
		g->listeners[t]();
	}
}

void bedrock_gossip_cleanup() {
	for (size_t t = 0; t < BEDROCK_GOSSIP_ID_MAX; t += 1) {
		if (gossipers[t].listeners) {
			free(gossipers[t].listeners);
		}
	}
}
