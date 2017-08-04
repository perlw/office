/**
 * Gossip - message passer
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../kronos/kronos.h"

#include "g_types.h"

void gossip_init(void);
void gossip_kill(void);

void gossip_register_system(KronosSystem *const system);
void gossip_unregister_system(KronosSystem *const system);
void gossip_post(const char *system, uint32_t id, RectifyMap *const map);
void gossip_emit(uint32_t id, RectifyMap *const map);

void gossip_update(void);
