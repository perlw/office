#pragma once

typedef enum {
  BOOMBOX_OK = 1,
  BOOMBOX_FAIL,
  BOOMBOX_NO_SOUND_LOADED,
  BOOMBOX_NOT_PLAYING,
} BoomboxResult;

// +Boombox
typedef struct Boombox Boombox;

Boombox *boombox_create(void);
BoomboxResult boombox_init(Boombox *const boombox);
void boombox_destroy(Boombox *const boombox);
void boombox_update(Boombox *const boombox);
// -Boombox

// +Cassette
typedef struct BoomboxCassette BoomboxCassette;

BoomboxCassette *boombox_cassette_create(Boombox *const boombox);
void boombox_cassette_destroy(BoomboxCassette *const cassette);
BoomboxResult boombox_cassette_load_sound(BoomboxCassette *const cassette, const char *filepath);
BoomboxResult boombox_cassette_play(BoomboxCassette *const cassette);
BoomboxResult boombox_cassette_stop(BoomboxCassette *const cassette);
BoomboxResult boombox_cassette_set_pitch(BoomboxCassette *const cassette, float pitch);
BoomboxResult boombox_cassette_get_spectrum(BoomboxCassette *const cassette, float *left, float *right);
// -Cassette
