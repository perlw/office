#include <stdio.h>
#include <stdint.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

// TODO: Errorchecks
uint8_t* read_file(const char* filepath, size_t* length) {
  FILE* file = fopen(filepath, "rb");
  fseek(file, 0, SEEK_END);
  *length = ftell(file);
  fseek(file, 0, SEEK_SET);
  uint8_t* buffer = malloc(*length);
  fread(buffer, *length, 1, file);
  fclose(file);
  return buffer;
}

int main() {
  if (!bedrock_init()) {
    printf("bedrock failed\n");
    return -1;
  }

  double last_tick = bedrock_kronos_time();
  double current_second = 0;

  BPicassoProgram* p_program = NULL;
  {
    size_t vert_length = 0, frag_length = 0;
    uint8_t* vert_source = read_file("shaders/dummy.vert", &vert_length);
    uint8_t* frag_source = read_file("shaders/dummy.frag", &frag_length);

    p_program = bedrock_picasso_program_create(vert_source, vert_length, frag_source, frag_length);

    free(vert_source);
    free(frag_source);
  }

  uint32_t frames = 0;
  glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
  while (!bedrock_should_close()) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double tick = bedrock_kronos_time();
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

  bedrock_picasso_program_destroy(p_program);

  bedrock_kill();

#ifdef MEM_DEBUG
  bedrock_occulus_print();
#endif

  return 0;
}
