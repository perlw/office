#pragma once

#include <stdint.h>

typedef uintptr_t GossipHandle;
typedef void (*GossipCallback)(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata);

#define GOSSIP_GROUP_ALL 0x0
#define GOSSIP_ID_ALL 0x0

void gossip_init(void);
void gossip_destroy(void);

GossipHandle gossip_subscribe(uint32_t group_id, uint32_t id, GossipCallback callback, void *const subscriberdata, void *const filter);
void gossip_unsubscribe(GossipHandle handle);

void gossip_emit(uint32_t group_id, uint32_t id, void *const self, void *const userdata);
