#pragma once

#include <stdint.h>

typedef void (*GossipCallback)(int32_t id, void *subscriberdata, void *userdata);

#define GOSSIP_ID_MAX 0xffff
typedef enum {
  GOSSIP_RESULT_OK = 1,
  GOSSIP_RESULT_OUT_OF_RANGE,
} GossipResult;

void gossip_init(void);
void gossip_destroy(void);

GossipResult gossip_subscribe(uint32_t id, GossipCallback callback, void *subscriberdata);
GossipResult gossip_emit(uint32_t id, void *userdata);
