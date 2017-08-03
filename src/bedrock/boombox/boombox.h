/**
 * Boombox - loading and playing sounds
 */

#pragma once

typedef enum {
  BOOMBOX_OK = 1,
  BOOMBOX_FAIL,
  BOOMBOX_NO_SOUND_LOADED,
  BOOMBOX_NOT_PLAYING,
} BoomboxResult;

// +Boombox
BoomboxResult boombox_init(void);
void boombox_kill(void);

void boombox_update(void);
// -Boombox

// +Cassette
typedef struct BoomboxCassette BoomboxCassette;

BoomboxCassette *boombox_cassette_create(void);
void boombox_cassette_destroy(BoomboxCassette **cassette);

BoomboxResult boombox_cassette_load_sound(BoomboxCassette *const cassette, const char *filepath);
BoomboxResult boombox_cassette_play(BoomboxCassette *const cassette);
BoomboxResult boombox_cassette_stop(BoomboxCassette *const cassette);

bool boombox_cassette_playing(BoomboxCassette *const cassette);
BoomboxResult boombox_cassette_set_pitch(BoomboxCassette *const cassette, float pitch);
BoomboxResult boombox_cassette_get_spectrum(BoomboxCassette *const cassette, float *left, float *right);
// -Cassette
