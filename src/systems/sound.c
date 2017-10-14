#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define USE_BOOMBOX
#define USE_KRONOS
#include "bedrock/bedrock.h"

#define USE_MESSAGES
#include "main.h"

typedef struct {
  BoomboxCassette *init_sound;
  BoomboxCassette *tap_sound;
  BoomboxCassette *boom_sound;
  BoomboxCassette *drip_sound;
  BoomboxCassette *water_footsteps_sound;

  BoomboxCassette *song;
  BoomboxCassette *song2;
} SystemSound;

SystemSound *system_sound_start(void);
void system_sound_stop(void **system);
void system_sound_update(SystemSound *system, double delta);
void system_sound_message(SystemSound *system, uint32_t id, RectifyMap *const map);

KronosSystem system_sound = {
  .name = "sound",
  .frames = 30,
  .autostart = true,
  .start = &system_sound_start,
  .stop = &system_sound_stop,
  .update = &system_sound_update,
  .message = &system_sound_message,
};

SystemSound *system_sound_start(void) {
  SystemSound *system = calloc(1, sizeof(SystemSound));

  if (boombox_init() != BOOMBOX_OK) {
    printf("Boombox: failed to init\n");
    return false;
  }

  system->init_sound = boombox_cassette_create();
  system->tap_sound = boombox_cassette_create();
  system->boom_sound = boombox_cassette_create();
  system->drip_sound = boombox_cassette_create();
  system->song = boombox_cassette_create();
  system->song2 = boombox_cassette_create();

  bool abort = false;
  if (boombox_cassette_load_sound(system->init_sound, "swish.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load init sound\n");
    abort = true;
  }
  if (boombox_cassette_load_sound(system->tap_sound, "tap.ogg") != BOOMBOX_OK) {
    printf("Boombox: failed to load tap sound\n");
    abort = true;
  }
  if (boombox_cassette_load_sound(system->boom_sound, "boom.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load boom sound\n");
    abort = true;
  }
  if (boombox_cassette_load_sound(system->drip_sound, "drip.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load drip sound\n");
    abort = true;
  }
  /*if (boombox_cassette_load_sound(system->water_footsteps_sound, "water_footsteps.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load water footsteps sound\n");
    abort = true;
  }*/
  if (boombox_cassette_load_sound(system->song, "music/settlers.mod") != BOOMBOX_OK) {
    printf("Boombox: failed to load song\n");
    abort = true;
  }
  if (boombox_cassette_load_sound(system->song2, "music/comicbak.mod") != BOOMBOX_OK) {
    printf("Boombox: failed to load song\n");
    abort = true;
  }

  if (abort) {
    boombox_kill();
    return NULL;
  }

  return system;
}

void system_sound_stop(void **system) {
  SystemSound *ptr = *system;
  assert(ptr && system);

  boombox_cassette_destroy(&ptr->song2);
  boombox_cassette_destroy(&ptr->song);
  //boombox_cassette_destroy(&ptr->water_footsteps_sound);
  boombox_cassette_destroy(&ptr->drip_sound);
  boombox_cassette_destroy(&ptr->boom_sound);
  boombox_cassette_destroy(&ptr->tap_sound);
  boombox_cassette_destroy(&ptr->init_sound);
  boombox_kill();

  free(ptr);
  *system = NULL;
}

void system_sound_update(SystemSound *system, double delta) {
  assert(system);

  boombox_update();

  // Temp
  if (boombox_cassette_playing(system->song) || boombox_cassette_playing(system->song2)) {
    uint32_t song_id = 0;
    float left[2048];
    float right[2048];
    if (boombox_cassette_playing(system->song)) {
      song_id = 0;
      boombox_cassette_get_spectrum(system->song, left, right);
    } else if (boombox_cassette_playing(system->song2)) {
      song_id = 1;
      boombox_cassette_get_spectrum(system->song2, left, right);
    }

    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "song", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), &song_id);
    rectify_map_set(map, "left", RECTIFY_MAP_TYPE_FLOAT, sizeof(float) * 2048, left);
    rectify_map_set(map, "right", RECTIFY_MAP_TYPE_FLOAT, sizeof(float) * 2048, right);
    kronos_emit(MSG_SOUND_SPECTRUM, map);
  }
}

void system_sound_message(SystemSound *system, uint32_t id, RectifyMap *const map) {
  assert(system);

  switch (id) {
    case MSG_SOUND_PLAY: {
      char *const sound = (char *const)rectify_map_get(map, "sound");
      if (!sound) {
        break;
      }

      if (strncmp(sound, "tap", 128) == 0) {
        boombox_cassette_play(system->tap_sound);
        boombox_cassette_set_pitch(system->tap_sound, 0.9f + ((float)(rand() % 20) / 100.0f));
      } else if (strncmp(sound, "boom", 128) == 0) {
        boombox_cassette_play(system->boom_sound);
      } else if (strncmp(sound, "drip", 128) == 0) {
        boombox_cassette_play(system->drip_sound);
        boombox_cassette_set_pitch(system->drip_sound, 0.8f + ((float)(rand() % 40) / 100.0f));
      }
      break;
    }

    case MSG_SOUND_PLAY_SONG: {
      uint32_t *const song = (uint32_t * const)rectify_map_get(map, "song");
      if (!song) {
        break;
      }

      if (*song == 0) {
        boombox_cassette_play(system->song);
      } else {
        boombox_cassette_play(system->song2);
      }
      break;
    }

    case MSG_SOUND_STOP_SONG: {
      boombox_cassette_stop(system->song);
      boombox_cassette_stop(system->song2);
      break;
    }
  }
}
