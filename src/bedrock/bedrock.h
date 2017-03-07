#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "occulus/occulus.h"
#include "muse/muse.h"
#include "archivist/archivist.h"
#include "gossip/gossip.h"
#include "picasso/picasso.h"

int bedrock_init(const char *title, uint32_t res_width, uint32_t res_height, bool gl_debug);
void bedrock_kill();

void bedrock_swap();
void bedrock_poll();
int bedrock_should_close();
double bedrock_time();
