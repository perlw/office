#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "fmod/fmod.h"
#include "fmod/fmod_errors.h"

FMOD_SYSTEM *p_system = NULL;
FMOD_SOUND *p_sound = NULL;
FMOD_CHANNEL *p_channel = NULL;

void boombox_create(void) {
  FMOD_RESULT result;
  result = FMOD_System_Create(&p_system);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }

  result = FMOD_System_Init(p_system, 512, FMOD_INIT_NORMAL, 0);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }

  result = FMOD_System_CreateSound(p_system, "swish.wav", FMOD_DEFAULT, 0, &p_sound);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }

  result = FMOD_System_PlaySound(p_system, p_sound, 0, false, &p_channel);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    exit(-1);
  }
}

void boombox_update(void) {
  FMOD_System_Update(p_system);
}

void boombox_destroy(void) {
  FMOD_Sound_Release(p_sound);
  FMOD_System_Close(p_system);
  FMOD_System_Release(p_system);
}
