#include <stdio.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

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


Muse *muse_init(void) {
  lua_State* muse = luaL_newstate();
  luaL_openlibs(muse);
  return muse;
}

void muse_kill(Muse *muse) {
  lua_close(muse);
}

// TODO: Error handling
void muse_call_simple(const Muse* muse, const char *name) {
  lua_getglobal(muse, name);
  int result = lua_pcall(muse, 0, 0, 0);
  if (result != LUA_OK) {
    const char* message = lua_tostring(muse, -1);
    printf("%s: %s\n", __FUNCTION__, message);
    lua_pop(muse, 1);
  }
}

