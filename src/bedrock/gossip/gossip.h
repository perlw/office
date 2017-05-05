#pragma once

#include <stdint.h>

typedef uintptr_t GossipHandle;
typedef void (*GossipCallback)(int32_t id, void *subscriberdata, void *userdata);

#define GOSSIP_ID_MAX 0xffff

void gossip_init(void);
void gossip_destroy(void);

GossipHandle gossip_subscribe(uint32_t id, GossipCallback callback, void *subscriberdata);
void gossip_unsubscribe(uint32_t id, GossipHandle handle);

void gossip_emit(uint32_t id, void *userdata);
