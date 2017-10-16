#include <assert.h>
#include <string.h>

#include "arkanis/math_3d.h"

#define USE_KRONOS
#include "bedrock/bedrock.h"

#define USE_ASCII
#define USE_CONFIG
#define USE_MESSAGES
#define USE_SCREEN
#include "main.h"

typedef struct {
  uint32_t song;
  float spectrum_left[158];
  float spectrum_right[158];

  Surface *spectrum;
} SceneSoundTest;

SceneSoundTest *scene_sound_test_start(void);
void scene_sound_test_stop(void **scene);
void scene_sound_test_update(SceneSoundTest *scene, double delta);
void scene_sound_test_message(SceneSoundTest *scene, uint32_t id, RectifyMap *const map);

KronosSystem scene_sound_test = {
  .name = "scene_sound-test",
  .frames = 30,
  .start = &scene_sound_test_start,
  .stop = &scene_sound_test_stop,
  .update = &scene_sound_test_update,
  .message = &scene_sound_test_message,
};

void scene_sound_test_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

SceneSoundTest *scene_sound_test_start(void) {
  SceneSoundTest *scene = calloc(1, sizeof(SceneSoundTest));
  scene->song = 0;
  memset(scene->spectrum_left, 0, sizeof(float) * 158);
  memset(scene->spectrum_right, 0, sizeof(float) * 158);

  Config *const config = config_get();

  // +Spectrum UI
  scene->spectrum = surface_create(0, 0, config->ascii_width, config->ascii_height);
  SurfaceRectTiles rect_tiles = {
    '+',
    '-',
    '+',
    '|',
    0,
    '|',
    '+',
    '-',
    '+',
  };
  surface_rect(scene->spectrum, 0, 0, config->ascii_width, 30, rect_tiles, true, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });
  surface_textc(scene->spectrum, 2, 29, 75, " #{ffffff}P#{c8c8c8}lay | #{ffffff}S#{c8c8c8}top | #{ffffff}N#{c8c8c8}ext ");
  // -Spectrum UI

  screen_hook_render(&scene_sound_test_internal_render_hook, scene, 0);

  return scene;
}

void scene_sound_test_stop(void **scene) {
  SceneSoundTest *ptr = *scene;
  assert(ptr && scene);

  screen_unhook_render(&scene_sound_test_internal_render_hook, ptr);
  surface_destroy(&ptr->spectrum);

  free(ptr);
  *scene = NULL;
}

void scene_sound_test_update(SceneSoundTest *scene, double delta) {
  assert(scene);

  for (uint32_t t = 0; t < 158; t++) {
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

void scene_sound_test_message(SceneSoundTest *scene, uint32_t id, RectifyMap *const map) {
  assert(scene);

  switch (id) {
    case MSG_INPUT_KEY: {
      PicassoKey key = *(uint32_t *const)rectify_map_get(map, "key");
      bool pressed = *(bool *const)rectify_map_get(map, "pressed");

      if (pressed) {
        switch (key) {
          case PICASSO_KEY_P: {
            RectifyMap *map = rectify_map_create();
            rectify_map_set(map, "song", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), &scene->song);
            kronos_emit(MSG_SOUND_PLAY_SONG, map);
            break;
          }

          case PICASSO_KEY_S: {
            kronos_emit(MSG_SOUND_STOP_SONG, NULL);
            for (uint32_t t = 0; t < 158; t++) {
              scene->spectrum_left[t] = 0.0f;
              scene->spectrum_right[t] = 0.0f;
            }
            break;
          }

          case PICASSO_KEY_N: {
            kronos_emit(MSG_SOUND_STOP_SONG, NULL);

            scene->song = (scene->song == 0 ? 1 : 0);
            RectifyMap *map = rectify_map_create();
            rectify_map_set(map, "song", sizeof(uint32_t), RECTIFY_MAP_TYPE_UINT, &scene->song);
            kronos_emit(MSG_SOUND_PLAY_SONG, map);
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

      scene->song = *song;

      uint32_t step = 1;
      for (uint32_t t = 0, u = 0; u < 158; t += step, u++) {
        float l = 0.0f;
        float r = 0.0f;
        for (uint32_t s = t; s < t + step; s++) {
          l += left[s];
          r += right[s];
        }
        l = l / (float)step;
        r = r / (float)step;
        scene->spectrum_left[u] = l * 10.0f;
        scene->spectrum_right[u] = r * 10.0f;
      }
      break;
    }
  }
}

void scene_sound_test_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  SceneSoundTest *scene = userdata;
  surface_draw(scene->spectrum, screen);
}