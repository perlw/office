#pragma once

#include "bedrock/bedrock.h"

typedef struct {
  char *action;
  int32_t key;
} InputActionBinding;

typedef struct {
  char *action;
  int32_t ref;
} InputActionRef;

typedef void (*InputActionCallback)(InputActionBinding *, void *);

void input_init();
void input_kill();
void input_keyboard_callback(const PicassoWindowKeyboardEvent *event);
void input_mousemove_callback(const PicassoWindowMouseEvent *event);
void input_click_callback(const PicassoWindowMouseEvent *event);

void input_action_add_binding(InputActionBinding *binding);
void input_action_add_action(InputActionRef *action_ref);
