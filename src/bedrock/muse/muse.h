#ifndef __MUSE_H__
#define __MUSE_H__

#include <stdint.h>

typedef enum {
  MUSE_RESULT_OK = 1,
  MUSE_RESULT_MISSING_FUNC,
  MUSE_RESULT_LOAD_CALL_FAILED,
  MUSE_RESULT_OUT_OF_IDS,
} MuseResult;

typedef void (*MuseFunc)(void);

typedef struct {
  char *name;
  MuseFunc func;
} MuseFunctionDef;

typedef struct Muse Muse;

Muse *muse_init(void);
Muse *muse_init_lite(void);
void muse_kill(Muse *muse);
MuseResult muse_call_simple(Muse *muse, const char *name);
MuseResult muse_load_file(Muse *muse, const char *filename);
MuseResult muse_add_module(Muse *muse, uintmax_t num_funcs, const MuseFunctionDef *funcs);
MuseResult muse_add_func(Muse *muse, const MuseFunctionDef *func);

#endif // __MUSE_H__
