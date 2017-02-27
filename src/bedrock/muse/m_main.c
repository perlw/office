#include "m_internal.h"

/*
 *lua_pushcfunction(lua_state, l_set_message);
 +  lua_setglobal(lua_state, "set_message");
   const luaL_Reg test_lib[] = {
   { "set_message", l_set_message },
   { "set_ascii", l_set_ascii },
   { "switch_font", l_switch_font },
   { "add_keyboard_callback", l_add_keyboard_callback },
   { "quit", l_quit },
   { NULL, NULL }
   };
   lua_newtable(lua_state);
   luaL_setfuncs(lua_state, test_lib, 0);
   lua_setglobal(lua_state, "testlib");
   */
Muse *instances[256] = { NULL };

Muse *muse_init_lite(void) {
  for (int t = 0; t < 256; t++) {
    if (!instances[t]) {
      Muse *muse = calloc(1, sizeof(Muse));

      *muse = (Muse){
        .state = luaL_newstate(),
        .instance_id = t,
        .func_defs = { NULL },
      };

      instances[t] = muse;

      return muse;
    }
  }

  return NULL;
}

Muse *muse_init(void) {
  Muse *muse = muse_init_lite();
  if (!muse) {
    return NULL;
  }

  luaL_openlibs(muse->state);
  return muse;
}

void muse_kill(Muse *muse) {
  assert(muse);

  for (int t = 0; t < 256; t++) {
    if (instances[t] == muse) {
      instances[t] = NULL;
      break;
    }
  }

  for (int t = 0; t < 256; t++) {
    if (muse->func_defs[t]) {
      free(muse->func_defs[t]->name);
      free(muse->func_defs[t]);
      muse->func_defs[t] = NULL;
    }
  }

  lua_close(muse->state);
  free(muse);
}

MuseResult muse_call_simple(Muse *muse, const char *name) {
  assert(muse);

  lua_getglobal(muse->state, name);
  int result = lua_pcall(muse->state, 0, 0, 0);
  if (result != LUA_OK) {
    const char *message = lua_tostring(muse->state, -1);
    printf("%s: %s\n", __FUNCTION__, message);
    lua_pop(muse->state, 1);
    return MUSE_RESULT_MISSING_FUNC;
  }

  return MUSE_RESULT_OK;
}

MuseResult muse_load_file(Muse *muse, const char *filename) {
  assert(muse);

  luaL_loadfile(muse->state, filename);

  int result = lua_pcall(muse->state, 0, LUA_MULTRET, 0);
  if (result != LUA_OK) {
    const char *message = lua_tostring(muse->state, -1);
    printf("%s: %s\n", __FUNCTION__, message);
    lua_pop(muse->state, 1);
    return MUSE_RESULT_LOAD_CALL_FAILED;
  }

  return MUSE_RESULT_OK;
}

MuseResult muse_add_module(Muse *muse, uintmax_t num_funcs, const MuseFunctionDef *func_defs) {
  assert(muse);
  assert(num_funcs > 0);
  assert(func_defs);

  return MUSE_RESULT_OK;
}

static int lua_callback(lua_State *state) {
  uint8_t instance_id = lua_tonumber(state, lua_upvalueindex(1));
  uint8_t func_id = lua_tonumber(state, lua_upvalueindex(2));

  printf("instance=%d, func=%d\n", instance_id, func_id);

  assert(instances[instance_id]);
  assert(instances[instance_id]->func_defs[func_id]);

  instances[instance_id]->func_defs[func_id]->func(instances[instance_id]);

  return 0;
}

MuseResult muse_add_func(Muse *muse, const MuseFunctionDef *func_def) {
  assert(muse);
  assert(func_def);

  for (int t = 0; t < 256; t++) {
    if (!muse->func_defs[t]) {
      muse->func_defs[t] = calloc(1, sizeof(MuseFunctionDef));
      muse->func_defs[t]->name = calloc(strlen(func_def->name), sizeof(char));
      strcpy(muse->func_defs[t]->name, func_def->name);
      muse->func_defs[t]->func = func_def->func;

      lua_pushnumber(muse->state, muse->instance_id);
      lua_pushnumber(muse->state, t);
      lua_pushcclosure(muse->state, &lua_callback, 2);
      lua_setglobal(muse->state, muse->func_defs[t]->name);

      return MUSE_RESULT_OK;
    }
  }

  return MUSE_RESULT_OUT_OF_IDS;
}
