#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "../occulus/occulus.h"

#include "muse.h"

struct Muse {
  lua_State* state;
  uint8_t instance_id;
  MuseFunctionDef *func_defs[256];
};
