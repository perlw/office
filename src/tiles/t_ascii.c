#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "arkanis/math_3d.h"
#include "glad/glad.h"

#include "bedrock/bedrock.h"

#include "assets.h"
#include "tiles.h"

TilesAscii *tiles_ascii_create(uint32_t width, uint32_t height, uint32_t ascii_width, uint32_t ascii_height) {
  TilesAscii *layer = calloc(1, sizeof(TilesAscii));

  layer->quad = picasso_buffergroup_create();

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

  PicassoBuffer *vertex_buffer = picasso_buffer_create(layer->quad, PICASSO_BUFFER_TYPE_ARRAY, PICASSO_BUFFER_USAGE_STATIC);
  picasso_buffer_set_data(vertex_buffer, 2, PICASSO_TYPE_INT, sizeof(*vertex_data) * 12, vertex_data);

  PicassoBuffer *coord_buffer = picasso_buffer_create(layer->quad, PICASSO_BUFFER_TYPE_ARRAY, PICASSO_BUFFER_USAGE_STATIC);
  picasso_buffer_set_data(coord_buffer, 2, PICASSO_TYPE_FLOAT, sizeof(*coord_data) * 12, coord_data);

  layer->program = (PicassoProgram *)tome_fetch(ASSET_SHADER, "tiles_ascii", "shaders/asciilayer");
  if (!layer->program) {
    printf("Something went wrong when fetching ascii shader :(\n");
    exit(-1);
  }

  {
    int32_t vertex_attr = picasso_program_attrib_location(layer->program, "vertex");
    picasso_buffer_shader_attrib(vertex_buffer, vertex_attr);

    int32_t coord_attr = picasso_program_attrib_location(layer->program, "coord");
    picasso_buffer_shader_attrib(coord_buffer, coord_attr);

    //mat4_t projection = m4_perspective(45, (float)width / (float)height, 1, 1000);
    //mat4_t model = m4_translation((vec3_t){ -320, -240, -640});
    mat4_t projection = m4_ortho(0, (float)width, 0, (float)height, 0, 1);
    mat4_t model = m4_identity();

    int32_t pmatrix_uniform = picasso_program_uniform_location(layer->program, "pMatrix");
    int32_t mvmatrix_uniform = picasso_program_uniform_location(layer->program, "mvMatrix");
    picasso_program_uniform_mat4(layer->program, pmatrix_uniform, (float *)&projection);
    picasso_program_uniform_mat4(layer->program, mvmatrix_uniform, (float *)&model);
  }

  {
    layer->font_texture = (PicassoTexture *)tome_fetch(ASSET_TEXTURE, "tiles_ascii_font", "fonts/cp437_8x8.png");
    if (!layer->font_texture) {
      printf("Something went wrong when fetching ascii font :(\n");
      exit(-1);
    }
    picasso_texture_bind_to(layer->font_texture, 0);

    int32_t texture_uniform = picasso_program_uniform_location(layer->program, "tiles_ascii_font");
    picasso_program_uniform_int(layer->program, texture_uniform, 0);
  }

  {
    layer->ascii_width = ascii_width;
    layer->ascii_height = ascii_height;
    layer->ascii_size = layer->ascii_width * layer->ascii_height;
    layer->asciimap = calloc(layer->ascii_size, sizeof(Glyph));
    layer->last_asciimap = calloc(layer->ascii_size, sizeof(Glyph));

    memset(layer->asciimap, 0, sizeof(Glyph) * layer->ascii_size);
    memset(layer->last_asciimap, 0, sizeof(Glyph) * layer->ascii_size);

    {
      layer->asciimap_texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D, layer->ascii_width, layer->ascii_height, PICASSO_TEXTURE_R);
      picasso_texture_bind_to(layer->asciimap_texture, 1);
      layer->forecolors_texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D, layer->ascii_width, layer->ascii_height, PICASSO_TEXTURE_RGB);
      picasso_texture_bind_to(layer->forecolors_texture, 2);
      layer->backcolors_texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D, layer->ascii_width, layer->ascii_height, PICASSO_TEXTURE_RGB);
      picasso_texture_bind_to(layer->backcolors_texture, 3);

      uint8_t *runes = calloc(layer->ascii_size, sizeof(uint8_t));
      GlyphColor *fore = calloc(layer->ascii_size, sizeof(GlyphColor));
      GlyphColor *back = calloc(layer->ascii_size, sizeof(GlyphColor));
      for (uintmax_t t = 0; t < layer->ascii_size; t++) {
        runes[t] = layer->asciimap[t].rune;
        fore[t] = layer->asciimap[t].fore;
        back[t] = layer->asciimap[t].back;
      }
      picasso_texture_set_data(layer->asciimap_texture, 0, 0, layer->ascii_width, layer->ascii_height, runes);
      picasso_texture_set_data(layer->forecolors_texture, 0, 0, layer->ascii_width, layer->ascii_height, fore);
      picasso_texture_set_data(layer->backcolors_texture, 0, 0, layer->ascii_width, layer->ascii_height, back);
      free(back);
      free(fore);
      free(runes);
    }

    {
      int32_t texture_uniform = picasso_program_uniform_location(layer->program, "asciimap_texture");
      picasso_program_uniform_int(layer->program, texture_uniform, 1);
    }
    {
      int32_t texture_uniform = picasso_program_uniform_location(layer->program, "forecolors_texture");
      picasso_program_uniform_int(layer->program, texture_uniform, 2);
    }
    {
      int32_t texture_uniform = picasso_program_uniform_location(layer->program, "backcolors_texture");
      picasso_program_uniform_int(layer->program, texture_uniform, 3);
    }

    int32_t ascii_width_uniform = picasso_program_uniform_location(layer->program, "ascii_res_width");
    int32_t ascii_height_uniform = picasso_program_uniform_location(layer->program, "ascii_res_height");
    picasso_program_uniform_int(layer->program, ascii_width_uniform, layer->ascii_width);
    picasso_program_uniform_int(layer->program, ascii_height_uniform, layer->ascii_height);
  }

  return layer;
}

void tiles_ascii_destroy(TilesAscii *layer) {
  assert(layer);

  free(layer->last_asciimap);
  free(layer->asciimap);

  picasso_texture_destroy(layer->backcolors_texture);
  picasso_texture_destroy(layer->forecolors_texture);
  picasso_texture_destroy(layer->asciimap_texture);

  tome_erase(ASSET_TEXTURE, "tiles_ascii_font");
  tome_erase(ASSET_SHADER, "tiles_ascii");

  picasso_buffergroup_destroy(layer->quad);

  free(layer);
}

void tiles_ascii_draw(TilesAscii *layer) {
  assert(layer);

  if (memcmp(layer->asciimap, layer->last_asciimap, sizeof(Glyph) * layer->ascii_size) != 0) {
    uint8_t *runes = calloc(layer->ascii_size, sizeof(uint8_t));
    GlyphColor *fore = calloc(layer->ascii_size, sizeof(GlyphColor));
    GlyphColor *back = calloc(layer->ascii_size, sizeof(GlyphColor));
    for (uintmax_t t = 0; t < layer->ascii_size; t++) {
      runes[t] = layer->asciimap[t].rune;
      fore[t] = layer->asciimap[t].fore;
      back[t] = layer->asciimap[t].back;
    }
    picasso_texture_set_data(layer->asciimap_texture, 0, 0, layer->ascii_width, layer->ascii_height, runes);
    picasso_texture_set_data(layer->forecolors_texture, 0, 0, layer->ascii_width, layer->ascii_height, fore);
    picasso_texture_set_data(layer->backcolors_texture, 0, 0, layer->ascii_width, layer->ascii_height, back);
    free(back);
    free(fore);
    free(runes);

    memcpy(layer->last_asciimap, layer->asciimap, sizeof(Glyph) * layer->ascii_size);
    Glyph *swp = layer->last_asciimap;
    layer->last_asciimap = layer->asciimap;
    layer->asciimap = swp;
  }

  picasso_program_use(layer->program);

  {
    mat4_t projection = m4_ortho(0, 640, 0, 480, 1, 0);
    mat4_t model = m4_identity();

    int32_t pmatrix_uniform = picasso_program_uniform_location(layer->program, "pMatrix");
    int32_t mvmatrix_uniform = picasso_program_uniform_location(layer->program, "mvMatrix");
    picasso_program_uniform_mat4(layer->program, pmatrix_uniform, (float *)&projection);
    picasso_program_uniform_mat4(layer->program, mvmatrix_uniform, (float *)&model);
    int32_t ascii_width_uniform = picasso_program_uniform_location(layer->program, "ascii_res_width");
    int32_t ascii_height_uniform = picasso_program_uniform_location(layer->program, "ascii_res_height");
    picasso_program_uniform_int(layer->program, ascii_width_uniform, layer->ascii_width);
    picasso_program_uniform_int(layer->program, ascii_height_uniform, layer->ascii_height);
  }

  picasso_texture_bind_to(layer->font_texture, 0);
  picasso_texture_bind_to(layer->asciimap_texture, 1);
  picasso_texture_bind_to(layer->forecolors_texture, 2);
  picasso_texture_bind_to(layer->backcolors_texture, 3);

  picasso_buffergroup_draw(layer->quad, PICASSO_BUFFER_MODE_TRIANGLES, 6);
}
