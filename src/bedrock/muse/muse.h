#pragma once

#include <stdint.h>

typedef enum {
  MUSE_RESULT_OK = 1,
  MUSE_RESULT_MISSING_FUNC,
  MUSE_RESULT_LOAD_CALL_FAILED,
  MUSE_RESULT_OUT_OF_IDS,
} MuseResult;

typedef enum {
  MUSE_ARGUMENT_NUMBER = 1,
  MUSE_ARGUMENT_STRING,
  MUSE_ARGUMENT_BOOLEAN,
} MuseArgumentType;

typedef struct Muse Muse;

typedef struct {
  void *argument;
  MuseArgumentType type;
} MuseArgument;

typedef void (*MuseFunc)(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata);

typedef struct {
  char *name;
  MuseFunc func;
  uintmax_t num_arguments;
  MuseArgumentType *arguments;
  void *userdata;
} MuseFunctionDef;

Muse *muse_init(void);
Muse *muse_init_lite(void);
void muse_kill(Muse *muse);
MuseResult muse_call_simple(Muse *muse, const char *name);
MuseResult muse_load_file(Muse *muse, const char *filename);
MuseResult muse_add_module(Muse *muse, uintmax_t num_funcs, const MuseFunctionDef *funcs);
MuseResult muse_add_func(Muse *muse, const MuseFunctionDef *func);
MuseResult muse_set_global_number(Muse *muse, const char *name, double number);
