#include <stdio.h>
#include <stdint.h>

#include "glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

Muse *muse;

int main() {
  muse = muse_init();
  muse_call_simple(muse, "foobar");

  if (!bedrock_init()) {
    printf("bedrock failed\n");
    return -1;
  }

  uint32_t vao;
  uint32_t vertex_buffer;
  uint32_t coord_buffer;

  {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    int32_t vertex_data[] = {
      0, 0,
      640, 480,
      0, 480,

      0, 0,
      640, 0,
      640, 480,
    };
    float coord_data[] = {
      0, 1,
      1, 0,
      0, 0,

      0, 1,
      1, 1,
      1, 0,
    };

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*vertex_data) * 12, vertex_data, GL_STATIC_DRAW);

    glGenBuffers(1, &coord_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, coord_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*coord_data) * 12, coord_data, GL_STATIC_DRAW);
  }

  PicassoProgram *p_program = NULL;
  {
    size_t vert_length = 0, frag_length = 0;
    uint8_t *vert_source, *frag_source;
    archivist_read_file("shaders/dummy.vert", &vert_source, &vert_length);
    archivist_read_file("shaders/dummy.frag", &frag_source, &frag_length);

    p_program = picasso_program_create(vert_source, vert_length, frag_source, frag_length);

    free(vert_source);
    free(frag_source);
  }
  picasso_program_use(p_program);
  {
    int32_t vertex_attr = picasso_program_attrib_location(p_program, "vertex");
    glEnableVertexAttribArray(vertex_attr);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(vertex_attr, 2, GL_INT, GL_FALSE, 0, NULL);

    int32_t coord_attr = picasso_program_attrib_location(p_program, "coord");
    glEnableVertexAttribArray(coord_attr);
    glBindBuffer(GL_ARRAY_BUFFER, coord_buffer);
    glVertexAttribPointer(coord_attr, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    mat4_t ortho = m4_ortho(0, 640, 0, 480, 1, 0);
    mat4_t model = m4_identity();

    int32_t pmatrix_uniform = picasso_program_uniform_location(p_program, "pMatrix");
    int32_t mvmatrix_uniform = picasso_program_uniform_location(p_program, "mvMatrix");
    picasso_program_mat4_set(p_program, pmatrix_uniform, (float*)&ortho);
    picasso_program_mat4_set(p_program, mvmatrix_uniform, (float*)&model);
  }

  double last_tick = bedrock_time();
  double current_second = 0;

  uint32_t frames = 0;
  glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
  while (!bedrock_should_close()) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double tick = bedrock_time();
    double diff = tick - last_tick;
    last_tick = tick;

    current_second += diff;

    frames++;
    if (current_second >= 1) {
      current_second = 0;
      printf("FPS: %d\n", frames);
      frames = 0;
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);

    bedrock_swap();
    bedrock_poll();
  }

  picasso_program_destroy(p_program);

  bedrock_kill();
  muse_kill(muse);

#ifdef MEM_DEBUG
  occulus_print();
#endif

  return 0;
}
