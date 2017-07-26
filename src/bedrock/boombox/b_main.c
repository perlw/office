#include "b_internal.h"

Boombox *boombox = NULL;

BoomboxResult boombox_init(void) {
  assert(!boombox);

  boombox = calloc(1, sizeof(Boombox));

  FMOD_RESULT result;
  result = FMOD_System_Create(&boombox->fmod.system);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_FAIL;
  }

  result = FMOD_System_Init(boombox->fmod.system, 512, FMOD_INIT_NORMAL, 0);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_FAIL;
  }

  result = FMOD_System_CreateDSPByType(boombox->fmod.system, FMOD_DSP_TYPE_FFT, &boombox->fmod.dsp);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_FAIL;
  }

  return BOOMBOX_OK;
}

void boombox_kill(void) {
  assert(boombox);

  FMOD_DSP_Release(boombox->fmod.dsp);
  FMOD_System_Close(boombox->fmod.system);
  FMOD_System_Release(boombox->fmod.system);

  free(boombox);
  boombox = NULL;
}

void boombox_update(void) {
  assert(boombox);

  FMOD_System_Update(boombox->fmod.system);
}
