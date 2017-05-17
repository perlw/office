#pragma once

#include "bedrock/bedrock.h"

typedef struct InputActionBinding InputActionBinding;
typedef void (*InputActionCallback)(InputActionBinding *, void *);

struct InputActionBinding {
  char *action;
  int32_t key;
};

typedef struct {
  char *action;
  MuseFunctionRef ref;
} ActionRef;

void input_init();
void input_kill();
void input_keyboard_callback(const PicassoWindowKeyboardEvent *event);
void input_mouse_callback(const PicassoWindowMouseEvent *event);

void input_action(InputActionBinding *binding, void *userdata);
void lua_action(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata);
void input_action_callback(InputActionCallback callback, void *userdata);
void input_action_add_binding(InputActionBinding *binding);
