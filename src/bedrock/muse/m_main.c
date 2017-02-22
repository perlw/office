#include "m_internal.h"

/*
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

/*
   const int result = lua_pcall(lua_state, 0, LUA_MULTRET, 0);
   if (result != LUA_OK) {
   const char* message = lua_tostring(lua_state, -1);
   printf(message);
   lua_pop(lua_state, 1);
   return 0;
   }
   */

/*
 luaL_loadfile(lua_state, "test.lua");
 */

Muse *muse_init_lite(void) {
  Muse *muse = calloc(1, sizeof(Muse));

  *muse = (Muse){
    .state = luaL_newstate(),
  };

  return muse;
}

Muse *muse_init(void) {
  Muse *muse = muse_init_lite();
  luaL_openlibs(muse->state);
  return muse;
}

void muse_kill(Muse *muse) {
  assert(muse);

  lua_close(muse->state);
  free(muse);
}

// TODO: Error handling
MuseResult muse_call_simple(const Muse *muse, const char *name) {
  assert(muse);

  lua_getglobal(muse->state, name);
  int result = lua_pcall(muse->state, 0, 0, 0);
  if (result != LUA_OK) {
    const char* message = lua_tostring(muse->state, -1);
    printf("%s: %s\n", __FUNCTION__, message);
    lua_pop(muse->state, 1);
    return MUSE_RESULT_MISSING_FUNC;
  }

  return MUSE_RESULT_OK;
}

// TODO: Error handling
MuseResult muse_load_file(const Muse *muse, const char *filename) {
  assert(muse);

  luaL_loadfile(muse->state, filename);

  int result = lua_pcall(muse->state, 0, LUA_MULTRET, 0);
  if (result != LUA_OK) {
    const char* message = lua_tostring(muse->state, -1);
    printf("%s: %s\n", __FUNCTION__, message);
    lua_pop(muse->state, 1);
    return MUSE_RESULT_LOAD_CALL_FAILED;
  }

  return MUSE_RESULT_OK;
}
