#include <stdio.h>
#include <stdint.h>

#include "glad/glad.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"

#include "occulus.h"
#include "bedrock.h"
#include "gossip.h"
#include "kronos.h"

int main() {
  if (!bedrock_init()) {
    printf("bedrock failed\n");
    return -1;
  }

  double last_tick = kronos_time();
  double current_second = 0;

  uint32_t frames = 0;
  glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
  while (!bedrock_should_close()) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double tick = kronos_time();
    double diff = tick - last_tick;
    last_tick = tick;

    current_second += diff;

    frames++;
    if (current_second >= 1) {
      current_second = 0;
      printf("FPS: %d\n", frames);
      frames = 0;
    }

    bedrock_swap();
    bedrock_poll();
  }

  bedrock_kill();
  gossip_cleanup();

#ifdef MEM_DEBUG
  occulus_print();
#endif

  return 0;
}
