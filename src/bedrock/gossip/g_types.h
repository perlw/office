#pragma once

typedef uintptr_t GossipHandle;
typedef void (*GossipCallback)(uint32_t group_id, uint32_t id, void *const subscriberdata, void *const userdata);

#define GOSSIP_GROUP_ALL 0x0
#define GOSSIP_ID_ALL 0x0
