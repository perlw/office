#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rectify/rectify.h"
#include "tome.h"

typedef struct {
  char *name;
  void *data;
  uintmax_t refs;
} Record;

typedef struct {
  int32_t type;
  TomeLoader loader;
  TomeDestroyer destroyer;
  Record *records;
} Handler;

typedef struct {
  Handler *handlers;
} Tome;
Tome *tome = NULL;

void tome_init(void) {
  if (!tome) {
    tome = calloc(1, sizeof(Tome));

    tome->handlers = rectify_array_alloc(10, sizeof(Handler));
  }
}

void tome_kill(void) {
  assert(tome);

  for (uintmax_t t = 0; t < rectify_array_size(tome->handlers); t++) {
    for (uintmax_t u = 0; u < rectify_array_size(tome->handlers[t].records); u++) {
      if (tome->handlers[t].records[u].data) { // Workaround until list is pruned
        free(tome->handlers[t].records[u].name);
        tome->handlers[t].destroyer(tome->handlers[t].records[u].data);
      }
    }
    rectify_array_free(tome->handlers[t].records);
  }
  rectify_array_free(tome->handlers);

  free(tome);
}

void *tome_fetch(int32_t type, const char *name, const char *path) {
  assert(tome);

  printf("TOME: Fetching #%d asset \"%s\"@\"%s\"...", type, name, path);
  for (uintmax_t t = 0; t < rectify_array_size(tome->handlers); t++) {
    if (tome->handlers[t].type == type) {
      for (uintmax_t u = 0; u < rectify_array_size(tome->handlers[t].records); u++) {
        if (strcmp(tome->handlers[t].records[u].name, name) == 0) {
          tome->handlers[t].records[u].refs++;
          printf("FOUND, now %" PRIuMAX " refs\n", tome->handlers[t].records[u].refs);
          return tome->handlers[t].records[u].data;
        }
      }

      void *data = tome->handlers[t].loader(name, path);
      if (!data) {
        printf("HANDLER FAILED\n");
        return NULL;
      }
      printf("LOADED\n");

      tome_record(type, name, data);

      return data;
    }
  }
  printf("NO SUCH HANDLER\n");

  return NULL;
}

void tome_record(int32_t type, const char *name, const void *data) {
  assert(tome);

  printf("TOME: Recording #%d asset \"%s\"...", type, name);
  for (uintmax_t t = 0; t < rectify_array_size(tome->handlers); t++) {
    if (tome->handlers[t].type == type) {
      for (uintmax_t u = 0; u < rectify_array_size(tome->handlers[t].records); u++) {
        if (strcmp(tome->handlers[t].records[u].name, name) == 0) {
          printf("ALREADY RECORDED\n");
          return;
        }
      }

      Record record = {
        .name = calloc(strlen(name) + 1, sizeof(char)),
        .data = (void *)data,
        .refs = 1,
      };
      memcpy(record.name, name, sizeof(char) * (strlen(name) + 1));

      tome->handlers[t].records = rectify_array_push(tome->handlers[t].records, &record);

      printf("DONE\n");
      return;
    }
  }
  printf("NO SUCH HANDLER\n");
}

// TODO: Remains in list, need to remove
void tome_erase(int32_t type, const char *name) {
  printf("TOME: Destroying #%d asset \"%s\"...", type, name);
  for (uintmax_t t = 0; t < rectify_array_size(tome->handlers); t++) {
    if (tome->handlers[t].type == type) {
      for (uintmax_t u = 0; u < rectify_array_size(tome->handlers[t].records); u++) {
        if (strcmp(tome->handlers[t].records[u].name, name) == 0) {
          tome->handlers[t].records[u].refs--;

          if (tome->handlers[t].records[u].refs == 0) {
            printf("KILLED\n");
            free(tome->handlers[t].records[u].name);
            tome->handlers[t].destroyer(tome->handlers[t].records[u].data);

            // Workaround until list is pruned
            tome->handlers[t].records[u].data = NULL;
            return;
          }

          printf("DONE, now %" PRIuMAX " refs\n", tome->handlers[t].records[u].refs);
          return;
        }
      }

      printf("NOT FOUND\n");
      return;
    }
  }
  printf("NO SUCH HANDLER\n");
}

void tome_handler(int32_t type, TomeLoader loader, TomeDestroyer destroyer) {
  assert(tome);

  printf("TOME: Registering #%d loader...", type);
  for (uintmax_t t = 0; t < rectify_array_size(tome->handlers); t++) {
    if (tome->handlers[t].type == type) {
      printf("FOUND, ignoring...\n");
      return;
    }
  }
  tome->handlers = rectify_array_push(tome->handlers, &(Handler){
                                                        .type = type,
                                                        .loader = loader,
                                                        .destroyer = destroyer,
                                                        .records = rectify_array_alloc(10, sizeof(Record)),
                                                      });
  printf("DONE\n");
}
