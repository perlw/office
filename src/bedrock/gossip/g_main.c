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

  /*for (uintmax_t t = 0; t < rectify_array_size(gossip->listeners); t++) {
    Listener *const listener = &gossip->listeners[t];
    if (listener->group_id) {
      free(listener->group_id);
    }
    if (listener->id) {
      free(listener->id);
    }
  }*/
  rectify_array_free((void **)&gossip->systems);

  for (uintmax_t t = 0; t < rectify_array_size(gossip->queue); t++) {
    QueueItem *const item = &gossip->queue[t];
    rectify_map_destroy(&item->map);
  }
  rectify_array_free(&gossip->queue);
  for (uintmax_t t = 0; t < rectify_array_size(gossip->active_queue); t++) {
    QueueItem *const item = &gossip->active_queue[t];
    rectify_map_destroy(&item->map);
  }
  rectify_array_free(&gossip->active_queue);

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
}

void gossip_emit(uint32_t id, RectifyMap *const map) {
  assert(gossip);

  gossip->active_queue = rectify_array_push(gossip->active_queue, &(QueueItem){
                                                                    .id = id,
                                                                    .map = map,
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

    for (uint32_t u = 0; u < rectify_array_size(gossip->systems); u++) {
      gossip->systems[u]->message(item->id, item->map);
    }

    rectify_map_destroy(&item->map);
  }

  rectify_array_free(&gossip->queue);
  gossip->queue = rectify_array_alloc(10, sizeof(QueueItem));
}

/*GossipHandle gossip_subscribe(const char *message, GossipCallback callback, void *const subscriberdata) {
  assert(gossip);

  Listener listener = (Listener){
    .subscriberdata = subscriberdata,
    .callback = callback,
    .delete = false,
    .handle = gossip->next_handle,
  };
  if (gossip->next_handle + 1 < gossip->next_handle) {
    printf("Gossip: Warning! Handles wrapped around!\n");
  }
  gossip->next_handle++;

  char *message_tokens = rectify_memory_alloc_copy(message, sizeof(char) * (strlen(message) + 1));
  char *message_token_group = strtok(message_tokens, ":");
  char *message_token_id = strtok(NULL, ":");
  listener.group_id = rectify_memory_alloc_copy(message_token_group, sizeof(char) * (strnlen(message_token_group, 128) + 1));
  listener.id = rectify_memory_alloc_copy(message_token_id, sizeof(char) * (strnlen(message_token_id, 128) + 1));
  free(message_tokens);

  gossip->listeners = rectify_array_push(gossip->listeners, &listener);

  //Listener *pushed_listener = &gossip->listeners[rectify_array_size(gossip->listeners) - 1];

  return listener.handle;
}

bool gossip_unsubscribe(GossipHandle handle) {
  assert(gossip);

  for (uintmax_t t = 0; t < rectify_array_size(gossip->listeners); t++) {
    Listener *const listener = &gossip->listeners[t];

    if (listener->handle == handle) {
      listener->delete = true;
      gossip->dirty = true;
      return true;
    }
  }
  return false;
}

void gossip_gc(void) {
  assert(gossip);

  if (!gossip->dirty) {
    return;
  }
  gossip->dirty = false;

  uintmax_t count = 0;
  Listener *listeners = rectify_array_alloc(10, sizeof(Listener));
  for (uintmax_t t = 0; t < rectify_array_size(gossip->listeners); t++) {
    Listener *const listener = &gossip->listeners[t];

    if (!listener->delete) {
      listeners = rectify_array_push(listeners, listener);
    } else {
      char buffer[256];
      handle_to_word(listener->handle, 256, buffer);
      printf("Gossip: Cleaning up handle [%s] (%d)\n", buffer, listener->handle);

      free(listener->group_id);
      listener->group_id = NULL;
      free(listener->id);
      listener->id = NULL;
      count++;
    }
  }
  rectify_array_free(gossip->listeners);
  gossip->listeners = listeners;

  printf("Gossip: GC pass, cleaned out %" PRIuMAX " dead handles, new count %" PRIuMAX "/%" PRIuMAX "\n", count, rectify_array_size(gossip->listeners), rectify_array_cap(gossip->listeners));
}

void gossip_update(void) {
  assert(gossip);

  gossip_gc();

  QueueItem *swp = gossip->queue;
  gossip->queue = gossip->active_queue;
  gossip->active_queue = swp;

  uintmax_t queue_size = rectify_array_size(gossip->queue);
  //if (queue_size > 0) {
  //printf("Gossip: Running queue, %" PRIuMAX " items\n", queue_size);
  //}

  for (uintmax_t t = 0; t < queue_size; t++) {
    QueueItem *const item = &gossip->queue[t];

    char *message_tokens = rectify_memory_alloc_copy(item->message, sizeof(char) * (strlen(item->message) + 1));
    char *message_token_group = strtok(message_tokens, ":");
    char *message_token_id = strtok(NULL, ":");
    bool skip_group_check = (strncmp(message_token_group, "*", 2) == 0);
    bool skip_id_check = (!message_token_id || strncmp(message_token_id, "*", 2) == 0);

    //printf("%s (%s:%s) %d %d\n", item->message, message_token_group, message_token_id, skip_group_check, skip_id_check);

    uint32_t count = 0;
    for (uintmax_t t = 0; t < rectify_array_size(gossip->listeners); t++) {
      Listener *const listener = &gossip->listeners[t];

      if (listener->delete) {
        continue;
      }

      //printf("\t%s:%s\n", listener->group_id, listener->id);

      if ((skip_group_check || strncmp(listener->group_id, "*", 2) == 0 || strncmp(listener->group_id, message_token_group, 128) == 0)
          && (skip_id_check || strncmp(listener->id, "*", 2) == 0 || strncmp(listener->id, message_token_id, 128) == 0)) {
        GossipCallback callback = listener->callback;
        callback((skip_group_check ? listener->group_id : message_token_group), (skip_id_check ? listener->id : message_token_id), listener->subscriberdata, item->userdata);
        count++;
      }
    }
    if (count == 0) {
      printf("Gossip: no one were listening? \"%s\"\n", item->message);
    }

    free(message_tokens);

    free(item->message);
    if (item->userdata) {
      free(item->userdata);
    }
  }
  rectify_array_free(gossip->queue);
  gossip->queue = rectify_array_alloc(10, sizeof(QueueItem));
}

void gossip_emit(const char *message, uintmax_t size, void *const userdata) {
  assert(gossip);

  // TODO: Wrap in debug, verbosity
  //if (strncmp(message_token_id, "paint", 128) != 0 && strncmp(message_token_id, "spectrum", 128) != 0) {
  //printf("Gossip: Emitting <%s>:<%s>\n", message_token_group, message_token_id);
  //}

  //printf("Gossip: Emitting %s -> %p\n", message, userdata);
  gossip->active_queue = rectify_array_push(gossip->active_queue, &(QueueItem){
                                                                    .message = rectify_memory_alloc_copy(message, strnlen(message, 128) + 1), .userdata = (size > 0 || userdata ? rectify_memory_alloc_copy(userdata, size) : NULL),
                                                                  });
}

GossipHandle gossip_subscribe_debug(const char *message, GossipCallback callback, void *const subscriberdata, const char *filepath, uintmax_t line, const char *function) {
  GossipHandle handle = gossip_subscribe(message, callback, subscriberdata);

  char buffer[256];
  handle_to_word(handle, 256, buffer);
  printf("Gossip:(%s:%" PRIuPTR "/%s) Subscribing to %s, handle is [%s] (%d)\n", filepath, line, function, message, buffer, handle);

  return handle;
}

bool gossip_unsubscribe_debug(GossipHandle handle, const char *filepath, uintmax_t line, const char *function) {
  char buffer[256];
  handle_to_word(handle, 256, buffer);
  printf("Gossip:(%s:%" PRIuPTR "/%s) Unsubscribing [%s] (%d)... ", filepath, line, function, buffer, handle);

  bool result = gossip_unsubscribe(handle);
  if (result) {
    printf("unsubscribed.\n");
  } else {
    printf("handle not found.\n");
  }
  return result;
}*/
