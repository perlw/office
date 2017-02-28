#include "bedrock/muse/muse.h"

#include <stdio.h>

void test_func(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments) {
  printf("FUNC WORKS\n");
}

void test_arguments(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments) {
  printf("ARGUMENTS WORKS %f %s\n", *(double*)arguments[0].argument, (char*)arguments[1].argument);
}

void read_config(void) {
  MuseFunctionDef test_func_def = {
    .name = "test_func",
    .func = &test_func,
    .num_arguments = 0,
    .arguments = NULL,
  };
  MuseFunctionDef test_arg_def = {
    .name = "test_arguments",
    .func = &test_arguments,
    .num_arguments = 2,
    .arguments = (MuseArgumentType[2]){
      MUSE_ARGUMENT_NUMBER,
      MUSE_ARGUMENT_STRING,
    },
  };

  Muse *muse = muse_init_lite();
  muse_add_func(muse, &test_func_def);
  muse_add_func(muse, &test_arg_def);
  muse_load_file(muse, "config.lua");
  muse_kill(muse);
}
