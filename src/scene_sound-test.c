#include <assert.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "screen.h"

bool scene_sound_test_start(void);
void scene_sound_test_stop(void);
void scene_sound_test_update(void);
void scene_sound_test_message(uint32_t id, RectifyMap *const map);

KronosSystem scene_sound_test = {
  .name = "scene_sound-test",
  .frames = 30,
  .start = &scene_sound_test_start,
  .stop = &scene_sound_test_stop,
  .update = &scene_sound_test_update,
  .message = &scene_sound_test_message,
};

typedef struct {
  uint32_t song;
  float spectrum_left[78];
  float spectrum_right[78];

  Surface *spectrum;
} SceneSoundTest;

void scene_sound_test_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

SceneSoundTest *scene_sound_test_internal = NULL;
bool scene_sound_test_start(void) {
  if (scene_sound_test_internal) {
    return false;
  }

  scene_sound_test_internal = calloc(1, sizeof(SceneSoundTest));
  scene_sound_test_internal->song = 0;
  memset(scene_sound_test_internal->spectrum_left, 0, sizeof(float) * 78);
  memset(scene_sound_test_internal->spectrum_right, 0, sizeof(float) * 78);

  const Config *const config = config_get();

  // +Spectrum UI
  scene_sound_test_internal->spectrum = surface_create(0, 0, config->ascii_width, config->ascii_height);
  SurfaceRectTiles rect_tiles = {
    '+', '-', '+',
    '|', 0, '|',
    '+', '-', '+',
  };
  surface_rect(scene_sound_test_internal->spectrum, 0, 0, config->ascii_width, 30, rect_tiles, true, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });
  surface_textc(scene_sound_test_internal->spectrum, 2, 29, 75, " #{ffffff}P#{c8c8c8}lay | #{ffffff}S#{c8c8c8}top | #{ffffff}N#{c8c8c8}ext ");
  // -Spectrum UI

  screen_hook_render(&scene_sound_test_internal_render_hook, NULL, 0);

  return true;
}

void scene_sound_test_stop(void) {
  if (!scene_sound_test_internal) {
    return;
  }

  screen_unhook_render(&scene_sound_test_internal_render_hook, NULL);
  surface_destroy(scene_sound_test_internal->spectrum);

  free(scene_sound_test_internal);
  scene_sound_test_internal = NULL;
}

void scene_sound_test_update(void) {
  if (!scene_sound_test_internal) {
    return;
  }

  for (uint32_t t = 0; t < 78; t++) {
    uint32_t height_l = (uint32_t)(scene_sound_test_internal->spectrum_left[t] * 14.0f);
    uint32_t height_r = (uint32_t)(scene_sound_test_internal->spectrum_right[t] * 14.0f);

    if (height_l > 14) {
      height_l = 14;
    }
    if (height_r > 14) {
      height_r = 14;
    }

    for (uint32_t y = 0; y < 14; y++) {
      uint32_t index_l = ((14 - y) * scene_sound_test_internal->spectrum->width) + (t + 1);
      uint32_t index_r = ((15 + y) * scene_sound_test_internal->spectrum->width) + (t + 1);

      if (y < height_l) {
        float gradient = (float)y / (float)height_l;
        GlyphColor flame = {
          .r = (uint8_t)(196.0f * gradient),
          .g = (uint8_t)(164.0f - (128.0f * gradient)),
          .b = 0,
        };

        scene_sound_test_internal->spectrum->buffer[index_l].rune = (y == height_l - 1 ? 178 : 177);
        scene_sound_test_internal->spectrum->buffer[index_l].fore = flame;
      } else {
        scene_sound_test_internal->spectrum->buffer[index_l].fore = glyphcolor_muls(scene_sound_test_internal->spectrum->buffer[index_l].fore, 0.9f);
      }
      if (y < height_r) {
        float gradient = (float)y / (float)height_r;
        GlyphColor flame = {
          .r = (uint8_t)(196.0f * gradient),
          .g = (uint8_t)(164.0f - (128.0f * gradient)),
          .b = 0,
        };

        scene_sound_test_internal->spectrum->buffer[index_r].rune = (y == height_r - 1 ? 178 : 177);
        scene_sound_test_internal->spectrum->buffer[index_r].fore = flame;
      } else {
        scene_sound_test_internal->spectrum->buffer[index_r].fore = glyphcolor_muls(scene_sound_test_internal->spectrum->buffer[index_r].fore, 0.9f);
      }
    }
  }

  if (scene_sound_test_internal->song == 0) {
    surface_text(scene_sound_test_internal->spectrum, 2, 0, 15, " settlers.mod ", (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 0, 0, 0 });
    surface_text(scene_sound_test_internal->spectrum, 16, 0, 16, "| comicbak.mod ", (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });
  } else {
    surface_text(scene_sound_test_internal->spectrum, 2, 0, 16, " settlers.mod |", (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });
    surface_text(scene_sound_test_internal->spectrum, 17, 0, 15, " comicbak.mod ", (GlyphColor){ 255, 255, 255 }, (GlyphColor){ 0, 0, 0 });
  }
}

void scene_sound_test_message(uint32_t id, RectifyMap *const map) {
  if (!scene_sound_test_internal) {
    return;
  }

  switch (id) {
    case MSG_INPUT_KEY: {
      PicassoKey key = *(uint32_t * const)rectify_map_get(map, "key");
      bool pressed = *(bool *const)rectify_map_get(map, "pressed");

      if (pressed) {
        switch (key) {
          case PICASSO_KEY_P: {
            RectifyMap *map = rectify_map_create();
            rectify_map_set(map, "song", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), &scene_sound_test_internal->song);
            gossip_emit(MSG_SOUND_PLAY_SONG, map);
            break;
          }

          case PICASSO_KEY_S: {
            gossip_emit(MSG_SOUND_STOP_SONG, NULL);
            for (uint32_t t = 0; t < 78; t++) {
              scene_sound_test_internal->spectrum_left[t] = 0.0f;
              scene_sound_test_internal->spectrum_right[t] = 0.0f;
            }
            break;
          }

          case PICASSO_KEY_N: {
            gossip_emit(MSG_SOUND_STOP_SONG, NULL);

            scene_sound_test_internal->song = (scene_sound_test_internal->song == 0 ? 1 : 0);
            RectifyMap *map = rectify_map_create();
            rectify_map_set(map, "song", sizeof(uint32_t), RECTIFY_MAP_TYPE_UINT, &scene_sound_test_internal->song);
            gossip_emit(MSG_SOUND_PLAY_SONG, map);
            break;
          }
        }
      }
      break;
    }

    case MSG_SOUND_SPECTRUM: {
      uint32_t *const song = rectify_map_get(map, "song");
      float *const left = rectify_map_get(map, "left");
      float *const right = rectify_map_get(map, "right");
      if (!left || !right) {
        return;
      }

      scene_sound_test_internal->song = *song;

      uint32_t step = 1;
      for (uint32_t t = 0, u = 0; u < 78; t += step, u++) {
        float l = 0.0f;
        float r = 0.0f;
        for (uint32_t s = t; s < t + step; s++) {
          l += left[s];
          r += right[s];
        }
        l = l / (float)step;
        r = r / (float)step;
        scene_sound_test_internal->spectrum_left[u] = l * 10.0f;
        scene_sound_test_internal->spectrum_right[u] = r * 10.0f;
      }
      break;
    }
  }
}

void scene_sound_test_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  if (!scene_sound_test_internal) {
    return;
  }
  surface_draw(scene_sound_test_internal->spectrum, screen);
}
