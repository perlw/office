#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "fmod/fmod.h"
#include "fmod/fmod_errors.h"

#include "occulus/occulus.h"

#include "boombox.h"

struct Boombox {
  bool init;
  struct {
    FMOD_SYSTEM *system;
  } fmod;
};

struct BoomboxCassette {
  bool loaded;
  Boombox *parent;
  struct {
    FMOD_SOUND *sound;
    FMOD_CHANNEL *channel;
  } fmod;
};
