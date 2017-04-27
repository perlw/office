#pragma once

#include <stdint.h>

typedef struct SoundSys SoundSys;

SoundSys *soundsys_create(void);
void soundsys_destroy(SoundSys *soundsys);
void soundsys_update(SoundSys *soundsys, double delta);
