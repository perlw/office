#pragma once

typedef void (*GossipCallback)(void *subscriberdata, void *userdata);

#define GOSSIP_ID_MAX 0xffff
typedef enum {
  GOSSIP_ID_CLOSE = 0x0001,
  GOSSIP_ID_INPUT_KEY,

  GOSSIP_ID_LAST,
} GossipID;

typedef enum {
  GOSSIP_RESULT_OK = 1,
  GOSSIP_RESULT_OUT_OF_RANGE,
} GossipResult;

GossipResult gossip_subscribe(GossipID id, GossipCallback callback, void *subscriberdata);
GossipResult gossip_emit(GossipID id, void *userdata);
void gossip_cleanup();
