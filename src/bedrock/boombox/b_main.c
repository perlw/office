#include "b_internal.h"

Boombox *boombox_create(void) {
  return calloc(1, sizeof(Boombox));
}

BoomboxResult boombox_init(Boombox *const boombox) {
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

  boombox->init = true;
  return BOOMBOX_OK;
}

void boombox_destroy(Boombox *const boombox) {
  assert(boombox);

  if (boombox->init) {
    FMOD_System_Close(boombox->fmod.system);
    FMOD_System_Release(boombox->fmod.system);
  }

  free(boombox);
}

void boombox_update(Boombox *const boombox) {
  assert(boombox);
  assert(boombox->init);

  FMOD_System_Update(boombox->fmod.system);
}
