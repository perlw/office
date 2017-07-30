#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "system_sound.h"

typedef struct {
  double timing;
  double since_update;

  uint32_t song;
  float spectrum_left[78];
  float spectrum_right[78];

  Surface *spectrum;
} SceneSoundTest;

/*void scene_sound_test_spectrum(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  SceneSoundTest *scene = (SceneSoundTest *)subscriberdata;
  Spectrum *spectrum = (Spectrum *)userdata;

  scene->song = spectrum->song_id;

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

void scene_sound_test_keyboard(const char *group_id, const char *id, void *const subscriberdata, void *const userdata) {
  SceneSoundTest *scene = (SceneSoundTest *)subscriberdata;
  PicassoWindowKeyboardEvent *event = (PicassoWindowKeyboardEvent *)userdata;

  if (event->pressed) {
    if (event->key == PICASSO_KEY_P) {
      gossip_emit("sound:play_song", sizeof(uint32_t), &scene->song);
    } else if (event->key == PICASSO_KEY_S) {
      gossip_emit("sound:stop_song", 0, NULL);
      for (uint32_t t = 0; t < 78; t++) {
        scene->spectrum_left[t] = 0.0f;
        scene->spectrum_right[t] = 0.0f;
      }
    } else if (event->key == PICASSO_KEY_N) {
      scene->song = (scene->song == 0 ? 1 : 0);
      gossip_emit("sound:stop_song", 0, NULL);
      gossip_emit("sound:play_song", sizeof(uint32_t), &scene->song);
    }
  }
}*/

SceneSoundTest *scene_sound_test_create(void) {
  SceneSoundTest *scene = calloc(1, sizeof(SceneSoundTest));

  const Config *const config = config_get();

  scene->timing = 1.0 / 30.0;
  scene->since_update = 1.0 / (double)((rand() % 29) + 1);

  // +Spectrum UI
  scene->spectrum = surface_create(0, 0, config->ascii_width, config->ascii_height);
  SurfaceRectTiles rect_tiles = {
    '+', '-', '+',
    '|', 0, '|',
    '+', '-', '+',
  };
  surface_rect(scene->spectrum, 0, 0, config->ascii_width, 30, rect_tiles, true, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });
  surface_text(scene->spectrum, 2, 29, 21, " Play | Stop | Next ", (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });
  uint32_t base = (29 * config->ascii_width) + 3;
  scene->spectrum->buffer[base].fore = (GlyphColor){ 255, 255, 255 };
  scene->spectrum->buffer[base + 7].fore = (GlyphColor){ 255, 255, 255 };
  scene->spectrum->buffer[base + 14].fore = (GlyphColor){ 255, 255, 255 };
  // -Spectrum UI

  return scene;
}

void scene_sound_test_destroy(SceneSoundTest *const scene) {
  assert(scene);

  surface_destroy(scene->spectrum);

  free(scene);
}

void scene_sound_test_update(SceneSoundTest *const scene, double delta) {
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
            .r = (uint8_t)(196.0f * gradient),
            .g = (uint8_t)(164.0f - (128.0f * gradient)),
            .b = 0,
          };

          scene->spectrum->buffer[index_l].rune = (y == height_l - 1 ? 178 : 177);
          scene->spectrum->buffer[index_l].fore = flame;
        } else {
          scene->spectrum->buffer[index_l].fore = glyphcolor_muls(scene->spectrum->buffer[index_l].fore, 0.9f);
        }
        if (y < height_r) {
          float gradient = (float)y / (float)height_r;
          GlyphColor flame = {
            .r = (uint8_t)(196.0f * gradient),
            .g = (uint8_t)(164.0f - (128.0f * gradient)),
            .b = 0,
          };

          scene->spectrum->buffer[index_r].rune = (y == height_r - 1 ? 178 : 177);
          scene->spectrum->buffer[index_r].fore = flame;
        } else {
          scene->spectrum->buffer[index_r].fore = glyphcolor_muls(scene->spectrum->buffer[index_r].fore, 0.9f);
        }
      }
    }

    if (scene->song == 0) {
      surface_text(scene->spectrum, 2, 0, 15, " settlers.mod ", (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 0, 0, 0 });
      surface_text(scene->spectrum, 16, 0, 16, "| comicbak.mod ", (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });
    } else {
      surface_text(scene->spectrum, 2, 0, 16, " settlers.mod |", (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });
      surface_text(scene->spectrum, 17, 0, 15, " comicbak.mod ", (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 0, 0, 0 });
    }
  }
}

void scene_sound_test_draw(SceneSoundTest *const scene, AsciiBuffer *const screen) {
  assert(scene);

  surface_draw(scene->spectrum, screen);
}
