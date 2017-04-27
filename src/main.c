#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "glad/glad.h"

#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "config.h"
#include "input.h"
#include "messages.h"
#include "screen.h"
#include "sound.h"

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
  input->surface->asciimap[0].fore = 255;
  for (uint32_t t = 1; t < width; t++) {
    input->surface->asciimap[t].rune = ' ';
    input->surface->asciimap[t].fore = 255;
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
    input->surface->asciimap[input->cursor_pos].fore = 255;
  } else {
    input->surface->asciimap[input->cursor_pos].rune = ' ';
    input->surface->asciimap[input->cursor_pos].fore = 0;
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
      input->surface->asciimap[0].fore = 255;
      for (uint32_t t = 1; t < input->cursor_max_pos + 1; t++) {
        input->surface->asciimap[t].rune = ' ';
        input->surface->asciimap[t].fore = 0;
      }

      return;
    }

    if (event->key == PICASSO_KEY_BACKSPACE) {
      if (input->cursor_pos > 0) {
        input->surface->asciimap[input->cursor_pos].rune = ' ';
        input->surface->asciimap[input->cursor_pos].fore = 0;
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
        input->surface->asciimap[input->cursor_pos].fore = 255;
        input->cursor_pos++;
      }
    }
  }
}
// -TextInput

// +Scene
typedef struct {
  double offset;
  double timing;
  double since_update;
  uint32_t frames;
  char fps_buffer[16];
  double current_second;
  bool dirty;

  Screen *screen;

  Surface *surface;
  Surface *surface2;
  Surface *surface3;

  Surface *fps_surface;

  TextInput *input;
} Scene;

Scene *scene_create(const Config *config) {
  Scene *scene = calloc(1, sizeof(Scene));

  scene->offset = 0.0;
  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;
  scene->frames = 0;
  snprintf(scene->fps_buffer, 16, "FPS: 0");
  scene->current_second = 0.0;
  scene->dirty = true;
  scene->screen = screen_create(config);

  scene->surface = surface_create(scene->screen, 0, 0, config->ascii_width, config->ascii_height);
  scene->surface2 = surface_create(scene->screen, 0, 0, 32, 32);
  scene->surface3 = surface_create(scene->screen, 24, 16, 32, 32);

  scene->fps_surface = surface_create(scene->screen, 0, 0, 16, 1);
  surface_text(scene->fps_surface, 0, 0, 16, scene->fps_buffer);

  scene->input = textinput_create(scene->screen, 1, config->ascii_height - 2, config->ascii_width - 2);

  return scene;
}

void scene_destroy(Scene *scene) {
  assert(scene);

  textinput_destroy(scene->input);

  surface_destroy(scene->fps_surface);
  surface_destroy(scene->surface3);
  surface_destroy(scene->surface2);
  surface_destroy(scene->surface);
  screen_destroy(scene->screen);

  free(scene);
}

void scene_update(Scene *scene, double delta) {
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
          scene->surface->asciimap[i].fore = color;
          scene->surface->asciimap[i].back = 0;
        }
      }
    }

    // Moving surface
    {
      scene->surface3->x = 24 + (cos(scene->offset) * 12);
    }

    textinput_update(scene->input, scene->timing);

    scene->dirty = true;
  }

  scene->current_second += delta;
  if (scene->current_second >= 1) {
    snprintf(scene->fps_buffer, 16, "FPS: %d", scene->frames);
    surface_text(scene->fps_surface, 0, 0, 16, scene->fps_buffer);

    scene->current_second = 0;
    scene->frames = 0;
    scene->dirty = true;
  }
}

void scene_draw(Scene *scene) {
  assert(scene);

  screen_draw(scene->screen, scene->dirty);

  scene->dirty = false;
  scene->frames++;
}
// -Scene

bool quit_game = false;
void game_kill_event(int32_t id, void *subscriberdata, void *userdata) {
  quit_game = true;
}

int main() {
  srand(time(NULL));

  Muse *muse = muse_create();

  input_action_callback(&input_action, muse);

  input_init();

  Config config = read_config();

  if (picasso_window_init("Office", config.res_width, config.res_height, config.gl_debug) != PICASSO_WINDOW_OK) {
    printf("Window: failed to init\n");
    return -1;
  }
  picasso_window_keyboard_callback(&input_keyboard_callback);

  MuseFunctionDef action_def = {
    .name = "action",
    .func = &lua_action,
    .num_arguments = 2,
    .arguments = (MuseType[]){
      MUSE_TYPE_STRING,
      MUSE_TYPE_FUNCTION,
    },
    .userdata = NULL,
  };
  muse_add_func(muse, &action_def);
  muse_load_file(muse, "main.lua");

  SoundSys *soundsys = soundsys_create();
  Scene *scene = scene_create(&config);

  double last_tick = bedrock_time();

  double frame_timing = (config.frame_lock > 0 ? 1.0 / (double)config.frame_lock : 0);
  double next_frame = frame_timing;

  gossip_subscribe(MSG_GAME_KILL, &game_kill_event, NULL);

  gossip_emit(MSG_GAME_INIT, NULL);

  while (!picasso_window_should_close() && !quit_game) {
    double tick = bedrock_time();
    double delta = tick - last_tick;
    last_tick = tick;

    MuseArgument arg = {
      .type = MUSE_TYPE_NUMBER,
      .argument = &delta,
    };
    muse_call_name(muse, "update", 1, &arg, 0, NULL);

    soundsys_update(soundsys, delta);
    scene_update(scene, delta);

    next_frame += delta;
    if (next_frame >= frame_timing) {
      next_frame = 0.0;

      picasso_window_clear();
      scene_draw(scene);
      picasso_window_swap();
    }

    picasso_window_update();
  }

  gossip_cleanup();

  scene_destroy(scene);
  soundsys_destroy(soundsys);
  input_kill();

  muse_destroy(muse);
  picasso_window_kill();

#ifdef MEM_DEBUG
  occulus_print(false);
#endif

  return 0;
}
