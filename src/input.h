#pragma once

#include "bedrock/bedrock.h"

typedef struct PicassoWindowInputBinding PicassoWindowInputBinding;
typedef void (*PicassoWindowInputCallback)(PicassoWindowInputBinding*, void*);

struct PicassoWindowInputBinding {
  char *action;
  // TODO: Combinations, modifiers, controllers
  int32_t key;
  PicassoWindowInputCallback callback;
  void *userdata;
};

typedef struct {
  char *action;
  MuseFunctionRef ref;
} ActionRef;

void input_init();
void input_kill();
void input_keyboard_callback(const PicassoWindowInputEvent *event);

void input_action(PicassoWindowInputBinding *binding, void *userdata);
void lua_action(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata);
void picasso_window_action_callback(PicassoWindowInputCallback callback, void *userdata);
void picasso_window_add_binding(PicassoWindowInputBinding *binding);
