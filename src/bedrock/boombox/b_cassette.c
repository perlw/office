#include "b_internal.h"

BoomboxCassette *boombox_cassette_create(Boombox *boombox) {
  BoomboxCassette *cassette = calloc(1, sizeof(BoomboxCassette));

  cassette->parent = boombox;

  return cassette;
}

void boombox_cassette_destroy(BoomboxCassette *cassette) {
  assert(cassette);

  if (cassette->loaded) {
    FMOD_Sound_Release(cassette->fmod.sound);
  }

  free(cassette);
}

BoomboxResult boombox_cassette_load_sound(BoomboxCassette *cassette, const char *filepath) {
  assert(cassette);

  FMOD_RESULT result = FMOD_System_CreateSound(cassette->parent->fmod.system, filepath, FMOD_DEFAULT, 0, &cassette->fmod.sound);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_FAIL;
  }

  cassette->loaded = true;
  return BOOMBOX_OK;
}

BoomboxResult boombox_cassette_play(BoomboxCassette *cassette) {
  assert(cassette);
  if (!cassette->loaded) {
    return BOOMBOX_NO_SOUND_LOADED;
  }

  FMOD_RESULT result = FMOD_System_PlaySound(cassette->parent->fmod.system, cassette->fmod.sound, 0, false, &cassette->fmod.channel);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_FAIL;
  }

  return BOOMBOX_OK;
}

BoomboxResult boombox_cassette_set_pitch(BoomboxCassette *cassette, float pitch) {
  assert(cassette);
  if (!cassette->loaded) {
    return BOOMBOX_NO_SOUND_LOADED;
  }

  FMOD_RESULT result = FMOD_Channel_SetPitch(cassette->fmod.channel, pitch);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_FAIL;
  }

  return BOOMBOX_OK;
}
