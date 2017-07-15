#include <assert.h>

#define UI_INTERNAL
#include "ui.h"

UIDialogRuneSelector *ui_dialog_rune_selector_create(uint32_t x, uint32_t y) {
  UIDialogRuneSelector *dialog = calloc(1, sizeof(UIDialogRuneSelector));

  dialog->window = ui_window_create("C FontSel", x, y, 18, 18);
  dialog->rune_selector = ui_widget_rune_selector_create(dialog->window);

  return dialog;
}

void ui_dialog_rune_selector_destroy(UIDialogRuneSelector *const dialog) {
  assert(dialog);

  ui_widget_rune_selector_destroy(dialog->rune_selector);
  ui_window_destroy(dialog->window);

  free(dialog);
}
