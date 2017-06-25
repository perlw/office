#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "config.h"
#include "messages.h"
#include "sound.h"
#include "tiles/tiles.h"

typedef struct {
  double timing;
  double since_update;

  float spectrum_left[78];
  float spectrum_right[78];
  GossipHandle spectrum_handle;

  TilesAscii *screen;
  Surface *spectrum;
} SceneSoundTest;

void scene_sound_test_spectrum(int32_t id, void *subscriberdata, void *userdata) {
  SceneSoundTest *scene = (SceneSoundTest *)subscriberdata;
  Spectrum *spectrum = (Spectrum *)userdata;

  uint32_t step = 1;
  for (uint32_t t = 0, u = 0; u < 78; t += step, u++) {
    float l = 0.0f;
    float r = 0.0f;
    for (uint32_t s = t; s < t + step; s++) {
      l += spectrum->left[s];
      r += spectrum->right[s];
    }
    l = l / (float)step;
    r = r / (float)step;
    scene->spectrum_left[u] = l * 10.0f;
    scene->spectrum_right[u] = r * 10.0f;
  }
}

SceneSoundTest *scene_sound_test_create(const Config *config) {
  SceneSoundTest *scene = calloc(1, sizeof(SceneSoundTest));

  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;

  scene->screen = tiles_ascii_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);

  // +Spectrum UI
  scene->spectrum = surface_create(0, 0, config->ascii_width, 30);
  SurfaceRectTiles rect_tiles = {
    '+', '-', '+',
    '|', 0, '|',
    '+', '-', '+',
  };
  surface_rect(scene->spectrum, 0, 0, config->ascii_width, 30, rect_tiles, true, (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 0, 0, 0 });
  surface_text(scene->spectrum, 2, 0, 15, " settlers.mod ", (GlyphColor){ 255, 255, 255, }, (GlyphColor){ 0, 0, 0 });
  // -Spectrum UI

  GlyphColor c = { 200, 96, 0 };
  GlyphColor r = glyphcolor_muls(c, -0.9);
  printf("%d %d %d\n", c.r, c.g, c.b);
  printf("%d %d %d\n", r.r, r.g, r.b);

  scene->spectrum_handle = gossip_subscribe(MSG_SOUND_SPECTRUM, &scene_sound_test_spectrum, scene);
  gossip_emit(MSG_SOUND_PLAY_SONG, NULL);

  return scene;
}

void scene_sound_test_destroy(SceneSoundTest *scene) {
  assert(scene);

  gossip_unsubscribe(MSG_SOUND_SPECTRUM, scene->spectrum_handle);
  gossip_emit(MSG_SOUND_STOP_SONG, NULL);

  surface_destroy(scene->spectrum);
  tiles_ascii_destroy(scene->screen);

  free(scene);
}

void scene_sound_test_update(SceneSoundTest *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    for (uint32_t t = 0; t < 78; t++) {
      uint32_t height_l = (uint32_t)(scene->spectrum_left[t] * 14.0f);
      uint32_t height_r = (uint32_t)(scene->spectrum_right[t] * 14.0f);

      if (height_l > 14) {
        height_l = 14;
      }
      if (height_r > 14) {
        height_r = 14;
      }

      for (uint32_t y = 0; y < 14; y++) {
        uint32_t index_l = ((14 - y) * scene->spectrum->width) + (t + 1);
        uint32_t index_r = ((15 + y) * scene->spectrum->width) + (t + 1);

        if (y < height_l) {
          float gradient = (float)y / (float)height_l;
          GlyphColor flame = {
            .r = (uint8_t)(200.0f * gradient),
            .g = (uint8_t)(128.0f - (128.0f * gradient)),
            .b = 0,
          };

          scene->spectrum->asciimap[index_l].rune = 1;
          scene->spectrum->asciimap[index_l].fore = flame;
          scene->spectrum->asciimap[index_l].back = (GlyphColor){ 0, 0, 0 };
        } else {
          scene->spectrum->asciimap[index_l].rune = 1;
          scene->spectrum->asciimap[index_l].fore = glyphcolor_muls(scene->spectrum->asciimap[index_l].fore, 0.9);
          scene->spectrum->asciimap[index_l].back = (GlyphColor){ 0, 0, 0 };
        }
        if (y < height_r) {
          float gradient = (float)y / (float)height_r;
          GlyphColor flame = {
            .r = (uint8_t)(200.0f * gradient),
            .g = (uint8_t)(128.0f - (128.0f * gradient)),
            .b = 0,
          };

          scene->spectrum->asciimap[index_r].rune = 1;
          scene->spectrum->asciimap[index_r].fore = flame;
          scene->spectrum->asciimap[index_r].back = (GlyphColor){ 0, 0, 0 };
        } else {
          scene->spectrum->asciimap[index_r].rune = 1;
          scene->spectrum->asciimap[index_r].fore = glyphcolor_muls(scene->spectrum->asciimap[index_r].fore, 0.9);
          scene->spectrum->asciimap[index_r].back = (GlyphColor){ 0, 0, 0 };
        }
      }
    }
  }
}

void scene_sound_test_draw(SceneSoundTest *scene) {
  assert(scene);

  surface_draw(scene->spectrum, scene->screen);

  tiles_ascii_draw(scene->screen);
}
