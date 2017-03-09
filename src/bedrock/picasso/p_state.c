#include "p_internal.h"

uint32_t context[PICASSO_STATE_END - 1] = { 0 };

uint32_t get_state(PicassoState state) {
  if (state < 1 || state >= PICASSO_STATE_END) {
    return 0;
  }

  return context[state - 1];
}

void set_state(PicassoState state, uint32_t value) {
  if (state < 1 || state >= PICASSO_STATE_END) {
    return;
  }

  context[state - 1] = value;
}
