#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "glad/glad.h"

#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "config.h"

// +AsciiLayer
typedef struct {
  PicassoBufferGroup *quad;
  PicassoProgram *program;
  PicassoTexture *font_texture;

  uint8_t asciimap[80 * 60];
  PicassoTexture *asciimap_texture;
} AsciiLayer;

AsciiLayer *asciilayer_create(uint32_t res_width, uint32_t res_height) {
  AsciiLayer *layer = calloc(1, sizeof(AsciiLayer));

  layer->quad = picasso_buffergroup_create();

  int32_t vertex_data[] = {
    0, 0,
    res_width, res_height,
    0, res_height,

    0, 0,
    res_width, 0,
    res_width, res_height,
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
    archivist_read_file("shaders/dummy.vert", &vert_length, &vert_source);
    archivist_read_file("shaders/dummy.frag", &frag_length, &frag_source);

    PicassoShader *vertex_shader = picasso_shader_create(PICASSO_SHADER_VERTEX);
    PicassoShader *fragment_shader = picasso_shader_create(PICASSO_SHADER_FRAGMENT);
    picasso_shader_compile(vertex_shader, vert_length, vert_source);
    picasso_shader_compile(fragment_shader, frag_length, frag_source);

    layer->program = picasso_program_create();
    picasso_program_link_shaders(layer->program, 2, (const PicassoShader*[]){
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

    mat4_t ortho = m4_ortho(0, (float)res_width, 0, (float)res_height, 1, 0);
    mat4_t model = m4_identity();

    int32_t pmatrix_uniform = picasso_program_uniform_location(layer->program, "pMatrix");
    int32_t mvmatrix_uniform = picasso_program_uniform_location(layer->program, "mvMatrix");
    picasso_program_uniform_mat4(layer->program, pmatrix_uniform, (float*)&ortho);
    picasso_program_uniform_mat4(layer->program, mvmatrix_uniform, (float*)&model);
  }

  {
    uintmax_t buffer_size = 0;
    uint8_t *buffer;

    layer->font_texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D);

    archivist_read_file("fonts/cp437_8x8.png", &buffer_size, &buffer);
    picasso_texture_load(layer->font_texture, PICASSO_TEXTURE_RGB, buffer_size, buffer);
    picasso_texture_bind_to(layer->font_texture, 0);

    int32_t texture_uniform = picasso_program_uniform_location(layer->program, "font_texture");
    picasso_program_uniform_int(layer->program, texture_uniform, 0);

    free(buffer);
  }

  {
    /*for (uintmax_t t = 0; t < 80 * 60; t++) {
      layer->asciimap[t] = 1;
    }*/
    for (uintmax_t y = 0; y < 60; y++) {
      for (uintmax_t x = 0; x < 80; x++) {
        uintmax_t i = (y * 80) + x;
        layer->asciimap[i] = x ^ y;
      }
    }
    layer->asciimap_texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D);
    picasso_texture_bind_to(layer->asciimap_texture, 1);
    picasso_texture_set_data(layer->asciimap_texture, 80, 60, PICASSO_TEXTURE_R, layer->asciimap);

    int32_t texture_uniform = picasso_program_uniform_location(layer->program, "asciimap_texture");
    picasso_program_uniform_int(layer->program, texture_uniform, 1);
  }

  return layer;
}

void asciilayer_destroy(AsciiLayer *layer) {
  assert(layer);

  picasso_texture_destroy(layer->asciimap_texture);
  picasso_texture_destroy(layer->font_texture);
  picasso_program_destroy(layer->program);
  picasso_buffergroup_destroy(layer->quad);

  free(layer);
}

void asciilayer_draw(AsciiLayer *layer) {
  assert(layer);

  picasso_program_use(layer->program);
  picasso_buffergroup_draw(layer->quad, PICASSO_BUFFER_MODE_TRIANGLES, 6);
}
// -AsciiLayer

// +Screen
typedef struct {
  AsciiLayer *asciilayer;
} Screen;

Screen *screen_create(const Config *config) {
  Screen *screen = calloc(1, sizeof(Screen));

  screen->asciilayer = asciilayer_create(config->res_width, config->res_height);

  return screen;
}

void screen_kill(Screen *screen) {
  assert(screen);

  asciilayer_destroy(screen->asciilayer);

  free(screen);
}

void screen_update(Screen *screen, double delta) {
}

void screen_draw(Screen *screen) {
  asciilayer_draw(screen->asciilayer);
}
// -Screen

// +INPUT
typedef struct {
  char *action;
  MuseFunctionRef ref;
} ActionRef;
ActionRef *action_refs;

void input_init() {
  action_refs = rectify_array_alloc(10, sizeof(ActionRef));
}

void input_kill() {
  for (uintmax_t t = 0; t < rectify_array_size(action_refs); t++) {
    if (action_refs[t].action) {
      free(action_refs[t].action);
    }
  }
  rectify_array_free(action_refs);
}

void input_action(NeglectBinding *binding, void *userdata) {
  printf("->binding %s\n", binding->action);

  if (strcmp(binding->action, "close") == 0) {
    gossip_emit(GOSSIP_ID_CLOSE, NULL);
  }

  for (uintmax_t t = 0; t < rectify_array_size(action_refs); t++) {
    if (strcmp(action_refs[t].action, binding->action) == 0) {
      muse_call_func_ref((Muse*)userdata, action_refs[t].ref);
    }
  }
}
void lua_action(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata) {
  char *action = (char*)arguments[0].argument;
  MuseFunctionRef ref = *(MuseFunctionRef*)arguments[1].argument;

  ActionRef action_ref = {
    .action = calloc(strlen(action) + 1, sizeof(char)),
    .ref = ref,
  };
  strcpy(action_ref.action, action);
  action_refs = rectify_array_push(action_refs, &action_ref);
}
// -INPUT

int main() {
  Muse *muse = muse_init();

  neglect_init();
  neglect_action_callback(&input_action, muse);

  input_init();

  Config config = read_config();

  if (!bedrock_init("Office", config.res_width, config.res_height, config.gl_debug)) {
    printf("bedrock failed\n");
    return -1;
  }

  MuseFunctionDef action_def = {
    .name = "action",
    .func = &lua_action,
    .num_arguments = 2,
    .arguments = (MuseArgumentType[]){
      MUSE_ARGUMENT_STRING,
      MUSE_ARGUMENT_FUNCTION,
    },
    .userdata = NULL,
  };
  muse_add_func(muse, &action_def);
  muse_load_file(muse, "main.lua");

  Screen *screen = screen_create(&config);

  double last_tick = bedrock_time();
  double current_second = 0;

  uint32_t frames = 0;
  bedrock_clear_color(0.5f, 0.5f, 1.0f, 1.0f);
  while (!bedrock_should_close()) {
    bedrock_clear();

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

    muse_call(muse, "update", 1, (MuseArgument[]){
      {
        .type = MUSE_ARGUMENT_NUMBER,
        .argument = &current_second,
      },
    });

    screen_draw(screen);

    bedrock_swap();
    bedrock_poll();
  }

  screen_kill(screen);
  input_kill();

  muse_kill(muse);
  bedrock_kill();
  neglect_kill();

#ifdef MEM_DEBUG
  occulus_print(false);
#endif

  return 0;
}
