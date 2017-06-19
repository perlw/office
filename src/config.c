#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "config.h"
#include "input.h"

void set_resolution(Muse *const muse, uintmax_t num_arguments, const MuseArgument *const arguments, const void *const userdata) {
  Config *config = (Config *)userdata;
  config->res_width = (uint32_t) * (double *)arguments[0].argument;
  config->res_height = (uint32_t) * (double *)arguments[1].argument;
}

void set_gl_debug(Muse *const muse, uintmax_t num_arguments, const MuseArgument *const arguments, const void *const userdata) {
  Config *config = (Config *)userdata;
  config->gl_debug = *(bool *)arguments[0].argument;
}

void set_frame_lock(Muse *const muse, uintmax_t num_arguments, const MuseArgument *const arguments, const void *const userdata) {
  Config *config = (Config *)userdata;
  config->frame_lock = (uint32_t) * (double *)arguments[0].argument;
}

void set_key_bind(Muse *const muse, uintmax_t num_arguments, const MuseArgument *const arguments, const void *const userdata) {
  char *action = (char *)arguments[0].argument;
  int32_t key = (int32_t) * (double *)arguments[1].argument;

  InputActionBinding binding = (InputActionBinding){
    .action = action,
    .key = key,
  };

  input_action_add_binding(&binding);
}

void set_ascii_resolution(Muse *const muse, uintmax_t num_arguments, const MuseArgument *const arguments, const void *const userdata) {
  Config *config = (Config *)userdata;
  config->ascii_width = (uint32_t) * (double *)arguments[0].argument;
  config->ascii_height = (uint32_t) * (double *)arguments[1].argument;
}

Config read_config(void) {
  Config config = {
    .res_width = 640,
    .res_height = 480,
    .gl_debug = false,
    .frame_lock = 0,
    .ascii_width = 80,
    .ascii_height = 60,
  };

  MuseFunctionDef resolution_def = {
    .name = "resolution",
    .func = &set_resolution,
    .num_arguments = 2,
    .arguments = (MuseType[]){
      MUSE_TYPE_NUMBER,
      MUSE_TYPE_NUMBER,
    },
    .userdata = &config,
  };
  MuseFunctionDef gl_debug_def = {
    .name = "gl_debug",
    .func = &set_gl_debug,
    .num_arguments = 1,
    .arguments = (MuseType[]){
      MUSE_TYPE_BOOLEAN,
    },
    .userdata = &config,
  };
  MuseFunctionDef frame_lock_def = {
    .name = "frame_lock",
    .func = &set_frame_lock,
    .num_arguments = 1,
    .arguments = (MuseType[]){
      MUSE_TYPE_NUMBER,
    },
    .userdata = &config,
  };
  MuseFunctionDef bind_def = {
    .name = "bind",
    .func = &set_key_bind,
    .num_arguments = 2,
    .arguments = (MuseType[]){
      MUSE_TYPE_STRING,
      MUSE_TYPE_NUMBER,
    },
    .userdata = &config,
  };
  MuseFunctionDef ascii_resolution_def = {
    .name = "ascii_resolution",
    .func = &set_ascii_resolution,
    .num_arguments = 2,
    .arguments = (MuseType[]){
      MUSE_TYPE_NUMBER,
      MUSE_TYPE_NUMBER,
    },
    .userdata = &config,
  };

  Muse *muse = muse_create_lite();

  muse_add_func(muse, &resolution_def);
  muse_add_func(muse, &gl_debug_def);
  muse_add_func(muse, &frame_lock_def);
  muse_add_func(muse, &bind_def);
  muse_add_func(muse, &ascii_resolution_def);

  muse_set_global_number(muse, "KEY_SPACE", PICASSO_KEY_SPACE);
  muse_set_global_number(muse, "KEY_APOSTROPHE", PICASSO_KEY_APOSTROPHE);
  muse_set_global_number(muse, "KEY_COMMA", PICASSO_KEY_COMMA);
  muse_set_global_number(muse, "KEY_MINUS", PICASSO_KEY_MINUS);
  muse_set_global_number(muse, "KEY_PERIOD", PICASSO_KEY_PERIOD);
  muse_set_global_number(muse, "KEY_SLASH", PICASSO_KEY_SLASH);
  muse_set_global_number(muse, "KEY_0", PICASSO_KEY_0);
  muse_set_global_number(muse, "KEY_1", PICASSO_KEY_1);
  muse_set_global_number(muse, "KEY_2", PICASSO_KEY_2);
  muse_set_global_number(muse, "KEY_3", PICASSO_KEY_3);
  muse_set_global_number(muse, "KEY_4", PICASSO_KEY_4);
  muse_set_global_number(muse, "KEY_5", PICASSO_KEY_5);
  muse_set_global_number(muse, "KEY_6", PICASSO_KEY_6);
  muse_set_global_number(muse, "KEY_7", PICASSO_KEY_7);
  muse_set_global_number(muse, "KEY_8", PICASSO_KEY_8);
  muse_set_global_number(muse, "KEY_9", PICASSO_KEY_9);
  muse_set_global_number(muse, "KEY_SEMICOLON", PICASSO_KEY_SEMICOLON);
  muse_set_global_number(muse, "KEY_EQUAL", PICASSO_KEY_EQUAL);
  muse_set_global_number(muse, "KEY_A", PICASSO_KEY_A);
  muse_set_global_number(muse, "KEY_B", PICASSO_KEY_B);
  muse_set_global_number(muse, "KEY_C", PICASSO_KEY_C);
  muse_set_global_number(muse, "KEY_D", PICASSO_KEY_D);
  muse_set_global_number(muse, "KEY_E", PICASSO_KEY_E);
  muse_set_global_number(muse, "KEY_F", PICASSO_KEY_F);
  muse_set_global_number(muse, "KEY_G", PICASSO_KEY_G);
  muse_set_global_number(muse, "KEY_H", PICASSO_KEY_H);
  muse_set_global_number(muse, "KEY_I", PICASSO_KEY_I);
  muse_set_global_number(muse, "KEY_J", PICASSO_KEY_J);
  muse_set_global_number(muse, "KEY_K", PICASSO_KEY_K);
  muse_set_global_number(muse, "KEY_L", PICASSO_KEY_L);
  muse_set_global_number(muse, "KEY_M", PICASSO_KEY_M);
  muse_set_global_number(muse, "KEY_N", PICASSO_KEY_N);
  muse_set_global_number(muse, "KEY_O", PICASSO_KEY_O);
  muse_set_global_number(muse, "KEY_P", PICASSO_KEY_P);
  muse_set_global_number(muse, "KEY_Q", PICASSO_KEY_Q);
  muse_set_global_number(muse, "KEY_R", PICASSO_KEY_R);
  muse_set_global_number(muse, "KEY_S", PICASSO_KEY_S);
  muse_set_global_number(muse, "KEY_T", PICASSO_KEY_T);
  muse_set_global_number(muse, "KEY_U", PICASSO_KEY_U);
  muse_set_global_number(muse, "KEY_V", PICASSO_KEY_V);
  muse_set_global_number(muse, "KEY_W", PICASSO_KEY_W);
  muse_set_global_number(muse, "KEY_X", PICASSO_KEY_X);
  muse_set_global_number(muse, "KEY_Y", PICASSO_KEY_Y);
  muse_set_global_number(muse, "KEY_Z", PICASSO_KEY_Z);
  muse_set_global_number(muse, "KEY_LEFT_BRACKET", PICASSO_KEY_LEFT_BRACKET);
  muse_set_global_number(muse, "KEY_BACKSLASH", PICASSO_KEY_BACKSLASH);
  muse_set_global_number(muse, "KEY_RIGHT_BRACKET", PICASSO_KEY_RIGHT_BRACKET);
  muse_set_global_number(muse, "KEY_GRAVE_ACCENT", PICASSO_KEY_GRAVE_ACCENT);
  muse_set_global_number(muse, "KEY_WORLD_1", PICASSO_KEY_WORLD_1);
  muse_set_global_number(muse, "KEY_WORLD_2", PICASSO_KEY_WORLD_2);

  muse_set_global_number(muse, "KEY_ESCAPE", PICASSO_KEY_ESCAPE);
  muse_set_global_number(muse, "KEY_ENTER", PICASSO_KEY_ENTER);
  muse_set_global_number(muse, "KEY_TAB", PICASSO_KEY_TAB);
  muse_set_global_number(muse, "KEY_BACKSPACE", PICASSO_KEY_BACKSPACE);
  muse_set_global_number(muse, "KEY_INSERT", PICASSO_KEY_INSERT);
  muse_set_global_number(muse, "KEY_DELETE", PICASSO_KEY_DELETE);
  muse_set_global_number(muse, "KEY_RIGHT", PICASSO_KEY_RIGHT);
  muse_set_global_number(muse, "KEY_LEFT", PICASSO_KEY_LEFT);
  muse_set_global_number(muse, "KEY_DOWN", PICASSO_KEY_DOWN);
  muse_set_global_number(muse, "KEY_UP", PICASSO_KEY_UP);
  muse_set_global_number(muse, "KEY_PAGE_UP", PICASSO_KEY_PAGE_UP);
  muse_set_global_number(muse, "KEY_PAGE_DOWN", PICASSO_KEY_PAGE_DOWN);
  muse_set_global_number(muse, "KEY_HOME", PICASSO_KEY_HOME);
  muse_set_global_number(muse, "KEY_END", PICASSO_KEY_END);
  muse_set_global_number(muse, "KEY_CAPS_LOCK", PICASSO_KEY_CAPS_LOCK);
  muse_set_global_number(muse, "KEY_SCROLL_LOCK", PICASSO_KEY_SCROLL_LOCK);
  muse_set_global_number(muse, "KEY_NUM_LOCK", PICASSO_KEY_NUM_LOCK);
  muse_set_global_number(muse, "KEY_PRINT_SCREEN", PICASSO_KEY_PRINT_SCREEN);
  muse_set_global_number(muse, "KEY_PAUSE", PICASSO_KEY_PAUSE);
  muse_set_global_number(muse, "KEY_F1", PICASSO_KEY_F1);
  muse_set_global_number(muse, "KEY_F2", PICASSO_KEY_F2);
  muse_set_global_number(muse, "KEY_F3", PICASSO_KEY_F3);
  muse_set_global_number(muse, "KEY_F4", PICASSO_KEY_F4);
  muse_set_global_number(muse, "KEY_F5", PICASSO_KEY_F5);
  muse_set_global_number(muse, "KEY_F6", PICASSO_KEY_F6);
  muse_set_global_number(muse, "KEY_F7", PICASSO_KEY_F7);
  muse_set_global_number(muse, "KEY_F8", PICASSO_KEY_F8);
  muse_set_global_number(muse, "KEY_F9", PICASSO_KEY_F9);
  muse_set_global_number(muse, "KEY_F10", PICASSO_KEY_F10);
  muse_set_global_number(muse, "KEY_F11", PICASSO_KEY_F11);
  muse_set_global_number(muse, "KEY_F12", PICASSO_KEY_F12);
  muse_set_global_number(muse, "KEY_F13", PICASSO_KEY_F13);
  muse_set_global_number(muse, "KEY_F14", PICASSO_KEY_F14);
  muse_set_global_number(muse, "KEY_F15", PICASSO_KEY_F15);
  muse_set_global_number(muse, "KEY_F16", PICASSO_KEY_F16);
  muse_set_global_number(muse, "KEY_F17", PICASSO_KEY_F17);
  muse_set_global_number(muse, "KEY_F18", PICASSO_KEY_F18);
  muse_set_global_number(muse, "KEY_F19", PICASSO_KEY_F19);
  muse_set_global_number(muse, "KEY_F20", PICASSO_KEY_F20);
  muse_set_global_number(muse, "KEY_F21", PICASSO_KEY_F21);
  muse_set_global_number(muse, "KEY_F22", PICASSO_KEY_F22);
  muse_set_global_number(muse, "KEY_F23", PICASSO_KEY_F23);
  muse_set_global_number(muse, "KEY_F24", PICASSO_KEY_F24);
  muse_set_global_number(muse, "KEY_F25", PICASSO_KEY_F25);
  muse_set_global_number(muse, "KEY_KP_0", PICASSO_KEY_KP_0);
  muse_set_global_number(muse, "KEY_KP_1", PICASSO_KEY_KP_1);
  muse_set_global_number(muse, "KEY_KP_2", PICASSO_KEY_KP_2);
  muse_set_global_number(muse, "KEY_KP_3", PICASSO_KEY_KP_3);
  muse_set_global_number(muse, "KEY_KP_4", PICASSO_KEY_KP_4);
  muse_set_global_number(muse, "KEY_KP_5", PICASSO_KEY_KP_5);
  muse_set_global_number(muse, "KEY_KP_6", PICASSO_KEY_KP_6);
  muse_set_global_number(muse, "KEY_KP_7", PICASSO_KEY_KP_7);
  muse_set_global_number(muse, "KEY_KP_8", PICASSO_KEY_KP_8);
  muse_set_global_number(muse, "KEY_KP_9", PICASSO_KEY_KP_9);
  muse_set_global_number(muse, "KEY_KP_DECIMAL", PICASSO_KEY_KP_DECIMAL);
  muse_set_global_number(muse, "KEY_KP_DIVIDE", PICASSO_KEY_KP_DIVIDE);
  muse_set_global_number(muse, "KEY_KP_MULTIPLY", PICASSO_KEY_KP_MULTIPLY);
  muse_set_global_number(muse, "KEY_KP_SUBTRACT", PICASSO_KEY_KP_SUBTRACT);
  muse_set_global_number(muse, "KEY_KP_ADD", PICASSO_KEY_KP_ADD);
  muse_set_global_number(muse, "KEY_KP_ENTER", PICASSO_KEY_KP_ENTER);
  muse_set_global_number(muse, "KEY_KP_EQUAL", PICASSO_KEY_KP_EQUAL);
  muse_set_global_number(muse, "KEY_LEFT_SHIFT", PICASSO_KEY_LEFT_SHIFT);
  muse_set_global_number(muse, "KEY_LEFT_CONTROL", PICASSO_KEY_LEFT_CONTROL);
  muse_set_global_number(muse, "KEY_LEFT_ALT", PICASSO_KEY_LEFT_ALT);
  muse_set_global_number(muse, "KEY_LEFT_SUPER", PICASSO_KEY_LEFT_SUPER);
  muse_set_global_number(muse, "KEY_RIGHT_SHIFT", PICASSO_KEY_RIGHT_SHIFT);
  muse_set_global_number(muse, "KEY_RIGHT_CONTROL", PICASSO_KEY_RIGHT_CONTROL);
  muse_set_global_number(muse, "KEY_RIGHT_ALT", PICASSO_KEY_RIGHT_ALT);
  muse_set_global_number(muse, "KEY_RIGHT_SUPER", PICASSO_KEY_RIGHT_SUPER);
  muse_set_global_number(muse, "KEY_MENU", PICASSO_KEY_MENU);

  muse_load_file(muse, "config.lua");
  muse_destroy(muse);

  return config;
}
