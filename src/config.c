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

void test_action(NeglectBinding *binding) {
  printf("TEST FROM LUA\n");
}
void key_bind(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata) {
  char *action = (char*)arguments[0].argument;
  int32_t scancode = (int32_t)*(double*)arguments[1].argument;

  NeglectBinding binding = (NeglectBinding){
    .action = action,
    .scancode = scancode,
    .callback = NULL,
  };

  if (strcmp(action, "test") == 0) {
    binding.callback = &test_action;
  }

  neglect_add_binding(&binding);
}

Config read_config(void) {
  Config config = {
    .res_width = 640,
    .res_height = 480,
    .gl_debug = false,
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

  Muse *muse = muse_init_lite();

  muse_add_func(muse, &set_resolution_def);
  muse_add_func(muse, &gl_debug_def);
  muse_add_func(muse, &bind_def);

  // TODO: Cleanup, break out, etc
  muse_set_global_number(muse, "KEY_ESCAPE", 9);
  muse_set_global_number(muse, "KEY_SPACE", 65);

  muse_load_file(muse, "config.lua");
  muse_kill(muse);

  return config;
}
