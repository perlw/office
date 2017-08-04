#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "assets.h"

void *shader_loader(const char *name, const char *path) {
  uintmax_t vert_length = 0, frag_length = 0;
  uint8_t *vert_source, *frag_source;

  uintmax_t filepath_length = strlen(path) + 6;
  char *vert_filepath = calloc(filepath_length, sizeof(char));
  char *frag_filepath = calloc(filepath_length, sizeof(char));
  snprintf(vert_filepath, filepath_length, "%s.vert", path);
  snprintf(frag_filepath, filepath_length, "%s.frag", path);

  archivist_read_file(vert_filepath, &vert_length, &vert_source);
  archivist_read_file(frag_filepath, &frag_length, &frag_source);

  free(vert_filepath);
  free(frag_filepath);

  PicassoShader *vertex_shader = picasso_shader_create(PICASSO_SHADER_VERTEX);
  PicassoShader *fragment_shader = picasso_shader_create(PICASSO_SHADER_FRAGMENT);
  {
    PicassoResult result = picasso_shader_compile(vertex_shader, vert_length, vert_source);
    if (result.result != PICASSO_SHADER_OK) {
      printf("PICASSO: Shader compile error!\n-=-\n%s\n-=-\n", result.detail);
      exit(-1);
    }
  }
  {
    PicassoResult result = picasso_shader_compile(fragment_shader, frag_length, frag_source);
    if (result.result != PICASSO_SHADER_OK) {
      printf("PICASSO: Shader compile error!\n-=-\n%s\n-=-\n", result.detail);
      exit(-1);
    }
  }

  PicassoProgram *program = picasso_program_create();
  picasso_program_link_shaders(program, 2, (const PicassoShader *[]){
                                             vertex_shader,
                                             fragment_shader,
                                           });

  picasso_shader_destroy(&vertex_shader);
  picasso_shader_destroy(&fragment_shader);

  free(vert_source);
  free(frag_source);

  return program;
}

void shader_destroyer(void *data) {
  PicassoProgram *program = (PicassoProgram *)data;
  picasso_program_destroy(&program);
}

void *texture_loader(const char *name, const char *path) {
  uintmax_t buffer_size = 0;
  uint8_t *buffer;

  if (!archivist_read_file(path, &buffer_size, &buffer)) {
    return NULL;
  }

  PicassoTexture *texture = picasso_texture_load(PICASSO_TEXTURE_TARGET_2D, PICASSO_TEXTURE_RGBA, buffer_size, buffer);
  free(buffer);

  return texture;
}

void texture_destroyer(void *data) {
  PicassoTexture *texture = (PicassoTexture *)data;
  picasso_texture_destroy(&texture);
}

void setup_asset_loaders(void) {
  tome_handler(ASSET_SHADER, &shader_loader, &shader_destroyer);
  tome_handler(ASSET_TEXTURE, &texture_loader, &texture_destroyer);
}
