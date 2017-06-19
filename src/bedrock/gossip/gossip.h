#pragma once

#include <stdint.h>

typedef uintptr_t GossipHandle;
typedef void (*GossipCallback)(int32_t id, void *const subscriberdata, void *const userdata);

#define GOSSIP_ID_MAX 0xffff

void gossip_init(void);
void gossip_destroy(void);

GossipHandle gossip_subscribe(uint32_t id, GossipCallback callback, void *const subscriberdata);
void gossip_unsubscribe(uint32_t id, GossipHandle handle);

void gossip_emit(uint32_t id, void *const userdata);
