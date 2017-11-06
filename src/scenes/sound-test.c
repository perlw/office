#include <assert.h>
#include <string.h>

#include "arkanis/math_3d.h"

#define USE_KRONOS
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#define USE_ASCII
#define USE_CONFIG
#define USE_MESSAGES
#include "main.h"

typedef struct {
  uint32_t song;
  float spectrum_left[158];
  float spectrum_right[158];

  uint32_t selected_sound;
  char **sounds;

  Surface *spectrum;
} SceneSoundTest;

SceneSoundTest *scene_sound_test_start(void);
void scene_sound_test_stop(void **scene);
void scene_sound_test_update(SceneSoundTest *scene, double delta);
RectifyMap *scene_sound_test_message(SceneSoundTest *scene, uint32_t id, RectifyMap *const map);

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

  scene->selected_sound = 0;
  scene->sounds = rectify_array_alloc(10, sizeof(char *));
  kronos_post("sound", MSG_SOUND_LIST, NULL, "scene_sound-test");
  surface_rect(scene->spectrum, 0, 31, 10, rectify_array_size(scene->sounds) + 2, rect_tiles, true, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });

  return scene;
}

void scene_sound_test_stop(void **scene) {
  SceneSoundTest *ptr = *scene;
  assert(ptr && scene);

  surface_destroy(&ptr->spectrum);

  for (uint32_t t = 0; t < rectify_array_size(ptr->sounds); t++) {
    free(ptr->sounds[t]);
  }
  void *sounds = (void *)ptr->sounds;
  rectify_array_free(&sounds);

  free(ptr);
  *scene = NULL;
}

void scene_sound_test_update(SceneSoundTest *scene, double delta) {
  assert(scene);

  for (uint32_t t = 0; t < scene->spectrum->width - 2; t++) {
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

  // +Sound list
  for (uint32_t t = 0; t < rectify_array_size(scene->sounds); t++) {
    GlyphColor fore = glyphcolor_hex(0xc8c8c8);
    GlyphColor back = glyphcolor_hex(0x0);
    if (scene->selected_sound == t) {
      fore = glyphcolor_hex(0xffffff);
      back = glyphcolor_hex(0x666666);
    }
    surface_text(scene->spectrum, 1, 32 + t, 0, scene->sounds[t], fore, back);
  }
  // -Sound list
}

RectifyMap *scene_sound_test_message(SceneSoundTest *scene, uint32_t id, RectifyMap *const map) {
  assert(scene);

  switch (id) {
    case MSG_INPUT_KEY: {
      PicassoKey key = rectify_map_get_uint(map, "key");
      bool pressed = rectify_map_get_bool(map, "pressed");

      if (pressed) {
        switch (key) {
          case PICASSO_KEY_UP: {
            scene->selected_sound -= (scene->selected_sound > 0 ? 1 : 0);
            break;
          }

          case PICASSO_KEY_DOWN: {
            scene->selected_sound += (scene->selected_sound < 2 ? 1 : 0);
            break;
          }

          case PICASSO_KEY_ENTER: {
            RectifyMap *map = rectify_map_create();
            rectify_map_set_string(map, "sound", scene->sounds[scene->selected_sound]);
            kronos_post("sound", MSG_SOUND_PLAY, map, NULL);
            break;
          }

          case PICASSO_KEY_P: {
            RectifyMap *map = rectify_map_create();
            rectify_map_set_uint(map, "song", scene->song);
            kronos_post("sound", MSG_SOUND_PLAY_SONG, map, NULL);
            break;
          }

          case PICASSO_KEY_S: {
            kronos_post("sound", MSG_SOUND_STOP_SONG, NULL, NULL);
            for (uint32_t t = 0; t < scene->spectrum->width - 2; t++) {
              scene->spectrum_left[t] = 0.0f;
              scene->spectrum_right[t] = 0.0f;
            }
            break;
          }

          case PICASSO_KEY_N: {
            kronos_post("sound", MSG_SOUND_STOP_SONG, NULL, NULL);

            scene->song = (scene->song == 0 ? 1 : 0);
            RectifyMap *map = rectify_map_create();
            rectify_map_set_uint(map, "song", scene->song);
            kronos_post("sound", MSG_SOUND_PLAY_SONG, map, NULL);
            break;
          }
        }
      }
      break;
    }

    case MSG_SOUND_SPECTRUM: {
      uint32_t song = rectify_map_get_uint(map, "song");
      float *const left = rectify_map_get(map, "left");
      float *const right = rectify_map_get(map, "right");
      if (!left || !right) {
        break;
      }

      scene->song = song;

      uint32_t step = 1;
      for (uint32_t t = 0, u = 0; u < scene->spectrum->width - 2; t += step, u++) {
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

    case MSG_SOUND_LIST: {
      char **sounds = (char **)rectify_map_get(map, "sounds");
      uint32_t num = rectify_map_get_uint(map, "num");

      for (uint32_t t = 0; t < num; t++) {
        char *sound = rectify_memory_alloc_copy(sounds[t], strnlen(sounds[t], 128) + 1);
        scene->sounds = rectify_array_push(scene->sounds, &sound);
      }
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
      surface_rect(scene->spectrum, 0, 31, 10, rectify_array_size(scene->sounds) + 2, rect_tiles, true, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });

      break;
    }

    case MSG_SYSTEM_RENDER: {
      surface_draw(scene->spectrum, *(AsciiBuffer **)rectify_map_get(map, "screen"));
      break;
    }
  }

  return NULL;
}
