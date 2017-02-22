#include "bedrock/muse/muse.h"

void read_config(void) {
  Muse *muse = muse_init_lite();
  muse_load_file(muse, "config.lua");
  muse_kill(muse);
}
