#include <assert.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"

// +TextInput
typedef struct {
  double cursor_blink_timing;
  double cursor_since_blink;
  bool cursor_visible;
  uint8_t cursor_pos;
  uint32_t cursor_max_pos;
  char buffer[128];
  Surface *surface;
  GossipHandle input_handle;
} TextInput;

void textinput_event(const char *message, void *const subscriberdata, void *const userdata);

TextInput *textinput_create(uint32_t x, uint32_t y, uint32_t width) {
  TextInput *input = calloc(1, sizeof(TextInput));

  input->cursor_blink_timing = 0.75;
  input->cursor_pos = 0;
  input->cursor_max_pos = width - 1;
  input->cursor_visible = true;
  memset(input->buffer, 0, 128);

  input->surface = surface_create(x, y, width, 1);
  input->surface->buffer[0].rune = 219;
  input->surface->buffer[0].fore = (GlyphColor){ 255, 255, 255 };
  for (uint32_t t = 1; t < width; t++) {
    input->surface->buffer[t].rune = ' ';
    input->surface->buffer[t].fore = (GlyphColor){ 255, 255, 255 };
  }

  input->input_handle = gossip_subscribe("input:keyboard", &textinput_event, input);

  return input;
}

void textinput_destroy(TextInput *input) {
  assert(input);

  gossip_unsubscribe(input->input_handle);

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
    input->surface->buffer[input->cursor_pos].rune = 219;
    input->surface->buffer[input->cursor_pos].fore = (GlyphColor){ 255, 255, 255 };
  } else {
    input->surface->buffer[input->cursor_pos].rune = ' ';
    input->surface->buffer[input->cursor_pos].fore = (GlyphColor){ 0, 0, 0 };
  }
}

void textinput_event(const char *message, void *const subscriberdata, void *const userdata) {
  TextInput *input = (TextInput *)subscriberdata;
  PicassoWindowKeyboardEvent *event = (PicassoWindowKeyboardEvent *)userdata;

  if (event->pressed) {
    gossip_emit("sound:play_tap", NULL);

    if (event->key == PICASSO_KEY_ENTER) {
      input->cursor_pos = 0;
      input->cursor_visible = true;
      printf("TEXTINPUT: YOU WROTE \"%s\"\n", input->buffer);
      if (strcmp(input->buffer, "quit") == 0) {
        gossip_emit("game:kill", NULL);
      }

      memset(input->buffer, 0, 128);
      input->surface->buffer[0].rune = 219;
      input->surface->buffer[0].fore = (GlyphColor){ 255, 255, 255 };
      for (uint32_t t = 1; t < input->cursor_max_pos + 1; t++) {
        input->surface->buffer[t].rune = ' ';
        input->surface->buffer[t].fore = (GlyphColor){ 0, 0, 0 };
      }

      return;
    }

    if (event->key == PICASSO_KEY_BACKSPACE) {
      if (input->cursor_pos > 0) {
        input->surface->buffer[input->cursor_pos].rune = ' ';
        input->surface->buffer[input->cursor_pos].fore = (GlyphColor){ 0, 0, 0 };
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

        input->surface->buffer[input->cursor_pos + 1] = input->surface->buffer[input->cursor_pos];
        input->surface->buffer[input->cursor_pos].rune = event->key + offset;
        input->surface->buffer[input->cursor_pos].fore = (GlyphColor){ 255, 255, 255 };
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

  Surface *surface;

  TextInput *input;
} SceneTest;

SceneTest *scene_test_create(void) {
  SceneTest *scene = calloc(1, sizeof(SceneTest));

  const Config *const config = config_get();

  scene->offset = 0.0;
  scene->timing = 1.0 / 30.0;
  scene->since_update = 1.0 / (double)((rand() % 29) + 1);

  scene->surface = surface_create(0, 0, config->ascii_width, config->ascii_height);

  scene->input = textinput_create(1, config->ascii_height - 2, config->ascii_width - 2);

  return scene;
}

void scene_test_destroy(SceneTest *const scene) {
  assert(scene);

  textinput_destroy(scene->input);

  surface_destroy(scene->surface);

  free(scene);
}

void scene_test_update(SceneTest *const scene, double delta) {
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
          uint8_t color = (uint8_t)(final_color * 255.0);
          if (color < 96) {
            scene->surface->buffer[i].rune = '.';
          } else if (color < 178) {
            scene->surface->buffer[i].rune = '+';
          } else {
            scene->surface->buffer[i].rune = '*';
          }
          scene->surface->buffer[i].fore.r = (uint8_t)(255.0 * (1.0 - final_color));
          scene->surface->buffer[i].fore.g = (uint8_t)(255.0 * final_color);
          scene->surface->buffer[i].fore.b = 255;
        }
      }
    }

    textinput_update(scene->input, scene->timing);
  }
}

void scene_test_draw(SceneTest *const scene, AsciiBuffer *const screen) {
  assert(scene);

  surface_draw(scene->surface, screen);
  surface_draw(scene->input->surface, screen);
}
// -Scene
