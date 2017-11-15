#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cJSON.h"

#define USE_ARCHIVIST
#define USE_PICASSO
#define USE_RECTIFY
#define USE_TOME
#include "bedrock/bedrock.h"

#define USE_ASCII
#define USE_ASSETS
#include "main.h"

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
    PicassoShaderDetailResult result = picasso_shader_compile(vertex_shader, vert_length, vert_source);
    if (result.result != PICASSO_SHADER_OK) {
      printf("PICASSO: Shader compile error!\n-=-\n%s\n-=-\n", result.detail);
      exit(-1);
    }
  }
  {
    PicassoShaderDetailResult result = picasso_shader_compile(fragment_shader, frag_length, frag_source);
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

void *tiledefs_loader(const char *name, const char *path) {
  size_t num_bytes = 0;
  uint8_t *data = NULL;
  if (!archivist_read_file(path, &num_bytes, &data)) {
    return NULL;
  }

  TileDef *tiledefs = rectify_array_alloc(10, sizeof(TileDef));

  cJSON *root = cJSON_Parse((const char *)data);
  for (int32_t t = 0; t < cJSON_GetArraySize(root); t++) {
    cJSON *item = cJSON_GetArrayItem(root, t);

    cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");
    cJSON *rune = cJSON_GetObjectItemCaseSensitive(item, "rune");
    cJSON *fore_color = cJSON_GetObjectItemCaseSensitive(item, "fore_color");
    cJSON *back_color = cJSON_GetObjectItemCaseSensitive(item, "back_color");
    cJSON *tags = cJSON_GetObjectItemCaseSensitive(item, "tags");

    TileDef def = {
      .id = rectify_memory_alloc_copy(id->valuestring, sizeof(char) * (strnlen(id->valuestring, 128) + 1)),
      .glyph = (Glyph){
        .rune = rune->valueint,
        .fore = glyphcolor_hex(fore_color->valueint),
        .back = glyphcolor_hex(back_color->valueint),
      },
      .collides = false,
    };
    for (int32_t u = 0; u < cJSON_GetArraySize(tags); u++) {
      cJSON *tag = cJSON_GetArrayItem(tags, u);
      if (strncmp(tag->valuestring, "wall", 128) == 0) {
        def.collides = true;
      }
    }

    printf("%s => { %d|%c, (%d %d %d) (%d %d %d) }\n", def.id, def.glyph.rune, def.glyph.rune, def.glyph.fore.r, def.glyph.fore.g, def.glyph.fore.b, def.glyph.back.r, def.glyph.back.g, def.glyph.back.b);

    tiledefs = rectify_array_push(tiledefs, &def);
  }
  cJSON_Delete(root);

  free(data);

  return tiledefs;
}

void tiledefs_destroyer(void *data) {
  TileDef *tiledefs = (TileDef *)data;
  for (uint32_t t = 0; t < rectify_array_size(tiledefs); t++) {
    free(tiledefs[t].id);
  }
  rectify_array_free(&data);
}

void setup_asset_loaders(void) {
  tome_handler(ASSET_SHADER, &shader_loader, &shader_destroyer);
  tome_handler(ASSET_TEXTURE, &texture_loader, &texture_destroyer);
  tome_handler(ASSET_TILEDEFS, &tiledefs_loader, &tiledefs_destroyer);
}
