#ifndef __BEDROCK_H__
#define __BEDROCK_H__

#include "occulus/occulus.h"
#include "archivist/archivist.h"
#include "gossip/gossip.h"
#include "picasso/picasso.h"

int bedrock_init();
void bedrock_kill();

void bedrock_swap();
void bedrock_poll();
int bedrock_should_close();
double bedrock_time();

#endif // __BEDROCK_H__
