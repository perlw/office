#ifndef __MUSE_H__
#define __MUSE_H__

#ifndef MUSE_INTERNAL
typedef void Muse;
#endif

Muse *muse_init(void);
void muse_kill(Muse *muse);
void muse_call_simple(const Muse* muse, const char *name);

#endif // __MUSE_H__
