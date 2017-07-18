#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "glad/glad.h"

#include "bedrock/bedrock.h"

#include "ascii.h"
#include "assets.h"

AsciiBuffer *ascii_buffer_create(uint32_t width, uint32_t height, uint32_t ascii_width, uint32_t ascii_height) {
  AsciiBuffer *ascii = calloc(1, sizeof(AsciiBuffer));

  ascii->quad = picasso_buffergroup_create();

  int32_t vertex_data[] = {
    0, 0,
    width, height,
    0, height,

    0, 0,
    width, 0,
    width, height,
  };
  float coord_data[] = {
    0, 1,
    1, 0,
    0, 0,

    0, 1,
    1, 1,
    1, 0,
  };

  PicassoBuffer *vertex_buffer = picasso_buffer_create(ascii->quad, PICASSO_BUFFER_TYPE_ARRAY, PICASSO_BUFFER_USAGE_STATIC);
  picasso_buffer_set_data(vertex_buffer, 2, PICASSO_TYPE_INT, sizeof(*vertex_data) * 12, vertex_data);

  PicassoBuffer *coord_buffer = picasso_buffer_create(ascii->quad, PICASSO_BUFFER_TYPE_ARRAY, PICASSO_BUFFER_USAGE_STATIC);
  picasso_buffer_set_data(coord_buffer, 2, PICASSO_TYPE_FLOAT, sizeof(*coord_data) * 12, coord_data);

  ascii->program = (PicassoProgram *)tome_fetch(ASSET_SHADER, "ascii_buffer", "shaders/asciilayer");
  if (!ascii->program) {
    printf("Something went wrong when fetching ascii shader :(\n");
    exit(-1);
  }

  {
    int32_t vertex_attr = picasso_program_attrib_location(ascii->program, "vertex");
    picasso_buffer_shader_attrib(vertex_buffer, vertex_attr);

    int32_t coord_attr = picasso_program_attrib_location(ascii->program, "coord");
    picasso_buffer_shader_attrib(coord_buffer, coord_attr);

    //mat4_t projection = m4_perspective(45, (float)width / (float)height, 1, 1000);
    //mat4_t model = m4_translation((vec3_t){ -320, -240, -640});
    ascii->shader.projection_matrix = m4_ortho(0, (float)width, 0, (float)height, 1, 0);
    mat4_t model = m4_identity();

    ascii->shader.pmatrix_uniform = picasso_program_uniform_location(ascii->program, "pMatrix");
    int32_t mvmatrix_uniform = picasso_program_uniform_location(ascii->program, "mvMatrix");
    picasso_program_uniform_mat4(ascii->program, ascii->shader.pmatrix_uniform, (float *)&ascii->shader.projection_matrix);
    picasso_program_uniform_mat4(ascii->program, mvmatrix_uniform, (float *)&model);
  }

  {
    ascii->font_texture = (PicassoTexture *)tome_fetch(ASSET_TEXTURE, "ascii_buffer_font", "fonts/cp437_8x8.png");
    if (!ascii->font_texture) {
      printf("Something went wrong when fetching ascii font :(\n");
      exit(-1);
    }
    picasso_texture_bind_to(ascii->font_texture, 0);

    int32_t texture_uniform = picasso_program_uniform_location(ascii->program, "ascii_buffer_font");
    picasso_program_uniform_int(ascii->program, texture_uniform, 0);
  }

  {
    ascii->width = ascii_width;
    ascii->height = ascii_height;
    ascii->size = ascii->width * ascii->height;
    ascii->dirty = true;

    {
      ascii->asciimap_texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D, ascii->width, ascii->height, PICASSO_TEXTURE_R);
      picasso_texture_bind_to(ascii->asciimap_texture, 1);
      ascii->forecolors_texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D, ascii->width, ascii->height, PICASSO_TEXTURE_RGB);
      picasso_texture_bind_to(ascii->forecolors_texture, 2);
      ascii->backcolors_texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D, ascii->width, ascii->height, PICASSO_TEXTURE_RGB);
      picasso_texture_bind_to(ascii->backcolors_texture, 3);

      for (uint32_t t = 0; t < 2; t++) {
        ascii->buffers[t].rune_buffer = calloc(ascii->size, sizeof(uint8_t));
        ascii->buffers[t].fore_buffer = calloc(ascii->size, sizeof(GlyphColor));
        ascii->buffers[t].back_buffer = calloc(ascii->size, sizeof(GlyphColor));

        memset(ascii->buffers[t].rune_buffer, 0, sizeof(uint8_t) * ascii->size);
        memset(ascii->buffers[t].fore_buffer, 0, sizeof(GlyphColor) * ascii->size);
        memset(ascii->buffers[t].back_buffer, 0, sizeof(GlyphColor) * ascii->size);
      }

      ascii->front_buffer = &ascii->buffers[0];
      ascii->back_buffer = &ascii->buffers[1];
    }

    {
      int32_t texture_uniform = picasso_program_uniform_location(ascii->program, "asciimap_texture");
      picasso_program_uniform_int(ascii->program, texture_uniform, 1);
    }
    {
      int32_t texture_uniform = picasso_program_uniform_location(ascii->program, "forecolors_texture");
      picasso_program_uniform_int(ascii->program, texture_uniform, 2);
    }
    {
      int32_t texture_uniform = picasso_program_uniform_location(ascii->program, "backcolors_texture");
      picasso_program_uniform_int(ascii->program, texture_uniform, 3);
    }

    ascii->shader.ascii_width_uniform = picasso_program_uniform_location(ascii->program, "ascii_res_width");
    ascii->shader.ascii_height_uniform = picasso_program_uniform_location(ascii->program, "ascii_res_height");
    picasso_program_uniform_int(ascii->program, ascii->shader.ascii_width_uniform, ascii->width);
    picasso_program_uniform_int(ascii->program, ascii->shader.ascii_height_uniform, ascii->height);
  }

  return ascii;
}

void ascii_buffer_destroy(AsciiBuffer *const ascii) {
  assert(ascii);

  for (uint32_t t = 0; t < 2; t++) {
    free(ascii->buffers[t].back_buffer);
    free(ascii->buffers[t].fore_buffer);
    free(ascii->buffers[t].rune_buffer);
  }

  picasso_texture_destroy(ascii->backcolors_texture);
  picasso_texture_destroy(ascii->forecolors_texture);
  picasso_texture_destroy(ascii->asciimap_texture);

  tome_release(ASSET_TEXTURE, "ascii_buffer_font");
  tome_release(ASSET_SHADER, "ascii_buffer");

  picasso_buffergroup_destroy(ascii->quad);

  free(ascii);
}

void ascii_buffer_glyph(AsciiBuffer *const ascii, uint32_t x, uint32_t y, Glyph glyph) {
  assert(ascii);

  if (x > ascii->width - 1 || y > ascii->height - 1) {
    return;
  }

  uint32_t index = (y * ascii->width) + x;
  ascii->dirty = (ascii->dirty
                  || (ascii->back_buffer->rune_buffer[index] != glyph.rune
                       || !glyphcolor_eq(ascii->back_buffer->fore_buffer[index], glyph.fore)
                       || !glyphcolor_eq(ascii->back_buffer->back_buffer[index], glyph.back)));

  ascii->front_buffer->rune_buffer[index] = glyph.rune;
  ascii->front_buffer->fore_buffer[index] = glyph.fore;
  ascii->front_buffer->back_buffer[index] = glyph.back;
}

void ascii_buffer_draw(AsciiBuffer *const ascii) {
  assert(ascii);

  if (ascii->dirty) {
    ascii->dirty = false;

    picasso_texture_set_data(ascii->asciimap_texture, 0, 0, ascii->width, ascii->height, ascii->front_buffer->rune_buffer);
    picasso_texture_set_data(ascii->forecolors_texture, 0, 0, ascii->width, ascii->height, ascii->front_buffer->fore_buffer);
    picasso_texture_set_data(ascii->backcolors_texture, 0, 0, ascii->width, ascii->height, ascii->front_buffer->back_buffer);

    DisplayBuffer *swp = ascii->front_buffer;
    ascii->front_buffer = ascii->back_buffer;
    ascii->back_buffer = swp;

    memset(ascii->front_buffer->rune_buffer, 0, sizeof(uint8_t) * ascii->size);
    memset(ascii->front_buffer->fore_buffer, 0, sizeof(GlyphColor) * ascii->size);
    memset(ascii->front_buffer->back_buffer, 0, sizeof(GlyphColor) * ascii->size);
  }

  picasso_program_use(ascii->program);

  {
    picasso_program_uniform_mat4(ascii->program, ascii->shader.pmatrix_uniform, (float *)&ascii->shader.projection_matrix);
    picasso_program_uniform_int(ascii->program, ascii->shader.ascii_width_uniform, ascii->width);
    picasso_program_uniform_int(ascii->program, ascii->shader.ascii_height_uniform, ascii->height);
  }

  picasso_texture_bind_to(ascii->font_texture, 0);
  picasso_texture_bind_to(ascii->asciimap_texture, 1);
  picasso_texture_bind_to(ascii->forecolors_texture, 2);
  picasso_texture_bind_to(ascii->backcolors_texture, 3);

  picasso_buffergroup_draw(ascii->quad, PICASSO_BUFFER_MODE_TRIANGLES, 6);
}
