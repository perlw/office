#pragma once

#define USE_KRONOS
#define USE_PICASSO
#include "bedrock/bedrock.h"

extern KronosSystem system_input;

void system_input_bind_input(PicassoWindow *window);
