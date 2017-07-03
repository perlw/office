#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "bedrock/bedrock.h"
#include "messages.h"
#include "sound.h"

struct SoundSys {
  Boombox *boombox;
  BoomboxCassette *init_sound;
  BoomboxCassette *tap_sound;

  BoomboxCassette *song;
  BoomboxCassette *song2;
};

void soundsys_event(uint32_t id, void *const subscriberdata, void *const userdata);

SoundSys *soundsys_create(void) {
  SoundSys *soundsys = calloc(1, sizeof(SoundSys));

  soundsys->boombox = boombox_create();
  if (boombox_init(soundsys->boombox) != BOOMBOX_OK) {
    printf("Boombox: failed to init\n");
    return NULL;
  }

  soundsys->init_sound = boombox_cassette_create(soundsys->boombox);
  soundsys->tap_sound = boombox_cassette_create(soundsys->boombox);
  soundsys->song = boombox_cassette_create(soundsys->boombox);
  soundsys->song2 = boombox_cassette_create(soundsys->boombox);

  if (boombox_cassette_load_sound(soundsys->init_sound, "swish.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load init sound\n");
    boombox_destroy(soundsys->boombox);
    return NULL;
  }
  if (boombox_cassette_load_sound(soundsys->tap_sound, "tap.ogg") != BOOMBOX_OK) {
    printf("Boombox: failed to load tap sound\n");
    boombox_destroy(soundsys->boombox);
    return NULL;
  }
  if (boombox_cassette_load_sound(soundsys->song, "music/settlers.mod") != BOOMBOX_OK) {
    printf("Boombox: failed to load song\n");
    boombox_destroy(soundsys->boombox);
    return NULL;
  }
  if (boombox_cassette_load_sound(soundsys->song2, "music/comicbak.mod") != BOOMBOX_OK) {
    printf("Boombox: failed to load song\n");
    boombox_destroy(soundsys->boombox);
    return NULL;
  }

  gossip_subscribe(MSG_GAME, MSG_GAME_INIT, &soundsys_event, soundsys);
  gossip_subscribe(MSG_SOUND, MSG_SOUND_PLAY_TAP, &soundsys_event, soundsys);
  gossip_subscribe(MSG_SOUND, MSG_SOUND_PLAY_SONG, &soundsys_event, soundsys);
  gossip_subscribe(MSG_SOUND, MSG_SOUND_STOP_SONG, &soundsys_event, soundsys);

  return soundsys;
}

void soundsys_destroy(SoundSys *soundsys) {
  assert(soundsys);

  boombox_cassette_destroy(soundsys->song2);
  boombox_cassette_destroy(soundsys->song);
  boombox_cassette_destroy(soundsys->tap_sound);
  boombox_cassette_destroy(soundsys->init_sound);
  boombox_destroy(soundsys->boombox);

  free(soundsys);
}

void soundsys_update(SoundSys *soundsys, double delta) {
  assert(soundsys);

  boombox_update(soundsys->boombox);

  // Temp
  if (boombox_cassette_playing(soundsys->song) || boombox_cassette_playing(soundsys->song2)) {
    Spectrum spectrum;
    if (boombox_cassette_playing(soundsys->song)) {
      spectrum.song_id = 0;
      boombox_cassette_get_spectrum(soundsys->song, spectrum.left, spectrum.right);
    } else if (boombox_cassette_playing(soundsys->song2)) {
      spectrum.song_id = 1;
      boombox_cassette_get_spectrum(soundsys->song2, spectrum.left, spectrum.right);
    }
    gossip_emit(MSG_SOUND, MSG_SOUND_SPECTRUM, &spectrum);
  }
}

void soundsys_event(uint32_t id, void *const subscriberdata, void *const userdata) {
  SoundSys *soundsys = (SoundSys *)subscriberdata;

  switch (id) {
    case MSG_GAME_INIT:
      boombox_cassette_play(soundsys->init_sound);
      break;

    case MSG_SOUND_PLAY_TAP:
      boombox_cassette_play(soundsys->tap_sound);
      boombox_cassette_set_pitch(soundsys->tap_sound, 0.9f + ((float)(rand() % 20) / 100.0f));
      break;

    case MSG_SOUND_PLAY_SONG: {
      uint32_t song = *(uint32_t *)userdata;
      if (song == 0) {
        boombox_cassette_play(soundsys->song);
      } else {
        boombox_cassette_play(soundsys->song2);
      }
      break;
    }

    case MSG_SOUND_STOP_SONG:
      boombox_cassette_stop(soundsys->song);
      boombox_cassette_stop(soundsys->song2);
      break;

    default:
      break;
  }
}
