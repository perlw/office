#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "occulus/occulus.h"

#include "kronos/kronos.h"
#include "rectify/rectify.h"

#include "g_types.h"
#include "g_words.h"

typedef struct {
  char *system;
  uint32_t id;
  RectifyMap *map;
} QueueItem;

typedef struct {
  KronosSystem **systems;
  QueueItem *queue;
  QueueItem *active_queue;

  bool dirty;
  uint32_t next_handle;
} Gossip;
Gossip *gossip = NULL;

void handle_to_word(GossipHandle handle, uintmax_t max_len, char *buffer) {
  uint32_t a = (handle >> 24) & 0xff;
  uint32_t b = (handle >> 16) & 0xff;
  uint32_t c = (handle >> 8) & 0xff;
  uint32_t d = handle & 0xff;
  snprintf(buffer, max_len, "%d %s %s %s", a, COLORS[b % NUM_COLORS], ADJECTIVES[c % NUM_ADJECTIVES], SUBJECTS[d % NUM_SUBJECTS]);
}

void gossip_init(void) {
  if (gossip) {
    return;
  }

  gossip = calloc(1, sizeof(Gossip));
  *gossip = (Gossip){
    .systems = rectify_array_alloc(10, sizeof(KronosSystem *)),
    .queue = rectify_array_alloc(10, sizeof(QueueItem)),
    .active_queue = rectify_array_alloc(10, sizeof(QueueItem)),
    .dirty = false,
  };
}

void gossip_kill(void) {
  assert(gossip);

  rectify_array_free((void **)&gossip->systems);

  for (uintmax_t t = 0; t < rectify_array_size(gossip->queue); t++) {
    QueueItem *const item = &gossip->queue[t];
    if (item->system) {
      free(item->system);
    }
    rectify_map_destroy(&item->map);
  }
  rectify_array_free((void **)&gossip->queue);
  for (uintmax_t t = 0; t < rectify_array_size(gossip->active_queue); t++) {
    QueueItem *const item = &gossip->active_queue[t];
    if (item->system) {
      free(item->system);
    }
    rectify_map_destroy(&item->map);
  }
  rectify_array_free((void **)&gossip->active_queue);

  free(gossip);
}

void gossip_register_system(KronosSystem *const system) {
  assert(gossip);
  gossip->systems = rectify_array_push(gossip->systems, &system);
}

void gossip_unregister_system(KronosSystem *const system) {
  assert(gossip);

  for (uint32_t t = 0; t < rectify_array_size(gossip->systems); t++) {
    if (strncmp(gossip->systems[t]->name, system->name, 128) == 0) {
      gossip->systems = rectify_array_delete(gossip->systems, t);
      break;
    }
  }
}

void gossip_post(const char *system, uint32_t id, RectifyMap *const map) {
  assert(gossip);
  gossip->active_queue = rectify_array_push(gossip->active_queue, &(QueueItem){
                                                                    .system = rectify_memory_alloc_copy(system, strnlen(system, 128) + 1), .id = id, .map = map,
                                                                  });
}

void gossip_emit(uint32_t id, RectifyMap *const map) {
  assert(gossip);
  gossip->active_queue = rectify_array_push(gossip->active_queue, &(QueueItem){
                                                                    .system = NULL, .id = id, .map = map,
                                                                  });
}

void gossip_update(void) {
  assert(gossip);

  QueueItem *swp = gossip->queue;
  gossip->queue = gossip->active_queue;
  gossip->active_queue = swp;

  uintmax_t queue_size = rectify_array_size(gossip->queue);
  for (uintmax_t t = 0; t < queue_size; t++) {
    QueueItem *const item = &gossip->queue[t];

    if (item->system) {
      for (uint32_t u = 0; u < rectify_array_size(gossip->systems); u++) {
        if (strncmp(gossip->systems[u]->name, item->system, 128) == 0) {
          gossip->systems[u]->message(item->id, item->map);
          free(item->system);
          item->system = NULL;
          break;
        }
      }
    } else {
      for (uint32_t u = 0; u < rectify_array_size(gossip->systems); u++) {
        gossip->systems[u]->message(item->id, item->map);
      }
    }

    rectify_map_destroy(&item->map);
  }

  rectify_array_free((void **)&gossip->queue);
  gossip->queue = rectify_array_alloc(10, sizeof(QueueItem));
}
