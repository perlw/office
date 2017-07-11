#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ascii.h"
#include "assets.h"

TileSet *tileset_create(void) {
  TileSet *tileset = calloc(1, sizeof(TileSet));

  memset(tileset->tile_defs, 0, sizeof(TileDef) * 256);

  return tileset;
}

void tileset_destroy(TileSet *const tileset) {
  assert(tileset);

  tome_release(ASSET_TEXTURE, "tileset");

  free(tileset);
}

/*void tileset_lua_func(Muse *const muse, uintmax_t num_arguments, const MuseArgument *const arguments, const void *const userdata) {
  TileSet *tileset = (TileSet *)userdata;
  char buffer[256];

  snprintf(buffer, 256, "tilesets/%s", (char *)arguments[0].argument);
  tileset->texture = (PicassoTexture *)tome_fetch(ASSET_TEXTURE, "tileset", buffer);
  if (!tileset->texture) {
    printf("Something went wrong when fetching tileset texture :(\n");
  }
}

void tileprops_lua_func(Muse *const muse, uintmax_t num_arguments, const MuseArgument *const arguments, const void *const userdata) {
  TileSet *tileset = (TileSet *)userdata;

  uint8_t tile_id = (uint8_t) * (double *)arguments[0].argument;
  MuseTableEntry *table = (MuseTableEntry *)arguments[1].argument;
  uint32_t curr = 0;
  while (table[curr].type != MUSE_TYPE_UNK) {
    if (strncmp(table[curr].key, "auto_tile", 10) == 0) {
      tileset->tile_defs[tile_id].auto_tile = *(bool *)table[curr].val;
    }

    printf("tile %d: auto_tile = %d\n", tile_id, tileset->tile_defs[tile_id].auto_tile);

    curr++;
  }
}*/

void tileset_load_defs(TileSet *const tileset, const char *filepath) {
  assert(tileset);

  /*MuseFunctionDef tileset_func_def = {
    .name = "tileset",
    .func = &tileset_lua_func,
    .num_arguments = 1,
    .arguments = (MuseType[]){
      MUSE_TYPE_STRING,
    },
    .userdata = tileset,
  };
  MuseFunctionDef tileprops_func_def = {
    .name = "tile_props",
    .func = &tileprops_lua_func,
    .num_arguments = 2,
    .arguments = (MuseType[]){
      MUSE_TYPE_NUMBER,
      MUSE_TYPE_TABLE,
    },
    .userdata = tileset,
  };

  Muse *muse = muse_create_lite();

  muse_add_func(muse, &tileset_func_def);
  muse_add_func(muse, &tileprops_func_def);

  muse_load_file(muse, filepath);
  muse_destroy(muse);*/
}
