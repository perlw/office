#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "fmod/fmod.h"
#include "fmod/fmod_errors.h"

#include "occulus/occulus.h"

#include "boombox.h"

struct Boombox {
  bool init;
  struct {
    FMOD_SYSTEM *system;
    FMOD_DSP *dsp;
  } fmod;
};

struct BoomboxCassette {
  bool loaded;
  bool playing;
  Boombox *parent;
  struct {
    FMOD_SOUND *sound;
    FMOD_CHANNEL *channel;
  } fmod;
};
