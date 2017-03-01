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

Config read_config(void) {
  Config config = {
    .res_width = 640,
    .res_height = 480,
  };

  MuseFunctionDef set_resolution_def = {
    .name = "resolution",
    .func = &set_resolution,
    .num_arguments = 2,
    .arguments = (MuseArgumentType[2]){
      MUSE_ARGUMENT_NUMBER,
      MUSE_ARGUMENT_NUMBER,
    },
    .userdata = &config,
  };

  Muse *muse = muse_init_lite();
  muse_add_func(muse, &set_resolution_def);
  muse_load_file(muse, "config.lua");
  muse_kill(muse);

  return config;
}
