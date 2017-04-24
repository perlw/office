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

typedef void (*MuseFunc)(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata);

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
void muse_destroy(Muse *muse);
MuseResult muse_call_simple(Muse *muse, const char *name);
MuseResult muse_call_name(Muse *muse, const char *name, uintmax_t num_arguments, const MuseArgument *arguments, uintmax_t num_results, MuseArgument *results);
MuseResult muse_call_funcref(Muse *muse, MuseFunctionRef ref, uintmax_t num_arguments, const MuseArgument *arguments, uintmax_t num_results, MuseArgument *results);
MuseResult muse_load_file(Muse *muse, const char *filename);
MuseResult muse_add_module(Muse *muse, uintmax_t num_funcs, const MuseFunctionDef *funcs);
MuseResult muse_add_func(Muse *muse, const MuseFunctionDef *func);
void muse_push_number(Muse *muse, double number);
double muse_pop_number(Muse *muse);
MuseResult muse_set_global_number(Muse *muse, const char *name, double number);
