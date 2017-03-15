#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  MUSE_RESULT_OK = 1,
  MUSE_RESULT_MISSING_FUNC,
  MUSE_RESULT_LOAD_CALL_FAILED,
  MUSE_RESULT_OUT_OF_IDS,
  MUSE_RESULT_CALL_ALREADY_INIT,
  MUSE_RESULT_CALL_NO_INIT,
} MuseResult;

typedef enum {
  MUSE_ARGUMENT_NUMBER = 1,
  MUSE_ARGUMENT_STRING,
  MUSE_ARGUMENT_BOOLEAN,
  MUSE_ARGUMENT_FUNCTION,
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

typedef uint32_t MuseFunctionRef;

Muse *muse_init(void);
Muse *muse_init_lite(void);
void muse_kill(Muse *muse);
MuseResult muse_call_simple(Muse *muse, const char *name);
MuseResult muse_call(Muse *muse, const char *name, uintmax_t num_arguments, const MuseArgument *arguments);
MuseResult muse_call_func_ref(Muse *muse, MuseFunctionRef ref);
MuseResult muse_load_file(Muse *muse, const char *filename);
MuseResult muse_add_module(Muse *muse, uintmax_t num_funcs, const MuseFunctionDef *funcs);
MuseResult muse_add_func(Muse *muse, const MuseFunctionDef *func);
void muse_push_number(Muse *muse, double number);
double muse_pop_number(Muse *muse);
MuseResult muse_set_global_number(Muse *muse, const char *name, double number);
