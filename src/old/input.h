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

typedef struct {
  int32_t scroll_x;
  int32_t scroll_y;
  uint32_t x;
  uint32_t y;
} InputMouseScrollEvent;

typedef void (*InputActionCallback)(InputActionBinding *, void *);

void input_init(void);
void input_kill(void);
void input_keyboard_callback(const PicassoWindowKeyboardEvent *event);
void input_mousemove_callback(const PicassoWindowMouseEvent *event);
void input_click_callback(const PicassoWindowMouseEvent *event);
void input_mousescroll_callback(const PicassoWindowMouseScrollEvent *event);

void input_action_add_binding(InputActionBinding *binding);
