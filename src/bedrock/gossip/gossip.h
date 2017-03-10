#pragma once

typedef void (*GossipCallback)(void *userdata);

#define GOSSIP_ID_MAX 0xffff
typedef enum {
  GOSSIP_ID_CLOSE = 0x0001,
  GOSSIP_ID_INPUT_KEY,

  GOSSIP_ID_LAST,
} GossipID;

void gossip_subscribe(GossipID id, GossipCallback callback);
void gossip_emit(GossipID id, void *userdata);
void gossip_cleanup();
