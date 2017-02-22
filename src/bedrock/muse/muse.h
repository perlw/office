#ifndef __MUSE_H__
#define __MUSE_H__

typedef enum {
  MUSE_RESULT_OK = 1,
  MUSE_RESULT_MISSING_FUNC,
  MUSE_RESULT_LOAD_CALL_FAILED,
} MuseResult;

#ifndef MUSE_INTERNAL
typedef void Muse;
#else
#include "m_types.h"
#endif

Muse *muse_init(void);
void muse_kill(Muse *muse);
MuseResult muse_call_simple(const Muse *muse, const char *name);
MuseResult muse_load_file(const Muse *muse, const char *filename);

#endif // __MUSE_H__
