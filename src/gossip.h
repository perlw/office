#ifndef __GOSSIP_H__
#define __GOSSIP_H__

typedef void (*GossipCallback)(void);

typedef enum {
	GOSSIP_BEDROCK_CLOSE = 0x0001,

	GOSSIP_MAX,
} GossipID;

void gossip_subscribe(GossipID id, GossipCallback callback);
void gossip_emit(GossipID id);
void gossip_cleanup();

#endif // __GOSSIP_H__
