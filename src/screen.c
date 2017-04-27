#include <assert.h>
#include <stdint.h>

#include "arkanis/math_3d.h"
#include "glad/glad.h"

#include "bedrock/bedrock.h"
#include "config.h"

// +AsciiLayer
typedef struct {
  uint8_t rune;
  uint8_t fore;
  uint8_t back;
} Glyph;

typedef struct {
  PicassoBufferGroup *quad;
  PicassoProgram *program;
  PicassoTexture *font_texture;

  uint32_t ascii_width;
  uint32_t ascii_height;
  uint32_t ascii_size;
  Glyph *asciimap;
  PicassoTexture *asciimap_texture;
} AsciiLayer;

AsciiLayer *asciilayer_create(uint32_t width, uint32_t height, uint32_t ascii_width, uint32_t ascii_height) {
  AsciiLayer *layer = calloc(1, sizeof(AsciiLayer));

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

  {
    uintmax_t vert_length = 0, frag_length = 0;
    uint8_t *vert_source, *frag_source;
    archivist_read_file("shaders/asciilayer.vert", &vert_length, &vert_source);
    archivist_read_file("shaders/asciilayer.frag", &frag_length, &frag_source);

    PicassoShader *vertex_shader = picasso_shader_create(PICASSO_SHADER_VERTEX);
    PicassoShader *fragment_shader = picasso_shader_create(PICASSO_SHADER_FRAGMENT);
    picasso_shader_compile(vertex_shader, vert_length, vert_source);
    picasso_shader_compile(fragment_shader, frag_length, frag_source);

    layer->program = picasso_program_create();
    picasso_program_link_shaders(layer->program, 2, (const PicassoShader *[]){
                                                      vertex_shader,
                                                      fragment_shader,
                                                    });

    picasso_shader_destroy(vertex_shader);
    picasso_shader_destroy(fragment_shader);

    free(vert_source);
    free(frag_source);
  }

  {
    int32_t vertex_attr = picasso_program_attrib_location(layer->program, "vertex");
    picasso_buffer_shader_attrib(vertex_buffer, vertex_attr);

    int32_t coord_attr = picasso_program_attrib_location(layer->program, "coord");
    picasso_buffer_shader_attrib(coord_buffer, coord_attr);

    mat4_t ortho = m4_ortho(0, (float)width, 0, (float)height, 1, 0);
    mat4_t model = m4_identity();

    int32_t pmatrix_uniform = picasso_program_uniform_location(layer->program, "pMatrix");
    int32_t mvmatrix_uniform = picasso_program_uniform_location(layer->program, "mvMatrix");
    picasso_program_uniform_mat4(layer->program, pmatrix_uniform, (float *)&ortho);
    picasso_program_uniform_mat4(layer->program, mvmatrix_uniform, (float *)&model);
  }

  {
    uintmax_t buffer_size = 0;
    uint8_t *buffer;

    archivist_read_file("fonts/cp437_8x8.png", &buffer_size, &buffer);
    layer->font_texture = picasso_texture_load(PICASSO_TEXTURE_TARGET_2D, PICASSO_TEXTURE_RGB, buffer_size, buffer);
    picasso_texture_bind_to(layer->font_texture, 0);

    int32_t texture_uniform = picasso_program_uniform_location(layer->program, "font_texture");
    picasso_program_uniform_int(layer->program, texture_uniform, 0);

    free(buffer);
  }

  {
    layer->ascii_width = ascii_width;
    layer->ascii_height = ascii_height;
    layer->ascii_size = layer->ascii_width * layer->ascii_height;
    layer->asciimap = calloc(layer->ascii_size, sizeof(Glyph));

    for (uintmax_t t = 0; t < layer->ascii_size; t++) {
      layer->asciimap[t].rune = 0;
      layer->asciimap[t].fore = 0;
      layer->asciimap[t].back = 0;
    }

    layer->asciimap_texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D, layer->ascii_width, layer->ascii_height, PICASSO_TEXTURE_RGB);
    picasso_texture_bind_to(layer->asciimap_texture, 1);
    picasso_texture_set_data(layer->asciimap_texture, 0, 0, layer->ascii_width, layer->ascii_height, layer->asciimap);

    int32_t texture_uniform = picasso_program_uniform_location(layer->program, "asciimap_texture");
    picasso_program_uniform_int(layer->program, texture_uniform, 1);

    int32_t ascii_width_uniform = picasso_program_uniform_location(layer->program, "ascii_res_width");
    int32_t ascii_height_uniform = picasso_program_uniform_location(layer->program, "ascii_res_height");
    picasso_program_uniform_int(layer->program, ascii_width_uniform, layer->ascii_width);
    picasso_program_uniform_int(layer->program, ascii_height_uniform, layer->ascii_height);
  }

  return layer;
}

void asciilayer_destroy(AsciiLayer *layer) {
  assert(layer);

  free(layer->asciimap);

  picasso_texture_destroy(layer->asciimap_texture);
  picasso_texture_destroy(layer->font_texture);
  picasso_program_destroy(layer->program);
  picasso_buffergroup_destroy(layer->quad);

  free(layer);
}

void asciilayer_draw(AsciiLayer *layer, bool dirty) {
  assert(layer);

  if (dirty) {
    picasso_texture_set_data(layer->asciimap_texture, 0, 0, layer->ascii_width, layer->ascii_height, layer->asciimap);
  }

  picasso_program_use(layer->program);
  picasso_buffergroup_draw(layer->quad, PICASSO_BUFFER_MODE_TRIANGLES, 6);
}
// -AsciiLayer

// +Surface
typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  uint32_t size;
  Glyph *asciimap;
} Surface;

typedef struct {
  AsciiLayer *asciilayer;
  Surface **surfaces;
} Screen;

Surface *surface_create(Screen *screen, uint32_t pos_x, uint32_t pos_y, uint32_t width, uint32_t height) {
  Surface *surface = calloc(1, sizeof(Surface));

  // TODO: Rethink, screen should control lifecycle?
  assert(screen);
  screen->surfaces = rectify_array_push(screen->surfaces, &surface);

  surface->x = pos_x;
  surface->y = pos_y;
  surface->width = width;
  surface->height = height;
  surface->size = surface->width * surface->height;
  surface->asciimap = calloc(surface->size, sizeof(Glyph));

  for (uintmax_t y = 0; y < surface->height; y++) {
    for (uintmax_t x = 0; x < surface->width; x++) {
      uintmax_t index = (y * surface->width) + x;
      surface->asciimap[index].rune = 1;
      surface->asciimap[index].fore = (uint8_t)((x ^ y) + 32);
    }
  }

  return surface;
}

void surface_destroy(Surface *surface) {
  assert(surface);

  free(surface->asciimap);

  free(surface);
}

void surface_text(Surface *surface, uint32_t x, uint32_t y, uintmax_t length, const char *string) {
  assert(surface);

  if (x >= surface->width || y >= surface->height) {
    return;
  }

  uint32_t index = (y * surface->width) + x;
  uint32_t max = index + (x + length >= surface->width ? surface->width - x : x + length);
  for (uint32_t t = index, u = 0; t < max; t++, u++) {
    if (string[u] == '\0') {
      break;
    }

    surface->asciimap[t].rune = string[u];
    surface->asciimap[t].fore = 255;
    surface->asciimap[t].back = 0;
  }
}
// -Surface

// +Screen
Screen *screen_create(const Config *config) {
  Screen *screen = calloc(1, sizeof(Screen));

  screen->asciilayer = asciilayer_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);
  screen->surfaces = rectify_array_alloc(10, sizeof(Surface *));

  return screen;
}

void screen_destroy(Screen *screen) {
  assert(screen);

  rectify_array_free(screen->surfaces);
  asciilayer_destroy(screen->asciilayer);

  free(screen);
}

void screen_draw(Screen *screen, bool dirty) {
  for (uintmax_t t = 0; t < rectify_array_size(screen->surfaces); t++) {
    Surface *surface = screen->surfaces[t];
    for (uintmax_t y = 0; y < surface->height; y++) {
      for (uintmax_t x = 0; x < surface->width; x++) {
        uintmax_t s_index = (y * surface->width) + x;
        uintmax_t index = ((y + surface->y) * screen->asciilayer->ascii_width) + (x + surface->x);
        screen->asciilayer->asciimap[index] = surface->asciimap[s_index];
      }
    }
  }

  asciilayer_draw(screen->asciilayer, dirty);
}
// -Screen
