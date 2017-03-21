#pragma once

typedef enum {
  BOOMBOX_OK = 1,
  BOOMBOX_INIT_FAILED,
} BoomboxResult;

typedef struct Boombox Boombox;

Boombox *boombox_create(void);
BoomboxResult boombox_init(Boombox *boombox);
void boombox_destroy(Boombox *boombox);
void boombox_update(Boombox *boombox);
