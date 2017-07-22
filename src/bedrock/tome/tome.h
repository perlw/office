/**
 * Tome - assetmanager
 */

#pragma once

typedef void *(*TomeLoader)(const char *name, const char *path);
typedef void (*TomeDestroyer)(void *const data);

void tome_init(void);
void tome_kill(void);

void *tome_fetch(int32_t type, const char *name, const char *path);
void tome_record(int32_t type, const char *name, const void *const data);
void tome_release(int32_t type, const char *name);

void tome_handler(int32_t type, TomeLoader loader, TomeDestroyer destroyer);
