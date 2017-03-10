#pragma once

#include <stdint.h>

typedef struct NeglectBinding NeglectBinding;
typedef void (*NeglectCallback)(NeglectBinding*);

struct NeglectBinding {
  uint32_t action;
  // TODO: Combinations, modifiers, controllers
  int32_t scancode;
  NeglectCallback callback;
};

void neglect_init(void);
void neglect_kill(void);
void neglect_action_callback(NeglectCallback callback);
void neglect_add_binding(NeglectBinding *binding);
