#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "arkanis/math_3d.h"

#include "config.h"
#include "messages.h"
#include "screen.h"

// +TextInput
typedef struct {
  double cursor_blink_timing;
  double cursor_since_blink;
  bool cursor_visible;
  uint8_t cursor_pos;
  uint32_t cursor_max_pos;
  char buffer[128];
  Surface *surface;
} TextInput;

void textinput_event(int32_t id, void *subscriberdata, void *userdata);

TextInput *textinput_create(Screen *screen, uint32_t x, uint32_t y, uint32_t width) {
  TextInput *input = calloc(1, sizeof(TextInput));

  input->cursor_blink_timing = 0.75;
  input->cursor_pos = 0;
  input->cursor_max_pos = width - 1;
  input->cursor_visible = true;
  memset(input->buffer, 0, 128);

  input->surface = surface_create(screen, x, y, width, 1);
  input->surface->asciimap[0].rune = 219;
  input->surface->asciimap[0].fore = (GlyphColor){ 255, 255, 255 };
  for (uint32_t t = 1; t < width; t++) {
    input->surface->asciimap[t].rune = ' ';
    input->surface->asciimap[t].fore = (GlyphColor){ 255, 255, 255 };
  }

  gossip_subscribe(MSG_INPUT_KEYBOARD, &textinput_event, input);

  return input;
}

void textinput_destroy(TextInput *input) {
  assert(input);

  surface_destroy(input->surface);

  free(input);
}

void textinput_update(TextInput *input, double delta) {
  input->cursor_since_blink += delta;
  while (input->cursor_since_blink >= input->cursor_blink_timing) {
    input->cursor_since_blink -= input->cursor_blink_timing;

    input->cursor_visible = !input->cursor_visible;
  }

  if (input->cursor_visible) {
    input->surface->asciimap[input->cursor_pos].rune = 219;
    input->surface->asciimap[input->cursor_pos].fore = (GlyphColor){ 255, 255, 255 };
  } else {
    input->surface->asciimap[input->cursor_pos].rune = ' ';
    input->surface->asciimap[input->cursor_pos].fore = (GlyphColor){ 0, 0, 0 };
  }
}

void textinput_event(int32_t id, void *subscriberdata, void *userdata) {
  TextInput *input = (TextInput *)subscriberdata;
  PicassoWindowInputEvent *event = (PicassoWindowInputEvent *)userdata;

  if (event->pressed) {
    gossip_emit(MSG_SOUND_PLAY_TAP, NULL);

    if (event->key == PICASSO_KEY_ENTER) {
      input->cursor_pos = 0;
      input->cursor_visible = true;
      printf("TEXTINPUT: YOU WROTE \"%s\"\n", input->buffer);
      memset(input->buffer, 0, 128);

      input->surface->asciimap[0].rune = 219;
      input->surface->asciimap[0].fore = (GlyphColor){ 255, 255, 255 };
      for (uint32_t t = 1; t < input->cursor_max_pos + 1; t++) {
        input->surface->asciimap[t].rune = ' ';
        input->surface->asciimap[t].fore = (GlyphColor){ 0, 0, 0 };
      }

      return;
    }

    if (event->key == PICASSO_KEY_BACKSPACE) {
      if (input->cursor_pos > 0) {
        input->surface->asciimap[input->cursor_pos].rune = ' ';
        input->surface->asciimap[input->cursor_pos].fore = (GlyphColor){ 0, 0, 0 };
        input->cursor_visible = true;
        input->cursor_pos--;
        input->buffer[input->cursor_pos] = 0;
      }
    }

    if (event->key >= 32 && event->key <= 96) {
      if (input->cursor_pos < input->cursor_max_pos) {
        uint8_t offset = 0;
        if (event->shift) {
          offset = (event->key < 65 ? -16 : 0);
        } else {
          offset = (event->key >= 65 && event->key <= 90 ? +32 : 0);
        }

        input->buffer[input->cursor_pos] = event->key + offset;

        input->surface->asciimap[input->cursor_pos + 1] = input->surface->asciimap[input->cursor_pos];
        input->surface->asciimap[input->cursor_pos].rune = event->key + offset;
        input->surface->asciimap[input->cursor_pos].fore = (GlyphColor){ 255, 255, 255 };
        input->cursor_pos++;
      }
    }
  }
}
// -TextInput

// +SceneTest
typedef struct {
  double offset;
  double timing;
  double since_update;
  uint32_t frames;
  char fps_buffer[32];
  double current_second;
  bool dirty;

  Screen *screen;

  Surface *surface;
  Surface *surface2;
  Surface *surface3;

  Surface *fps_surface;

  TextInput *input;
} SceneTest;

SceneTest *scene_test_create(Screen *screen, const Config *config) {
  SceneTest *scene = calloc(1, sizeof(SceneTest));

  scene->offset = 0.0;
  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;
  scene->frames = 0;
  snprintf(scene->fps_buffer, 32, "FPS: 0 | MEM: 0.00kb");
  scene->current_second = 0.0;
  scene->screen = screen;

  scene->surface = surface_create(scene->screen, 0, 0, config->ascii_width, config->ascii_height);
  scene->surface2 = surface_create(scene->screen, 0, 0, 32, 32);
  scene->surface3 = surface_create(scene->screen, 24, 16, 32, 32);

  scene->fps_surface = surface_create(scene->screen, 0, 0, 32, 1);
  surface_text(scene->fps_surface, 0, 0, 32, scene->fps_buffer);

  scene->input = textinput_create(scene->screen, 1, config->ascii_height - 2, config->ascii_width - 2);

  return scene;
}

void scene_test_destroy(SceneTest *scene) {
  assert(scene);

  textinput_destroy(scene->input);

  surface_destroy(scene->fps_surface);
  surface_destroy(scene->surface3);
  surface_destroy(scene->surface2);
  surface_destroy(scene->surface);

  free(scene);
}

void scene_test_update(SceneTest *scene, double delta) {
  assert(scene);

  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    // Wave
    {
      scene->offset += 0.1;

      double wave_depth = 0.25;
      double wave_thickness = M_PI * 4.0;
      double cx = scene->surface->width / 2;
      double cy = scene->surface->height / 2;
      for (uintmax_t y = 0; y < scene->surface->height; y++) {
        for (uintmax_t x = 0; x < scene->surface->width; x++) {
          double dx = fabs((double)x - cx);
          double dy = fabs((double)y - cy);
          double dist = sqrt(pow(dx, 2) + pow(dy, 2));
          double ndist = dist / 50.0;

          double final_color = ((cos((ndist * wave_thickness) + scene->offset) + 1.0) / 4.0) + wave_depth;

          uintmax_t i = (y * scene->surface->width) + x;
          uint8_t color = (uintmax_t)(final_color * 255.0);
          if (color < 96) {
            scene->surface->asciimap[i].rune = '.';
          } else if (color < 178) {
            scene->surface->asciimap[i].rune = '+';
          } else {
            scene->surface->asciimap[i].rune = '*';
          }
          scene->surface->asciimap[i].fore.r = (uint8_t)(255.0 * (1.0 - final_color));
          scene->surface->asciimap[i].fore.g = (uint8_t)(255.0 * final_color);
          scene->surface->asciimap[i].fore.b = 255;
        }
      }
    }

    // Moving surface
    {
      scene->surface3->x = 24 + (cos(scene->offset) * 12);
    }

    textinput_update(scene->input, scene->timing);
  }

  scene->current_second += delta;
  if (scene->current_second >= 1) {
    snprintf(scene->fps_buffer, 32, "FPS: %d | MEM: %.2fkb", scene->frames, (double)occulus_current_allocated() / 1024.0);
    surface_text(scene->fps_surface, 0, 0, 32, scene->fps_buffer);

    scene->current_second = 0;
    scene->frames = 0;
  }
}

void scene_test_draw(SceneTest *scene) {
  assert(scene);

  screen_draw(scene->screen);
  scene->frames++;
}
// -Scene
