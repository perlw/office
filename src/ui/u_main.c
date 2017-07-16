#include "bedrock/bedrock.h"

#include "ascii/ascii.h"

double timing = 1.0 / 30.0;
double since_update = 0.0;
void ui_update(double delta) {
  since_update += delta;
  while (since_update >= timing) {
    since_update -= timing;
    gossip_emit("window:update", &delta);
  }
}

void ui_draw(AsciiBuffer *const screen) {
  gossip_emit("window:draw", screen);
}
