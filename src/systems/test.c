#include <assert.h>
#include <stdio.h>
#include <string.h>

#define USE_KRONOS
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#define USE_CONFIG
#define USE_MESSAGES
#include "main.h"

typedef struct {
  int dummy;
} SystemTest;

SystemTest *system_test_start(void);
void system_test_stop(void **system);
void system_test_update(SystemTest *system, double delta);
RectifyMap *system_test_message(SystemTest *system, uint32_t id, RectifyMap *const map);

KronosSystem system_test = {
  .name = "test",
  .frames = 0,
  .autostart = false,
  .start = &system_test_start,
  .stop = &system_test_stop,
  .update = &system_test_update,
  .message = &system_test_message,
};

SystemTest *system_test_start(void) {
  Config *const config = config_get();

  SystemTest *system = calloc(1, sizeof(SystemTest));
  *system = (SystemTest){};

  return system;
}

void system_test_stop(void **system) {
  SystemTest *ptr = *system;
  assert(ptr && system);

  free(ptr);
  *system = NULL;
}

void system_test_update(SystemTest *system, double delta) {
  assert(system);
}

RectifyMap *system_test_message(SystemTest *system, uint32_t id, RectifyMap *const map) {
  assert(system);

  switch (id) {
    case MSG_DEBUG_TEST: {
      RectifyMap *map = rectify_map_create();
      rectify_map_set_string(map, "foo", "bar");
      rectify_map_set_uint(map, "life", 42);
      return map;
    }
  }

  return NULL;
}
