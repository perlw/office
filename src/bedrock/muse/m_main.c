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
      if (muse->func_defs[t]->num_arguments > 0) {
        free(muse->func_defs[t]->arguments);
      }
      free(muse->func_defs[t]);
      muse->func_defs[t] = NULL;
    }
  }

  lua_close(muse->state);
  free(muse);
}

MuseResult muse_call_simple(Muse *restrict muse, const char *name) {
  assert(muse);

  lua_getglobal(muse->state, name);
  int result = lua_pcall(muse->state, 0, 0, 0);
  if (result != LUA_OK) {
    const char *message = lua_tostring(muse->state, -1);
    printf("MUSE: %s: %s\n", __func__, message);
    lua_pop(muse->state, 1);
    return MUSE_RESULT_MISSING_FUNC;
  }

  return MUSE_RESULT_OK;
}

// TODO: Read results
MuseResult muse_call(Muse *restrict muse, const char *name, uintmax_t num_arguments, const MuseArgument *arguments) {
  assert(muse);

  lua_getglobal(muse->state, name);
  for (uintmax_t t = 0; t < num_arguments; t++) {
    switch (arguments[t].type) {
      case MUSE_ARGUMENT_NUMBER:
        lua_pushnumber(muse->state, *(double*)arguments[t].argument);
        break;

      case MUSE_ARGUMENT_BOOLEAN:
        lua_pushboolean(muse->state, *(bool*)arguments[t].argument);
        break;

      case MUSE_ARGUMENT_STRING:
      default:
        printf("MUSE (%s:%d): unknown/unimplemented type %d\n", __FILE__, __LINE__, arguments[t].type);
        break;
    }
  }

  int result = lua_pcall(muse->state, num_arguments, 0, 0);
  if (result != LUA_OK) {
    const char *message = lua_tostring(muse->state, -1);
    printf("MUSE: %s: %s\n", __func__, message);
    lua_pop(muse->state, 1);
    return MUSE_RESULT_MISSING_FUNC;
  }

  return MUSE_RESULT_OK;
}

// TODO: arguments/results
MuseResult muse_call_func_ref(Muse *restrict muse, MuseFunctionRef ref) {
  assert(muse);

  lua_rawgeti(muse->state, LUA_REGISTRYINDEX, ref);
  lua_pcall(muse->state, 0, 0, 0);

  return MUSE_RESULT_OK;
}

MuseResult muse_load_file(Muse *restrict muse, const char *filename) {
  assert(muse);

  luaL_loadfile(muse->state, filename);

  int result = lua_pcall(muse->state, 0, LUA_MULTRET, 0);
  if (result != LUA_OK) {
    const char *message = lua_tostring(muse->state, -1);
    printf("MUSE: %s: %s\n", __func__, message);
    lua_pop(muse->state, 1);
    return MUSE_RESULT_LOAD_CALL_FAILED;
  }

  return MUSE_RESULT_OK;
}

MuseResult muse_add_module(Muse *restrict muse, uintmax_t num_funcs, const MuseFunctionDef *func_defs) {
  assert(muse);
  assert(num_funcs > 0);
  assert(func_defs);

  return MUSE_RESULT_OK;
}

// FIXME: Finish error checks
// TODO: Lists, functions?
static int lua_callback(lua_State *state) {
  uint8_t instance_id = lua_tonumber(state, lua_upvalueindex(1));
  uint8_t func_id = lua_tonumber(state, lua_upvalueindex(2));

  printf("MUSE: instance=%d, func=%d\n", instance_id, func_id);

  assert(instances[instance_id]);
  assert(instances[instance_id]->func_defs[func_id]);

  uintmax_t num_arguments = 0;
  MuseArgument *arguments = NULL;
  Muse *muse = instances[instance_id];
  MuseFunctionDef *func_def = muse->func_defs[func_id];
  if (func_def->num_arguments > 0) {
    if (lua_gettop(muse->state) != func_def->num_arguments) {
      printf("MUSE (%s:%d): incorrect amount of args\n", __FILE__, __LINE__);
      return 0;
    }

    num_arguments = func_def->num_arguments;
    arguments = calloc(num_arguments, sizeof(MuseArgument));
    for (uintmax_t t = 0; t < func_def->num_arguments; t++) {
      switch (func_def->arguments[t]) {
        case MUSE_ARGUMENT_NUMBER:
          if (!lua_isnumber(muse->state, t + 1)) {
            printf("MUSE (%s:%d): incorrect arg type, expected number!\n", __FILE__, __LINE__);
          }

          arguments[t] = (MuseArgument){
            .argument = calloc(1, sizeof(double)),
            .type = func_def->arguments[t],
          };
          *(double*)arguments[t].argument = (double)lua_tonumber(muse->state, t + 1);
          break;

        case MUSE_ARGUMENT_STRING:
          if (!lua_isstring(muse->state, t + 1)) {
            printf("MUSE (%s:%d): incorrect arg type, expected string!\n", __FILE__, __LINE__);
          }

          uintmax_t length = 0;
          const char *string = lua_tolstring(muse->state, t + 1, &length);
          arguments[t] = (MuseArgument){
            .argument = calloc(length + 1, sizeof(char)),
            .type = func_def->arguments[t],
          };
          memcpy(arguments[t].argument, string, length * sizeof(char));
          break;

        case MUSE_ARGUMENT_BOOLEAN:
          if (!lua_isboolean(muse->state, t + 1)) {
            printf("MUSE (%s:%d): incorrect arg type, expected boolean!\n", __FILE__, __LINE__);
          }

          arguments[t] = (MuseArgument){
            .argument = calloc(1, sizeof(double)),
            .type = func_def->arguments[t],
          };
          *(bool*)arguments[t].argument = (bool)lua_toboolean(muse->state, t + 1);
          break;

        case MUSE_ARGUMENT_FUNCTION:
          luaL_checktype(muse->state, t + 1, LUA_TFUNCTION);

          arguments[t] = (MuseArgument){
            .argument = calloc(1, sizeof(MuseFunctionRef)),
            .type = func_def->arguments[t],
          };
          *(MuseFunctionRef*)arguments[t].argument = (MuseFunctionRef)luaL_ref(muse->state, LUA_REGISTRYINDEX);

          break;

        default:
          printf("MUSE (%s:%d): unknown/unimplemented type %d\n", __FILE__, __LINE__, func_def->arguments[t]);
          break;
      }
    }
  }

  func_def->func(instances[instance_id], num_arguments, arguments, func_def->userdata);

  if (num_arguments > 0) {
    for (uintmax_t t = 0; t < num_arguments; t++) {
      free(arguments[t].argument);
    }
    free(arguments);
  }

  return 0;
}

MuseResult muse_add_func(Muse *restrict muse, const MuseFunctionDef *func_def) {
  assert(muse);
  assert(func_def);

  for (int t = 0; t < 256; t++) {
    if (!muse->func_defs[t]) {
      muse->func_defs[t] = calloc(1, sizeof(MuseFunctionDef));
      uintmax_t size = strlen(func_def->name) + 1;
      muse->func_defs[t]->name = calloc(size, sizeof(char));
      memcpy(muse->func_defs[t]->name, func_def->name, size);
      muse->func_defs[t]->func = func_def->func;
      muse->func_defs[t]->userdata = func_def->userdata;
      if (func_def->num_arguments > 0) {
        muse->func_defs[t]->num_arguments = func_def->num_arguments;
        muse->func_defs[t]->arguments = calloc(func_def->num_arguments, sizeof(MuseArgumentType));
        memcpy(muse->func_defs[t]->arguments, func_def->arguments, func_def->num_arguments * sizeof(MuseArgumentType));
      } else {
        muse->func_defs[t]->num_arguments = 0;
        muse->func_defs[t]->arguments = NULL;
      }

      lua_pushnumber(muse->state, muse->instance_id);
      lua_pushnumber(muse->state, t);
      lua_pushcclosure(muse->state, &lua_callback, 2);
      lua_setglobal(muse->state, muse->func_defs[t]->name);

      return MUSE_RESULT_OK;
    }
  }

  return MUSE_RESULT_OUT_OF_IDS;
}

void muse_push_number(Muse *restrict muse, double number) {
  assert(muse);
  lua_pushnumber(muse->state, number);
}

double muse_pop_number(Muse *restrict muse) {
  assert(muse);

  if (!lua_isnumber(muse->state, 1)) {
    printf("MUSE (%s:%d): incorrect arg type, expected number!\n", __FILE__, __LINE__);
  }

  return lua_tonumber(muse->state, 1);
}

MuseResult muse_set_global_number(Muse *restrict muse, const char *name, double number) {
  assert(muse);

  lua_pushnumber(muse->state, number);
  lua_setglobal(muse->state, name);

  return MUSE_RESULT_OK;
}
