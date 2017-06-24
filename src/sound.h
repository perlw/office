#pragma once

#include <stdint.h>

typedef struct SoundSys SoundSys;

typedef struct {
  float left[2048];
  float right[2048];
} Spectrum;

SoundSys *soundsys_create(void);
void soundsys_destroy(SoundSys *soundsys);
void soundsys_update(SoundSys *soundsys, double delta);
