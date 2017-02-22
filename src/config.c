#include "bedrock/muse/muse.h"

void read_config(void) {
  // FIXME: Loads all of lua at the moment, not necessary for config.
  Muse *muse = muse_init();
  muse_load_file(muse, "config.lua");
  muse_kill(muse);
}
