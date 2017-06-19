#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  MUSE_RESULT_OK = 1,
  MUSE_RESULT_MISSING_FUNC,
  MUSE_RESULT_LOAD_CALL_FAILED,
  MUSE_RESULT_OUT_OF_IDS,
  MUSE_RESULT_CALL_ALREADY_INIT,
  MUSE_RESULT_CALL_NO_INIT,
} MuseResult;

typedef enum {
  MUSE_TYPE_NUMBER = 1,
  MUSE_TYPE_STRING,
  MUSE_TYPE_BOOLEAN,
  MUSE_TYPE_FUNCTION,
} MuseType;

typedef struct Muse Muse;

typedef struct {
  void *argument;
  MuseType type;
} MuseArgument;

typedef void (*MuseFunc)(Muse *const muse, uintmax_t num_arguments, const MuseArgument *const arguments, const void *const userdata);

typedef struct {
  char *name;
  MuseFunc func;
  uintmax_t num_arguments;
  MuseType *arguments;
  void *userdata;
} MuseFunctionDef;

typedef uint32_t MuseFunctionRef;

Muse *muse_create(void);
Muse *muse_create_lite(void);
void muse_destroy(Muse *const muse);
MuseResult muse_call_simple(Muse *const muse, const char *name);
MuseResult muse_call_name(Muse *const muse, const char *name, uintmax_t num_arguments, const MuseArgument *const arguments, uintmax_t num_results, MuseArgument *const results);
MuseResult muse_call_funcref(Muse *const muse, MuseFunctionRef ref, uintmax_t num_arguments, const MuseArgument *const arguments, uintmax_t num_results, MuseArgument *const results);
MuseResult muse_load_file(Muse *const muse, const char *filename);
MuseResult muse_add_module(Muse *const muse, uintmax_t num_funcs, const MuseFunctionDef *const funcs);
MuseResult muse_add_func(Muse *const muse, const MuseFunctionDef *const func);
void muse_push_number(Muse *const muse, double number);
double muse_pop_number(Muse *const muse);
MuseResult muse_set_global_number(Muse *const muse, const char *name, double number);
