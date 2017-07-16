#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "occulus/occulus.h"

#include "g_types.h"
#include "g_words.h"
#include "rectify/rectify.h"

typedef struct {
  char *group_id;
  char *id;
  bool delete;
  GossipHandle handle;
  void *subscriberdata;
  GossipCallback callback;
} Listener;

typedef struct {
  Listener *listeners;
  bool dirty;
} Gossip;
Gossip *gossip = NULL;

void gossip_init(void) {
  if (gossip) {
    return;
  }

  gossip = calloc(1, sizeof(Gossip));
  *gossip = (Gossip){
    .listeners = rectify_array_alloc(10, sizeof(Listener)),
    .dirty = false,
  };
}

void gossip_destroy(void) {
  assert(gossip);

  for (uintmax_t t = 0; t < rectify_array_size(gossip->listeners); t++) {
    Listener *const listener = &gossip->listeners[t];
    if (listener->group_id) {
      free(listener->group_id);
    }
    if (listener->id) {
      free(listener->id);
    }
  }
  rectify_array_free(gossip->listeners);

  free(gossip);
}

GossipHandle gossip_subscribe(const char *message, GossipCallback callback, void *const subscriberdata) {
  assert(gossip);

  Listener listener = (Listener){
    .subscriberdata = subscriberdata,
    .callback = callback,
    .delete = false,
  };

  char *message_tokens = rectify_memory_alloc_copy(message, sizeof(char) * (strlen(message) + 1));
  char *message_token_group = strtok(message_tokens, ":");
  char *message_token_id = strtok(NULL, ":");
  listener.group_id = rectify_memory_alloc_copy(message_token_group, sizeof(char) * (strnlen(message_token_group, 128) + 1));
  listener.id = rectify_memory_alloc_copy(message_token_id, sizeof(char) * (strnlen(message_token_id, 128) + 1));
  free(message_tokens);

  gossip->listeners = rectify_array_push(gossip->listeners, &listener);

  Listener *pushed_listener = &gossip->listeners[rectify_array_size(gossip->listeners) - 1];
  pushed_listener->handle = (uintptr_t)pushed_listener;

  return pushed_listener->handle;
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

void gossip_emit(const char *message, void *const userdata) {
  assert(gossip);

  char *message_tokens = rectify_memory_alloc_copy(message, sizeof(char) * (strlen(message) + 1));
  char *message_token_group = strtok(message_tokens, ":");
  char *message_token_id = strtok(NULL, ":");
  bool skip_group_check = (strncmp(message_token_group, "*", 2) == 0);
  bool skip_id_check = (strncmp(message_token_id, "*", 2) == 0);

  // TODO: Wrap in debug, verbosity
  //if (strncmp(message_token_id, "paint", 128) != 0 && strncmp(message_token_id, "spectrum", 128) != 0) {
  //printf("Gossip: Emitting <%s>:<%s>\n", message_token_group, message_token_id);
  //}

  uintmax_t count = 0;
  for (uintmax_t t = 0; t < rectify_array_size(gossip->listeners); t++) {
    Listener *const listener = &gossip->listeners[t];

    if (listener->delete) {
      continue;
    }

    if ((skip_group_check || strncmp(listener->group_id, "*", 2) == 0 || strncmp(listener->group_id, message_token_group, 128) == 0)
        && (skip_id_check || strncmp(listener->id, "*", 2) == 0 || strncmp(listener->id, message_token_id, 128) == 0)) {
      GossipCallback callback = listener->callback;
      callback((skip_group_check ? listener->group_id : message_token_group), (skip_id_check ? listener->id : message_token_id), listener->subscriberdata, userdata);
      count++;
    }
  }

  free(message_tokens);
}

void handle_to_word(GossipHandle handle, uintmax_t max_len, char *buffer) {
  uint32_t a = (handle >> 24) & 0xff;
  uint32_t b = (handle >> 16) & 0xff;
  uint32_t c = (handle >> 8) & 0xff;
  uint32_t d = handle & 0xff;
  snprintf(buffer, max_len, "%d %s %s %s", a, COLORS[b % NUM_COLORS], ADJECTIVES[c % NUM_ADJECTIVES], SUBJECTS[d % NUM_SUBJECTS]);
}

GossipHandle gossip_subscribe_debug(const char *message, GossipCallback callback, void *const subscriberdata, const char *filepath, uintmax_t line, const char *function) {
  GossipHandle handle = gossip_subscribe(message, callback, subscriberdata);

  char buffer[256];
  handle_to_word(handle, 256, buffer);
  printf("Gossip:(%s:%" PRIuPTR "/%s) Subscribing to %s, handle is [%s] (%p)\n", filepath, line, function, message, buffer, (void *)handle);

  return handle;
}

bool gossip_unsubscribe_debug(GossipHandle handle, const char *filepath, uintmax_t line, const char *function) {
  char buffer[256];
  handle_to_word(handle, 256, buffer);
  printf("Gossip:(%s:%" PRIuPTR "/%s) Unsubscribing [%s] (%p)... ", filepath, line, function, buffer, (void *)handle);

  bool result = gossip_unsubscribe(handle);
  if (result) {
    printf("unsubscribed.\n");
  } else {
    printf("handle not found.\n");
  }
  return result;
}
