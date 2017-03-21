#pragma once

typedef enum {
  BOOMBOX_OK = 1,
  BOOMBOX_FAIL,
  BOOMBOX_NO_SOUND_LOADED,
} BoomboxResult;

// +Boombox
typedef struct Boombox Boombox;

Boombox *boombox_create(void);
BoomboxResult boombox_init(Boombox *boombox);
void boombox_destroy(Boombox *boombox);
void boombox_update(Boombox *boombox);
// -Boombox

// +Cassette
typedef struct BoomboxCassette BoomboxCassette;

BoomboxCassette *boombox_cassette_create(Boombox *boombox);
void boombox_cassette_destroy(BoomboxCassette *cassette);
BoomboxResult boombox_cassette_load_sound(BoomboxCassette *cassette, const char *filepath);
BoomboxResult boombox_cassette_play(BoomboxCassette *cassette);
// -Cassette
