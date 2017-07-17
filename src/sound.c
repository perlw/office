#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"
#include "sound.h"

struct SoundSys {
  double timing;
  double since_update;

  Boombox *boombox;
  BoomboxCassette *init_sound;
  BoomboxCassette *tap_sound;
  BoomboxCassette *boom_sound;

  BoomboxCassette *song;
  BoomboxCassette *song2;

  GossipHandle sound_handle;
};

void soundsys_internal_sound_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);

SoundSys *soundsys_create(void) {
  SoundSys *soundsys = calloc(1, sizeof(SoundSys));

  soundsys->timing = 1.0 / 30.0;
  soundsys->since_update = 1.0 / (double)((rand() % 29) + 1);

  soundsys->boombox = boombox_create();
  if (boombox_init(soundsys->boombox) != BOOMBOX_OK) {
    printf("Boombox: failed to init\n");
    return NULL;
  }

  soundsys->init_sound = boombox_cassette_create(soundsys->boombox);
  soundsys->tap_sound = boombox_cassette_create(soundsys->boombox);
  soundsys->boom_sound = boombox_cassette_create(soundsys->boombox);
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
  if (boombox_cassette_load_sound(soundsys->boom_sound, "boom.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load boom sound\n");
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

  //gossip_subscribe(MSG_GAME, MSG_GAME_INIT, &soundsys_event, soundsys);
  soundsys->sound_handle = gossip_subscribe("sound:*", &soundsys_internal_sound_event, soundsys);

  return soundsys;
}

void soundsys_destroy(SoundSys *soundsys) {
  assert(soundsys);

  gossip_unsubscribe(soundsys->sound_handle);

  boombox_cassette_destroy(soundsys->song2);
  boombox_cassette_destroy(soundsys->song);
  boombox_cassette_destroy(soundsys->boom_sound);
  boombox_cassette_destroy(soundsys->tap_sound);
  boombox_cassette_destroy(soundsys->init_sound);
  boombox_destroy(soundsys->boombox);

  free(soundsys);
}

void soundsys_update(SoundSys *soundsys, double delta) {
  assert(soundsys);

  boombox_update(soundsys->boombox);

  soundsys->since_update += delta;
  while (soundsys->since_update >= soundsys->timing) {
    soundsys->since_update -= soundsys->timing;

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
      gossip_emit("sound:spectrum", sizeof(Spectrum), &spectrum);
    }
  }
}

void soundsys_internal_sound_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  SoundSys *soundsys = (SoundSys *)subscriberdata;

  if (strncmp(id, "play_tap", 128) == 0) {
    boombox_cassette_play(soundsys->tap_sound);
    boombox_cassette_set_pitch(soundsys->tap_sound, 0.9f + ((float)(rand() % 20) / 100.0f));
  } else if (strncmp(id, "play_boom", 128) == 0) {
    boombox_cassette_play(soundsys->boom_sound);
  } else if (strncmp(id, "play_song", 128) == 0) {
    uint32_t song = *(uint32_t *)userdata;
    if (song == 0) {
      boombox_cassette_play(soundsys->song);
    } else {
      boombox_cassette_play(soundsys->song2);
    }
  } else if (strncmp(id, "stop_song", 128) == 0) {
    boombox_cassette_stop(soundsys->song);
    boombox_cassette_stop(soundsys->song2);
  }
}
