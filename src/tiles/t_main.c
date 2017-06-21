#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "arkanis/math_3d.h"
#include "glad/glad.h"

#include "bedrock/bedrock.h"

#include "assets.h"
#include "tiles.h"

Tiles *tiles_create(uint32_t width, uint32_t height, uint32_t num_tiles_x, uint32_t num_tiles_y) {
  Tiles *tiles = calloc(1, sizeof(Tiles));

  tiles->quad = picasso_buffergroup_create();

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

  PicassoBuffer *vertex_buffer = picasso_buffer_create(tiles->quad, PICASSO_BUFFER_TYPE_ARRAY, PICASSO_BUFFER_USAGE_STATIC);
  picasso_buffer_set_data(vertex_buffer, 2, PICASSO_TYPE_INT, sizeof(*vertex_data) * 12, vertex_data);

  PicassoBuffer *coord_buffer = picasso_buffer_create(tiles->quad, PICASSO_BUFFER_TYPE_ARRAY, PICASSO_BUFFER_USAGE_STATIC);
  picasso_buffer_set_data(coord_buffer, 2, PICASSO_TYPE_FLOAT, sizeof(*coord_data) * 12, coord_data);

  tiles->program = (PicassoProgram *)tome_fetch(ASSET_SHADER, "tiles", "shaders/tiles");
  if (!tiles->program) {
    printf("Something went wrong when fetching tiles shader :(\n");
    exit(-1);
  }

  {
    int32_t vertex_attr = picasso_program_attrib_location(tiles->program, "vertex");
    picasso_buffer_shader_attrib(vertex_buffer, vertex_attr);

    int32_t coord_attr = picasso_program_attrib_location(tiles->program, "coord");
    picasso_buffer_shader_attrib(coord_buffer, coord_attr);

    //mat4_t projection = m4_perspective(45, (float)width / (float)height, 1, 1000);
    //mat4_t model = m4_translation((vec3_t){ -320, -240, -640});
    tiles->shader.projection_matrix = m4_ortho(0, (float)width, 0, (float)height, 1, 0);
    mat4_t model = m4_identity();

    tiles->shader.pmatrix_uniform = picasso_program_uniform_location(tiles->program, "pMatrix");
    int32_t mvmatrix_uniform = picasso_program_uniform_location(tiles->program, "mvMatrix");
    picasso_program_uniform_mat4(tiles->program, tiles->shader.pmatrix_uniform, (float *)&tiles->shader.projection_matrix);
    picasso_program_uniform_mat4(tiles->program, mvmatrix_uniform, (float *)&model);
  }

  {
    tiles->tileset_texture = (PicassoTexture *)tome_fetch(ASSET_TEXTURE, "tileset", "tilesets/office.png");
    if (!tiles->tileset_texture) {
      printf("Something went wrong when fetching tileset :(\n");
      exit(-1);
    }
    picasso_texture_bind_to(tiles->tileset_texture, 0);

    int32_t texture_uniform = picasso_program_uniform_location(tiles->program, "tileset");
    picasso_program_uniform_int(tiles->program, texture_uniform, 0);
  }

  {
    TileSet *tileset = (TileSet *)tome_fetch(ASSET_TILESET, "office", "tilesets/office.lua");
    if (!tileset) {
      printf("Something went wrong when fetching tileset :(\n");
      exit(-1);
    }
  }

  {
    tiles->num_tiles_x = num_tiles_x;
    tiles->num_tiles_y = num_tiles_y;
    tiles->num_tiles = tiles->num_tiles_x * tiles->num_tiles_y;
    tiles->tilemap = calloc(tiles->num_tiles, sizeof(uint8_t));
    tiles->last_tilemap = calloc(tiles->num_tiles, sizeof(uint8_t));

    memset(tiles->tilemap, 0, sizeof(uint8_t) * tiles->num_tiles);
    memset(tiles->last_tilemap, 0, sizeof(uint8_t) * tiles->num_tiles);

    {
      tiles->tilemap_texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D, tiles->num_tiles_x, tiles->num_tiles_y, PICASSO_TEXTURE_R);
      picasso_texture_bind_to(tiles->tilemap_texture, 1);
      picasso_texture_set_data(tiles->tilemap_texture, 0, 0, tiles->num_tiles_x, tiles->num_tiles_y, tiles->tilemap);
    }

    {
      int32_t texture_uniform = picasso_program_uniform_location(tiles->program, "tilemap");
      picasso_program_uniform_int(tiles->program, texture_uniform, 1);
    }

    tiles->shader.num_tiles_x_uniform = picasso_program_uniform_location(tiles->program, "num_tiles_x");
    tiles->shader.num_tiles_y_uniform = picasso_program_uniform_location(tiles->program, "num_tiles_y");
    picasso_program_uniform_int(tiles->program, tiles->shader.num_tiles_x_uniform, tiles->num_tiles_x);
    picasso_program_uniform_int(tiles->program, tiles->shader.num_tiles_y_uniform, tiles->num_tiles_y);
  }

  return tiles;
}

void tiles_destroy(Tiles *tiles) {
  assert(tiles);

  free(tiles->last_tilemap);
  free(tiles->tilemap);

  picasso_texture_destroy(tiles->tilemap_texture);

  tome_release(ASSET_TEXTURE, "tileset");
  tome_release(ASSET_SHADER, "tiles");

  picasso_buffergroup_destroy(tiles->quad);

  free(tiles);
}

void tiles_draw(Tiles *tiles) {
  assert(tiles);

  if (memcmp(tiles->tilemap, tiles->last_tilemap, sizeof(uint8_t) * tiles->num_tiles) != 0) {
    picasso_texture_set_data(tiles->tilemap_texture, 0, 0, tiles->num_tiles_x, tiles->num_tiles_y, tiles->tilemap);

    memcpy(tiles->last_tilemap, tiles->tilemap, sizeof(uint8_t) * tiles->num_tiles);
    uint8_t *swp = tiles->last_tilemap;
    tiles->last_tilemap = tiles->tilemap;
    tiles->tilemap = swp;
  }

  picasso_program_use(tiles->program);

  {
    picasso_program_uniform_mat4(tiles->program, tiles->shader.pmatrix_uniform, (float *)&tiles->shader.projection_matrix);
    picasso_program_uniform_int(tiles->program, tiles->shader.num_tiles_x_uniform, tiles->num_tiles_x);
    picasso_program_uniform_int(tiles->program, tiles->shader.num_tiles_y_uniform, tiles->num_tiles_y);
  }

  picasso_texture_bind_to(tiles->tileset_texture, 0);
  picasso_texture_bind_to(tiles->tilemap_texture, 1);

  picasso_buffergroup_draw(tiles->quad, PICASSO_BUFFER_MODE_TRIANGLES, 6);
}
