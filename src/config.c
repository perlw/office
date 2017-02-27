#include "bedrock/muse/muse.h"

#include <stdio.h>

void test_func(Muse *muse) {
  printf("WORKS\n");
}

void read_config(void) {
  MuseFunctionDef test_def= {
    .name = "test_func",
    .func = &test_func,
  };

  Muse *muse = muse_init_lite();
  muse_add_func(muse, &test_def);
  muse_load_file(muse, "config.lua");
  muse_kill(muse);
}
