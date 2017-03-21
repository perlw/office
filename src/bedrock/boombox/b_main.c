#include "b_internal.h"

Boombox *boombox_create(void) {
  Boombox *boombox = calloc(1, sizeof(Boombox));

  return boombox;
}

BoomboxResult boombox_init(Boombox *boombox) {
  FMOD_RESULT result;
  result = FMOD_System_Create(&boombox->fmod.system);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_INIT_FAILED;
  }

  result = FMOD_System_Init(boombox->fmod.system, 512, FMOD_INIT_NORMAL, 0);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_INIT_FAILED;
  }

  result = FMOD_System_CreateSound(boombox->fmod.system, "swish.wav", FMOD_DEFAULT, 0, &boombox->fmod.sound);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_INIT_FAILED;
  }

  result = FMOD_System_PlaySound(boombox->fmod.system, boombox->fmod.sound, 0, false, &boombox->fmod.channel);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_INIT_FAILED;
  }

  boombox->init = true;
  return BOOMBOX_OK;
}

void boombox_destroy(Boombox *boombox) {
  assert(boombox);

  if (boombox->init) {
    FMOD_Sound_Release(boombox->fmod.sound);
    FMOD_System_Close(boombox->fmod.system);
    FMOD_System_Release(boombox->fmod.system);
  }

  free(boombox);
}

void boombox_update(Boombox *boombox) {
  assert(boombox);
  assert(boombox->init);

  FMOD_System_Update(boombox->fmod.system);
}
