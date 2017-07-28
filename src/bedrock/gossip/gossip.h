/**
 * Gossip - message passer
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "kronos/kronos.h"

#include "g_types.h"

void gossip_init(void);
void gossip_kill(void);

void gossip_register_system(KronosSystem *const system);
void gossip_unregister_system(KronosSystem *const system);
void gossip_post(const char *system, uint32_t id, void *dummy);
void gossip_emit(uint32_t id, void *dummy);

void gossip_update(void);

/*
GossipHandle gossip_subscribe(const char *message, GossipCallback callback, void *const subscriberdata);
bool gossip_unsubscribe(GossipHandle handle);
void gossip_gc(void);

void gossip_update(void);

void gossip_emit(const char *message, uintmax_t size, void *const userdata);

#ifdef GOSSIP_DEBUG
GossipHandle gossip_subscribe_debug(const char *message, GossipCallback callback, void *const subscriberdata, const char *filepath, uintmax_t line, const char *function);
bool gossip_unsubscribe_debug(GossipHandle handle, const char *filepath, uintmax_t line, const char *function);

#define gossip_subscribe(a, b, c) gossip_subscribe_debug(a, b, c, __FILE__, __LINE__, __func__)
#define gossip_unsubscribe(a) gossip_unsubscribe_debug(a, __FILE__, __LINE__, __func__)
#endif // GOSSIP_DEBUG
*/
