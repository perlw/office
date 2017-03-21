#include <stdio.h>
#include <string.h>

#include "config.h"

void set_resolution(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata) {
  double width = *(double*)arguments[0].argument;
  double height = *(double*)arguments[1].argument;

  Config *config = (Config*)userdata;
  config->res_width = (uint32_t)width;
  config->res_height = (uint32_t)height;
}

void gl_debug(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata) {
  Config *config = (Config*)userdata;
  config->gl_debug = *(bool*)arguments[0].argument;
}

void set_frame_lock(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata) {
  Config *config = (Config*)userdata;
  config->frame_lock = (uint32_t)*(double*)arguments[0].argument;
}

void key_bind(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata) {
  char *action = (char*)arguments[0].argument;
  int32_t key = (int32_t)*(double*)arguments[1].argument;

  NeglectBinding binding = (NeglectBinding){
    .action = action,
    .key = key,
    .callback = NULL,
  };

  neglect_add_binding(&binding);
}

Config read_config(void) {
  Config config = {
    .res_width = 640,
    .res_height = 480,
    .gl_debug = false,
    .frame_lock = 0,
  };

  MuseFunctionDef set_resolution_def = {
    .name = "resolution",
    .func = &set_resolution,
    .num_arguments = 2,
    .arguments = (MuseArgumentType[]){
      MUSE_ARGUMENT_NUMBER,
      MUSE_ARGUMENT_NUMBER,
    },
    .userdata = &config,
  };
  MuseFunctionDef gl_debug_def = {
    .name = "gl_debug",
    .func = &gl_debug,
    .num_arguments = 1,
    .arguments = (MuseArgumentType[]){
      MUSE_ARGUMENT_BOOLEAN,
    },
    .userdata = &config,
  };
  MuseFunctionDef frame_lock_def = {
    .name = "frame_lock",
    .func = &set_frame_lock,
    .num_arguments = 1,
    .arguments = (MuseArgumentType[]){
      MUSE_ARGUMENT_NUMBER,
    },
    .userdata = &config,
  };
  MuseFunctionDef bind_def = {
    .name = "bind",
    .func = &key_bind,
    .num_arguments = 2,
    .arguments = (MuseArgumentType[]){
      MUSE_ARGUMENT_STRING,
      MUSE_ARGUMENT_NUMBER,
    },
    .userdata = &config,
  };

  Muse *muse = muse_create_lite();

  muse_add_func(muse, &set_resolution_def);
  muse_add_func(muse, &gl_debug_def);
  muse_add_func(muse, &frame_lock_def);
  muse_add_func(muse, &bind_def);

  muse_set_global_number(muse, "KEY_SPACE", NEGLECT_KEY_SPACE);
  muse_set_global_number(muse, "KEY_APOSTROPHE", NEGLECT_KEY_APOSTROPHE);
  muse_set_global_number(muse, "KEY_COMMA", NEGLECT_KEY_COMMA);
  muse_set_global_number(muse, "KEY_MINUS", NEGLECT_KEY_MINUS);
  muse_set_global_number(muse, "KEY_PERIOD", NEGLECT_KEY_PERIOD);
  muse_set_global_number(muse, "KEY_SLASH", NEGLECT_KEY_SLASH);
  muse_set_global_number(muse, "KEY_0", NEGLECT_KEY_0);
  muse_set_global_number(muse, "KEY_1", NEGLECT_KEY_1);
  muse_set_global_number(muse, "KEY_2", NEGLECT_KEY_2);
  muse_set_global_number(muse, "KEY_3", NEGLECT_KEY_3);
  muse_set_global_number(muse, "KEY_4", NEGLECT_KEY_4);
  muse_set_global_number(muse, "KEY_5", NEGLECT_KEY_5);
  muse_set_global_number(muse, "KEY_6", NEGLECT_KEY_6);
  muse_set_global_number(muse, "KEY_7", NEGLECT_KEY_7);
  muse_set_global_number(muse, "KEY_8", NEGLECT_KEY_8);
  muse_set_global_number(muse, "KEY_9", NEGLECT_KEY_9);
  muse_set_global_number(muse, "KEY_SEMICOLON", NEGLECT_KEY_SEMICOLON);
  muse_set_global_number(muse, "KEY_EQUAL", NEGLECT_KEY_EQUAL);
  muse_set_global_number(muse, "KEY_A", NEGLECT_KEY_A);
  muse_set_global_number(muse, "KEY_B", NEGLECT_KEY_B);
  muse_set_global_number(muse, "KEY_C", NEGLECT_KEY_C);
  muse_set_global_number(muse, "KEY_D", NEGLECT_KEY_D);
  muse_set_global_number(muse, "KEY_E", NEGLECT_KEY_E);
  muse_set_global_number(muse, "KEY_F", NEGLECT_KEY_F);
  muse_set_global_number(muse, "KEY_G", NEGLECT_KEY_G);
  muse_set_global_number(muse, "KEY_H", NEGLECT_KEY_H);
  muse_set_global_number(muse, "KEY_I", NEGLECT_KEY_I);
  muse_set_global_number(muse, "KEY_J", NEGLECT_KEY_J);
  muse_set_global_number(muse, "KEY_K", NEGLECT_KEY_K);
  muse_set_global_number(muse, "KEY_L", NEGLECT_KEY_L);
  muse_set_global_number(muse, "KEY_M", NEGLECT_KEY_M);
  muse_set_global_number(muse, "KEY_N", NEGLECT_KEY_N);
  muse_set_global_number(muse, "KEY_O", NEGLECT_KEY_O);
  muse_set_global_number(muse, "KEY_P", NEGLECT_KEY_P);
  muse_set_global_number(muse, "KEY_Q", NEGLECT_KEY_Q);
  muse_set_global_number(muse, "KEY_R", NEGLECT_KEY_R);
  muse_set_global_number(muse, "KEY_S", NEGLECT_KEY_S);
  muse_set_global_number(muse, "KEY_T", NEGLECT_KEY_T);
  muse_set_global_number(muse, "KEY_U", NEGLECT_KEY_U);
  muse_set_global_number(muse, "KEY_V", NEGLECT_KEY_V);
  muse_set_global_number(muse, "KEY_W", NEGLECT_KEY_W);
  muse_set_global_number(muse, "KEY_X", NEGLECT_KEY_X);
  muse_set_global_number(muse, "KEY_Y", NEGLECT_KEY_Y);
  muse_set_global_number(muse, "KEY_Z", NEGLECT_KEY_Z);
  muse_set_global_number(muse, "KEY_LEFT_BRACKET", NEGLECT_KEY_LEFT_BRACKET);
  muse_set_global_number(muse, "KEY_BACKSLASH", NEGLECT_KEY_BACKSLASH);
  muse_set_global_number(muse, "KEY_RIGHT_BRACKET", NEGLECT_KEY_RIGHT_BRACKET);
  muse_set_global_number(muse, "KEY_GRAVE_ACCENT", NEGLECT_KEY_GRAVE_ACCENT);
  muse_set_global_number(muse, "KEY_WORLD_1", NEGLECT_KEY_WORLD_1);
  muse_set_global_number(muse, "KEY_WORLD_2", NEGLECT_KEY_WORLD_2);

  muse_set_global_number(muse, "KEY_ESCAPE", NEGLECT_KEY_ESCAPE);
  muse_set_global_number(muse, "KEY_ENTER", NEGLECT_KEY_ENTER);
  muse_set_global_number(muse, "KEY_TAB", NEGLECT_KEY_TAB);
  muse_set_global_number(muse, "KEY_BACKSPACE", NEGLECT_KEY_BACKSPACE);
  muse_set_global_number(muse, "KEY_INSERT", NEGLECT_KEY_INSERT);
  muse_set_global_number(muse, "KEY_DELETE", NEGLECT_KEY_DELETE);
  muse_set_global_number(muse, "KEY_RIGHT", NEGLECT_KEY_RIGHT);
  muse_set_global_number(muse, "KEY_LEFT", NEGLECT_KEY_LEFT);
  muse_set_global_number(muse, "KEY_DOWN", NEGLECT_KEY_DOWN);
  muse_set_global_number(muse, "KEY_UP", NEGLECT_KEY_UP);
  muse_set_global_number(muse, "KEY_PAGE_UP", NEGLECT_KEY_PAGE_UP);
  muse_set_global_number(muse, "KEY_PAGE_DOWN", NEGLECT_KEY_PAGE_DOWN);
  muse_set_global_number(muse, "KEY_HOME", NEGLECT_KEY_HOME);
  muse_set_global_number(muse, "KEY_END", NEGLECT_KEY_END);
  muse_set_global_number(muse, "KEY_CAPS_LOCK", NEGLECT_KEY_CAPS_LOCK);
  muse_set_global_number(muse, "KEY_SCROLL_LOCK", NEGLECT_KEY_SCROLL_LOCK);
  muse_set_global_number(muse, "KEY_NUM_LOCK", NEGLECT_KEY_NUM_LOCK);
  muse_set_global_number(muse, "KEY_PRINT_SCREEN", NEGLECT_KEY_PRINT_SCREEN);
  muse_set_global_number(muse, "KEY_PAUSE", NEGLECT_KEY_PAUSE);
  muse_set_global_number(muse, "KEY_F1", NEGLECT_KEY_F1);
  muse_set_global_number(muse, "KEY_F2", NEGLECT_KEY_F2);
  muse_set_global_number(muse, "KEY_F3", NEGLECT_KEY_F3);
  muse_set_global_number(muse, "KEY_F4", NEGLECT_KEY_F4);
  muse_set_global_number(muse, "KEY_F5", NEGLECT_KEY_F5);
  muse_set_global_number(muse, "KEY_F6", NEGLECT_KEY_F6);
  muse_set_global_number(muse, "KEY_F7", NEGLECT_KEY_F7);
  muse_set_global_number(muse, "KEY_F8", NEGLECT_KEY_F8);
  muse_set_global_number(muse, "KEY_F9", NEGLECT_KEY_F9);
  muse_set_global_number(muse, "KEY_F10", NEGLECT_KEY_F10);
  muse_set_global_number(muse, "KEY_F11", NEGLECT_KEY_F11);
  muse_set_global_number(muse, "KEY_F12", NEGLECT_KEY_F12);
  muse_set_global_number(muse, "KEY_F13", NEGLECT_KEY_F13);
  muse_set_global_number(muse, "KEY_F14", NEGLECT_KEY_F14);
  muse_set_global_number(muse, "KEY_F15", NEGLECT_KEY_F15);
  muse_set_global_number(muse, "KEY_F16", NEGLECT_KEY_F16);
  muse_set_global_number(muse, "KEY_F17", NEGLECT_KEY_F17);
  muse_set_global_number(muse, "KEY_F18", NEGLECT_KEY_F18);
  muse_set_global_number(muse, "KEY_F19", NEGLECT_KEY_F19);
  muse_set_global_number(muse, "KEY_F20", NEGLECT_KEY_F20);
  muse_set_global_number(muse, "KEY_F21", NEGLECT_KEY_F21);
  muse_set_global_number(muse, "KEY_F22", NEGLECT_KEY_F22);
  muse_set_global_number(muse, "KEY_F23", NEGLECT_KEY_F23);
  muse_set_global_number(muse, "KEY_F24", NEGLECT_KEY_F24);
  muse_set_global_number(muse, "KEY_F25", NEGLECT_KEY_F25);
  muse_set_global_number(muse, "KEY_KP_0", NEGLECT_KEY_KP_0);
  muse_set_global_number(muse, "KEY_KP_1", NEGLECT_KEY_KP_1);
  muse_set_global_number(muse, "KEY_KP_2", NEGLECT_KEY_KP_2);
  muse_set_global_number(muse, "KEY_KP_3", NEGLECT_KEY_KP_3);
  muse_set_global_number(muse, "KEY_KP_4", NEGLECT_KEY_KP_4);
  muse_set_global_number(muse, "KEY_KP_5", NEGLECT_KEY_KP_5);
  muse_set_global_number(muse, "KEY_KP_6", NEGLECT_KEY_KP_6);
  muse_set_global_number(muse, "KEY_KP_7", NEGLECT_KEY_KP_7);
  muse_set_global_number(muse, "KEY_KP_8", NEGLECT_KEY_KP_8);
  muse_set_global_number(muse, "KEY_KP_9", NEGLECT_KEY_KP_9);
  muse_set_global_number(muse, "KEY_KP_DECIMAL", NEGLECT_KEY_KP_DECIMAL);
  muse_set_global_number(muse, "KEY_KP_DIVIDE", NEGLECT_KEY_KP_DIVIDE);
  muse_set_global_number(muse, "KEY_KP_MULTIPLY", NEGLECT_KEY_KP_MULTIPLY);
  muse_set_global_number(muse, "KEY_KP_SUBTRACT", NEGLECT_KEY_KP_SUBTRACT);
  muse_set_global_number(muse, "KEY_KP_ADD", NEGLECT_KEY_KP_ADD);
  muse_set_global_number(muse, "KEY_KP_ENTER", NEGLECT_KEY_KP_ENTER);
  muse_set_global_number(muse, "KEY_KP_EQUAL", NEGLECT_KEY_KP_EQUAL);
  muse_set_global_number(muse, "KEY_LEFT_SHIFT", NEGLECT_KEY_LEFT_SHIFT);
  muse_set_global_number(muse, "KEY_LEFT_CONTROL", NEGLECT_KEY_LEFT_CONTROL);
  muse_set_global_number(muse, "KEY_LEFT_ALT", NEGLECT_KEY_LEFT_ALT);
  muse_set_global_number(muse, "KEY_LEFT_SUPER", NEGLECT_KEY_LEFT_SUPER);
  muse_set_global_number(muse, "KEY_RIGHT_SHIFT", NEGLECT_KEY_RIGHT_SHIFT);
  muse_set_global_number(muse, "KEY_RIGHT_CONTROL", NEGLECT_KEY_RIGHT_CONTROL);
  muse_set_global_number(muse, "KEY_RIGHT_ALT", NEGLECT_KEY_RIGHT_ALT);
  muse_set_global_number(muse, "KEY_RIGHT_SUPER", NEGLECT_KEY_RIGHT_SUPER);
  muse_set_global_number(muse, "KEY_MENU", NEGLECT_KEY_MENU);

  muse_load_file(muse, "config.lua");
  muse_destroy(muse);

  return config;
}
