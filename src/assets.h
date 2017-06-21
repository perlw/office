#pragma once

typedef enum {
  ASSET_SHADER = 1,
  ASSET_TEXTURE,
  ASSET_TILESET,
} AssetTypes;

void setup_asset_loaders(void);
