#pragma once

typedef void (*GossipCallback)(void);

typedef enum {
  GOSSIP_ID_CLOSE = 0x0001,

  GOSSIP_ID_MAX,
} GossipID;

void gossip_subscribe(GossipID id, GossipCallback callback);
void gossip_emit(GossipID id);
void gossip_cleanup();
