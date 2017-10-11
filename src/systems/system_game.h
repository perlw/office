#pragma once

#define USE_KRONOS
#include "bedrock/bedrock.h"

extern KronosSystem system_game;

bool system_game_should_kill(void);
