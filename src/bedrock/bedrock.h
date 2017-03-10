#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "occulus/occulus.h"
#include "muse/muse.h"
#include "neglect/neglect.h"
#include "archivist/archivist.h"
#include "gossip/gossip.h"
#include "picasso/picasso.h"

typedef struct {
  int32_t key;
  int32_t scancode;
  bool press;
  bool release;
} BedrockKeyboardEvent;

int bedrock_init(const char *title, uint32_t res_width, uint32_t res_height, bool gl_debug);
void bedrock_kill();

void bedrock_swap();
void bedrock_poll();
int bedrock_should_close();
double bedrock_time();
