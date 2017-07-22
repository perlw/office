#pragma once

#include "bedrock/bedrock.h"

typedef struct {
  char *action;
  int32_t key;
} InputActionBinding;

typedef struct {
  int32_t button;
  uint32_t x;
  uint32_t y;
  bool pressed;
  bool released;
} InputClickEvent;

typedef struct {
  uint32_t x;
  uint32_t y;
} InputMouseMoveEvent;

typedef void (*InputActionCallback)(InputActionBinding *, void *);

void input_init(void);
void input_kill(void);
void input_keyboard_callback(const PicassoWindowKeyboardEvent *event);
void input_mousemove_callback(const PicassoWindowMouseEvent *event);
void input_click_callback(const PicassoWindowMouseEvent *event);

void input_action_add_binding(InputActionBinding *binding);
