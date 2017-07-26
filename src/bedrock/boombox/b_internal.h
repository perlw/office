#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "fmod/fmod.h"
#include "fmod/fmod_errors.h"

#include "occulus/occulus.h"

#include "boombox.h"

typedef struct {
  struct {
    FMOD_SYSTEM *system;
    FMOD_DSP *dsp;
  } fmod;
} Boombox;

extern Boombox *boombox;

struct BoomboxCassette {
  bool loaded;
  bool playing;

  struct {
    FMOD_SOUND *sound;
    FMOD_CHANNEL *channel;
  } fmod;
};
