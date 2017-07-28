#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "messages.h"

bool system_sound_start(void);
void system_sound_stop(void);
void system_sound_update(void);
void system_sound_message(uint32_t id, RectifyMap *const map);

KronosSystem system_sound = {
  .name = "sound",
  .frames = 30,
  .start = &system_sound_start,
  .stop = &system_sound_stop,
  .update = &system_sound_update,
  .message = &system_sound_message,
};

typedef struct {
  BoomboxCassette *init_sound;
  BoomboxCassette *tap_sound;
  BoomboxCassette *boom_sound;
  BoomboxCassette *drip_sound;
  BoomboxCassette *water_footsteps_sound;

  BoomboxCassette *song;
  BoomboxCassette *song2;
} SystemSound;

SystemSound *system_sound_internal = NULL;
bool system_sound_start(void) {
  if (system_sound_internal) {
    return false;
  }

  system_sound_internal = calloc(1, sizeof(SystemSound));

  if (boombox_init() != BOOMBOX_OK) {
    printf("Boombox: failed to init\n");
    return false;
  }

  system_sound_internal->init_sound = boombox_cassette_create();
  system_sound_internal->tap_sound = boombox_cassette_create();
  system_sound_internal->boom_sound = boombox_cassette_create();
  system_sound_internal->drip_sound = boombox_cassette_create();
  system_sound_internal->song = boombox_cassette_create();
  system_sound_internal->song2 = boombox_cassette_create();

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
    boombox_kill();
    return false;
  }

  return true;
}

void system_sound_stop(void) {
  if (!system_sound_internal) {
    return;
  }

  boombox_cassette_destroy(system_sound_internal->song2);
  boombox_cassette_destroy(system_sound_internal->song);
  //boombox_cassette_destroy(system_sound_internal->water_footsteps_sound);
  boombox_cassette_destroy(system_sound_internal->drip_sound);
  boombox_cassette_destroy(system_sound_internal->boom_sound);
  boombox_cassette_destroy(system_sound_internal->tap_sound);
  boombox_cassette_destroy(system_sound_internal->init_sound);
  boombox_kill();

  free(system_sound_internal);
  system_sound_internal = NULL;
}

void system_sound_update(void) {
  assert(system_sound_internal);

  boombox_update();

  // Temp
  if (boombox_cassette_playing(system_sound_internal->song) || boombox_cassette_playing(system_sound_internal->song2)) {
    uint32_t song_id = 0;
    float left[2048];
    float right[2048];
    if (boombox_cassette_playing(system_sound_internal->song)) {
      song_id = 0;
      boombox_cassette_get_spectrum(system_sound_internal->song, left, right);
    } else if (boombox_cassette_playing(system_sound_internal->song2)) {
      song_id = 1;
      boombox_cassette_get_spectrum(system_sound_internal->song2, left, right);
    }

    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "song_id", sizeof(uint32_t), &song_id);
    rectify_map_set(map, "left", sizeof(float) * 2048, left);
    rectify_map_set(map, "right", sizeof(float) * 2048, right);
    gossip_emit(MSG_SOUND_SPECTRUM, map);
  }
}

void system_sound_message(uint32_t id, RectifyMap *const map) {
  if (!system_sound_internal) {
    return;
  }

  switch (id) {
    case MSG_SOUND_PLAY:
      boombox_cassette_play(system_sound_internal->drip_sound);
      boombox_cassette_set_pitch(system_sound_internal->drip_sound, 0.8f + ((float)(rand() % 40) / 100.0f));
      break;
  }
}
/*
void system_sound_internal_sound_event(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
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
*/
