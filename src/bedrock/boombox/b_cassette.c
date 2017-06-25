#include "b_internal.h"

BoomboxCassette *boombox_cassette_create(Boombox *const boombox) {
  BoomboxCassette *cassette = calloc(1, sizeof(BoomboxCassette));

  cassette->parent = boombox;

  return cassette;
}

void boombox_cassette_destroy(BoomboxCassette *const cassette) {
  assert(cassette);

  if (cassette->loaded) {
    FMOD_Sound_Release(cassette->fmod.sound);
  }

  free(cassette);
}

BoomboxResult boombox_cassette_load_sound(BoomboxCassette *const cassette, const char *filepath) {
  assert(cassette);

  FMOD_RESULT result = FMOD_System_CreateStream(cassette->parent->fmod.system, filepath, FMOD_DEFAULT, 0, &cassette->fmod.sound);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_FAIL;
  }

  cassette->loaded = true;
  return BOOMBOX_OK;
}

BoomboxResult boombox_cassette_play(BoomboxCassette *const cassette) {
  assert(cassette);
  if (!cassette->loaded) {
    return BOOMBOX_NO_SOUND_LOADED;
  }

  FMOD_RESULT result = FMOD_System_PlaySound(cassette->parent->fmod.system, cassette->fmod.sound, 0, false, &cassette->fmod.channel);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_FAIL;
  }

  cassette->playing = true;

  return BOOMBOX_OK;
}

BoomboxResult boombox_cassette_stop(BoomboxCassette *const cassette) {
  assert(cassette);
  if (!cassette->loaded) {
    return BOOMBOX_NO_SOUND_LOADED;
  }
  if (!cassette->playing) {
    return BOOMBOX_NOT_PLAYING;
  }

  FMOD_RESULT result = FMOD_Channel_Stop(cassette->fmod.channel);
  if (result != FMOD_OK) {
    printf("FMOD: (%d) %s\n", result, FMOD_ErrorString(result));
    return BOOMBOX_FAIL;
  }

  cassette->playing = false;

  return BOOMBOX_OK;
}

bool boombox_cassette_playing(BoomboxCassette *const cassette) {
  assert(cassette);
  if (!cassette->loaded) {
    return BOOMBOX_NO_SOUND_LOADED;
  }

  return cassette->playing;
}

BoomboxResult boombox_cassette_set_pitch(BoomboxCassette *const cassette, float pitch) {
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

BoomboxResult boombox_cassette_get_spectrum(BoomboxCassette *const cassette, float *left, float *right) {
  assert(cassette);
  if (!cassette->loaded) {
    return BOOMBOX_NO_SOUND_LOADED;
  }
  if (!cassette->playing) {
    return BOOMBOX_NOT_PLAYING;
  }

  FMOD_DSP_PARAMETER_FFT *fft;
  uint32_t len = 0;
  FMOD_Channel_AddDSP(cassette->fmod.channel, 0, cassette->parent->fmod.dsp);
  FMOD_DSP_GetParameterData(cassette->parent->fmod.dsp, FMOD_DSP_FFT_SPECTRUMDATA, &fft, &len, NULL, 0);

  if (left && fft->numchannels == 1) {
    for (uint32_t t = 0; t < fft->length; t++) {
      left[t] = fft->spectrum[0][t];
      right[t] = 0.0f;
    }
  } else if (left && right) {
    for (uint32_t t = 0; t < fft->length; t++) {
      left[t] = fft->spectrum[0][t];
      right[t] = fft->spectrum[1][t];
    }
  }

  return BOOMBOX_OK;
}
