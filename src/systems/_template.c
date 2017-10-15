#include <assert.h>
#include <stdio.h>
#include <string.h>

#define USE_KRONOS
#include "bedrock/bedrock.h"

#define USE_CONFIG
#define USE_MESSAGES
#include "main.h"

// HOWTO: Search/Replace Template and template with correct values.

typedef struct {
  int dummy;
} SystemTemplate;

SystemTemplate *system_template_start(void);
void system_template_stop(void **system);
void system_template_update(SystemTemplate *system, double delta);
void system_template_message(SystemTemplate *system, uint32_t id, RectifyMap *const map);

KronosSystem system_template = {
  .name = "template",
  .frames = 0,
  .prevent_stop = true || false,
  .autostart = true || false,
  .start = &system_template_start,
  .stop = &system_template_stop,
  .update = &system_template_update,
  .message = &system_template_message,
};

SystemTemplate *system_template_start(void) {
  Config *const config = config_get();

  SystemTemplate *system = calloc(1, sizeof(SystemTemplate));
  *system = (SystemTemplate){};

  return system;
}

void system_template_stop(SystemTemplate **system) {
  SystemTemplate *ptr = *system;
  assert(ptr && system);

  free(ptr);
  *system = NULL;
}

void system_template_update(SystemTemplate *system, double delta) {
  assert(system);
}

void system_template_message(SystemTemplate *system, uint32_t id, RectifyMap *const map) {
  assert(system);

  switch (id) {
  }
}
