#pragma once

typedef void (*GossipCallback)(int32_t id, void *subscriberdata, void *userdata);

#define GOSSIP_ID_MAX 0xffff
typedef enum {
  GOSSIP_RESULT_OK = 1,
  GOSSIP_RESULT_OUT_OF_RANGE,
} GossipResult;

GossipResult gossip_subscribe(uint32_t id, GossipCallback callback, void *subscriberdata);
GossipResult gossip_emit(uint32_t id, void *userdata);
void gossip_cleanup();
