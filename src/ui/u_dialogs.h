#pragma once

#include "u_types.h"

// +UIDialogRuneSelector
UIDialogRuneSelector *ui_dialog_rune_selector_create(uint32_t x, uint32_t y);
void ui_dialog_rune_selector_destroy(UIDialogRuneSelector *const dialog);

void ui_dialog_rune_selector_update(UIDialogRuneSelector *const dialog, double delta);
void ui_dialog_rune_selector_draw(UIDialogRuneSelector *const dialog, AsciiBuffer *const ascii);
// -UIDialogRuneSelector
