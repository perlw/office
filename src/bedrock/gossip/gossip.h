#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "g_types.h"

void gossip_init(void);
void gossip_destroy(void);

GossipHandle gossip_subscribe(uint32_t group_id, uint32_t id, GossipCallback callback, void *const subscriberdata, void *const filter);
bool gossip_unsubscribe(GossipHandle handle);

void gossip_emit(uint32_t group_id, uint32_t id, void *const self, void *const userdata);

#ifdef GOSSIP_DEBUG
GossipHandle gossip_subscribe_debug(uint32_t group_id, uint32_t id, GossipCallback callback, void *const subscriberdata, void *const filter, const char *filepath, uintmax_t line, const char *function);
bool gossip_unsubscribe_debug(GossipHandle handle, const char *filepath, uintmax_t line, const char *function);

#define gossip_subscribe(a, b, c, d, e) gossip_subscribe_debug(a, b, c, d, e, __FILE__, __LINE__, __func__)
#define gossip_unsubscribe(a) gossip_unsubscribe_debug(a, __FILE__, __LINE__, __func__)
#endif // GOSSIP_DEBUG
