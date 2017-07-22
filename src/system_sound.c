#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "system_sound.h"

bool system_sound_start(void);
void system_sound_stop(void);
void system_sound_update(void);

KronosSystem system_sound = {
  .name = "sound",
  .frames = 30,
  .start = &system_sound_start,
  .stop = &system_sound_stop,
  .update = &system_sound_update,
};

typedef struct {
  Boombox *boombox;
  BoomboxCassette *init_sound;
  BoomboxCassette *tap_sound;
  BoomboxCassette *boom_sound;
  BoomboxCassette *drip_sound;
  BoomboxCassette *water_footsteps_sound;

  BoomboxCassette *song;
  BoomboxCassette *song2;

  GossipHandle sound_handle;
} SystemSound;

void system_sound_internal_sound_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata);

SystemSound *system_sound_internal = NULL;
bool system_sound_start(void) {
  if (system_sound_internal) {
    return false;
  }

  system_sound_internal = calloc(1, sizeof(SystemSound));

  system_sound_internal->boombox = boombox_create();
  if (boombox_init(system_sound_internal->boombox) != BOOMBOX_OK) {
    printf("Boombox: failed to init\n");
    return false;
  }

  system_sound_internal->init_sound = boombox_cassette_create(system_sound_internal->boombox);
  system_sound_internal->tap_sound = boombox_cassette_create(system_sound_internal->boombox);
  system_sound_internal->boom_sound = boombox_cassette_create(system_sound_internal->boombox);
  system_sound_internal->drip_sound = boombox_cassette_create(system_sound_internal->boombox);
  system_sound_internal->song = boombox_cassette_create(system_sound_internal->boombox);
  system_sound_internal->song2 = boombox_cassette_create(system_sound_internal->boombox);

  bool abort = false;
  if (boombox_cassette_load_sound(system_sound_internal->init_sound, "swish.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load init sound\n");
    abort = true;
  }
  if (boombox_cassette_load_sound(system_sound_internal->tap_sound, "tap.ogg") != BOOMBOX_OK) {
    printf("Boombox: failed to load tap sound\n");
    abort = true;
  }
  if (boombox_cassette_load_sound(system_sound_internal->boom_sound, "boom.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load boom sound\n");
    abort = true;
  }
  if (boombox_cassette_load_sound(system_sound_internal->drip_sound, "drip.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load drip sound\n");
    abort = true;
  }
  /*if (boombox_cassette_load_sound(system_sound_internal->water_footsteps_sound, "water_footsteps.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load water footsteps sound\n");
    abort = true;
  }*/
  if (boombox_cassette_load_sound(system_sound_internal->song, "music/settlers.mod") != BOOMBOX_OK) {
    printf("Boombox: failed to load song\n");
    abort = true;
  }
  if (boombox_cassette_load_sound(system_sound_internal->song2, "music/comicbak.mod") != BOOMBOX_OK) {
    printf("Boombox: failed to load song\n");
    abort = true;
  }

  if (abort) {
    boombox_destroy(system_sound_internal->boombox);
    return false;
  }

  //gossip_subscribe(MSG_GAME, MSG_GAME_INIT, &system_sound_event, system_sound_internal);
  system_sound_internal->sound_handle = gossip_subscribe("sound:*", &system_sound_internal_sound_event, system_sound_internal);

  return true;
}

void system_sound_stop(void) {
  if (!system_sound_internal) {
    return;
  }

  gossip_unsubscribe(system_sound_internal->sound_handle);

  boombox_cassette_destroy(system_sound_internal->song2);
  boombox_cassette_destroy(system_sound_internal->song);
  //boombox_cassette_destroy(system_sound_internal->water_footsteps_sound);
  boombox_cassette_destroy(system_sound_internal->drip_sound);
  boombox_cassette_destroy(system_sound_internal->boom_sound);
  boombox_cassette_destroy(system_sound_internal->tap_sound);
  boombox_cassette_destroy(system_sound_internal->init_sound);
  boombox_destroy(system_sound_internal->boombox);

  free(system_sound_internal);
}

void system_sound_update(void) {
  assert(system_sound_internal);

  boombox_update(system_sound_internal->boombox);

  // Temp
  if (boombox_cassette_playing(system_sound_internal->song) || boombox_cassette_playing(system_sound_internal->song2)) {
    Spectrum spectrum;
    if (boombox_cassette_playing(system_sound_internal->song)) {
      spectrum.song_id = 0;
      boombox_cassette_get_spectrum(system_sound_internal->song, spectrum.left, spectrum.right);
    } else if (boombox_cassette_playing(system_sound_internal->song2)) {
      spectrum.song_id = 1;
      boombox_cassette_get_spectrum(system_sound_internal->song2, spectrum.left, spectrum.right);
    }
    gossip_emit("sound:spectrum", sizeof(Spectrum), &spectrum);
  }
}

void system_sound_internal_sound_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  //SystemSound *system_sound = (SystemSound *)subscriberdata;

  if (strncmp(id, "play_tap", 128) == 0) {
    boombox_cassette_play(system_sound_internal->tap_sound);
    boombox_cassette_set_pitch(system_sound_internal->tap_sound, 0.9f + ((float)(rand() % 20) / 100.0f));
  } else if (strncmp(id, "play_boom", 128) == 0) {
    boombox_cassette_play(system_sound_internal->boom_sound);
  } else if (strncmp(id, "play_drip", 128) == 0) {
    boombox_cassette_play(system_sound_internal->drip_sound);
    boombox_cassette_set_pitch(system_sound_internal->drip_sound, 0.8f + ((float)(rand() % 40) / 100.0f));
  } else if (strncmp(id, "play_water_footsteps", 128) == 0) {
    boombox_cassette_play(system_sound_internal->water_footsteps_sound);
    boombox_cassette_set_pitch(system_sound_internal->drip_sound, 0.8f + ((float)(rand() % 40) / 100.0f));
  } else if (strncmp(id, "play_song", 128) == 0) {
    uint32_t song = *(uint32_t *)userdata;
    if (song == 0) {
      boombox_cassette_play(system_sound_internal->song);
    } else {
      boombox_cassette_play(system_sound_internal->song2);
    }
  } else if (strncmp(id, "stop_song", 128) == 0) {
    boombox_cassette_stop(system_sound_internal->song);
    boombox_cassette_stop(system_sound_internal->song2);
  }
}
