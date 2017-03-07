#include "bedrock/muse/muse.h"

#include <stdio.h>

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

  Muse *muse = muse_init_lite();
  muse_add_func(muse, &set_resolution_def);
  muse_add_func(muse, &gl_debug_def);
  muse_load_file(muse, "config.lua");
  muse_kill(muse);

  return config;
}
