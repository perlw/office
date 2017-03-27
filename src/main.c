#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "glad/glad.h"

#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"

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

  double offset;
  uint32_t ascii_width;
  uint32_t ascii_height;
  uint32_t ascii_size;
  Glyph *asciimap;
  PicassoTexture *asciimap_texture;
} AsciiLayer;

AsciiLayer *asciilayer_create(const Config *config) {
  AsciiLayer *layer = calloc(1, sizeof(AsciiLayer));

  layer->quad = picasso_buffergroup_create();

  int32_t vertex_data[] = {
    0, 0,
    config->res_width, config->res_height,
    0, config->res_height,

    0, 0,
    config->res_width, 0,
    config->res_width, config->res_height,
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

    mat4_t ortho = m4_ortho(0, (float)config->res_width, 0, (float)config->res_height, 1, 0);
    mat4_t model = m4_identity();

    int32_t pmatrix_uniform = picasso_program_uniform_location(layer->program, "pMatrix");
    int32_t mvmatrix_uniform = picasso_program_uniform_location(layer->program, "mvMatrix");
    picasso_program_uniform_mat4(layer->program, pmatrix_uniform, (float*)&ortho);
    picasso_program_uniform_mat4(layer->program, mvmatrix_uniform, (float*)&model);
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
    layer->ascii_width = config->ascii_width;
    layer->ascii_height = config->ascii_height;
    layer->ascii_size = layer->ascii_width * layer->ascii_height;
    layer->asciimap = calloc(layer->ascii_size, sizeof(Glyph));

    layer->offset = 0;
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

void asciilayer_tick(AsciiLayer *layer) {
  layer->offset += 0.1;

  double wave_depth = 0.25;
  double wave_thickness = M_PI * 4.0;
  double cx = layer->ascii_width / 2;
  double cy = layer->ascii_height / 2;
  for (uintmax_t y = 0; y < layer->ascii_height; y++) {
    for (uintmax_t x = 0; x < layer->ascii_width; x++) {
      double dx = abs((double)x - cx);
      double dy = abs((double)y - cy);
      double dist = sqrt(pow(dx, 2) + pow(dy, 2));
      double ndist = dist / 50.0;

      double final_color = ((cos((ndist * wave_thickness) + layer->offset) + 1.0) / 4.0) + wave_depth;

      uintmax_t i = (y * layer->ascii_width) + x;
      uint8_t color = (uintmax_t)(final_color * 255.0);
      if (color < 96) {
        layer->asciimap[i].rune = '.';
      } else if (color < 178) {
        layer->asciimap[i].rune = '+';
      } else {
        layer->asciimap[i].rune = '*';
      }
      layer->asciimap[i].fore = color;
      layer->asciimap[i].back = 0;
    }
  }

  picasso_texture_set_data(layer->asciimap_texture, 0, 0, layer->ascii_width, layer->ascii_height, layer->asciimap);
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
  double timing;
  double since_update;
} Screen;

Screen *screen_create(const Config *config) {
  Screen *screen = calloc(1, sizeof(Screen));

  screen->asciilayer = asciilayer_create(config);
  screen->timing = 1 / 30.0;
  screen->since_update = screen->timing;

  return screen;
}

void screen_destroy(Screen *screen) {
  assert(screen);

  asciilayer_destroy(screen->asciilayer);

  free(screen);
}

void screen_update(Screen *screen, double delta) {
  screen->since_update += delta;
  while (screen->since_update >= screen->timing) {
    screen->since_update -= screen->timing;

    asciilayer_tick(screen->asciilayer);
  }
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

void input_action(PicassoWindowInputBinding *binding, void *userdata) {
  printf("->binding %s\n", binding->action);

  if (strcmp(binding->action, "close") == 0) {
    gossip_emit(GOSSIP_ID_CLOSE, NULL);
  }

  for (uintmax_t t = 0; t < rectify_array_size(action_refs); t++) {
    if (strcmp(action_refs[t].action, binding->action) == 0) {
      muse_call_funcref((Muse*)userdata, action_refs[t].ref, 0, NULL, 0, NULL);
    }
  }
}
void lua_action(Muse *muse, uintmax_t num_arguments, const MuseArgument *arguments, void *userdata) {
  char *action = (char*)arguments[0].argument;
  MuseFunctionRef ref = *(MuseFunctionRef*)arguments[1].argument;

  ActionRef action_ref = {
    .action = rectify_memory_alloc_copy(action, strlen(action) + 1),
    .ref = ref,
  };
  action_refs = rectify_array_push(action_refs, &action_ref);
}
// -INPUT

int main() {
  Boombox *boombox = boombox_create();
  if (boombox_init(boombox) != BOOMBOX_OK) {
    printf("Boombox: failed to init\n");
    return -1;
  }
  BoomboxCassette *sound = boombox_cassette_create(boombox);
  if (boombox_cassette_load_sound(sound, "swish.wav") != BOOMBOX_OK) {
    printf("Boombox: failed to load sound\n");
    return -1;
  }

  Muse *muse = muse_create();

  picasso_window_action_callback(&input_action, muse);

  input_init();

  Config config = read_config();

  if (picasso_window_init("Office", config.res_width, config.res_height, config.gl_debug) != PICASSO_WINDOW_OK) {
    printf("Window: failed to init\n");
    return -1;
  }

  MuseFunctionDef action_def = {
    .name = "action",
    .func = &lua_action,
    .num_arguments = 2,
    .arguments = (MuseType[]){
      MUSE_TYPE_STRING,
      MUSE_TYPE_FUNCTION,
    },
    .userdata = NULL,
  };
  muse_add_func(muse, &action_def);
  muse_load_file(muse, "main.lua");

  Screen *screen = screen_create(&config);

  double last_tick = bedrock_time();
  double current_second = 0;

  double frame_timing = (config.frame_lock > 0 ? 1.0 / (double)config.frame_lock : 0);
  double next_frame = frame_timing;

  boombox_cassette_play(sound);

  {
    double test_me_val = 1000;
    MuseArgument result = {
      .type = MUSE_TYPE_NUMBER,
    };
    muse_call_name(muse, "make_leet", 1, (MuseArgument[]){
        {
        .type = MUSE_TYPE_NUMBER,
        .argument = &test_me_val,
        },
        }, 1, &result);
    printf("--> %d leeted becomes %d <--\n", (uint32_t)test_me_val, (uint32_t)*(double*)result.argument);
    free(result.argument);
  }
  {
    double test_me_val = 2000;
    MuseArgument result = {
      .type = MUSE_TYPE_NUMBER,
    };
    muse_call_name(muse, "make_leet", 1, (MuseArgument[]){
        {
        .type = MUSE_TYPE_NUMBER,
        .argument = &test_me_val,
        },
        }, 1, &result);
    printf("--> %d leeted becomes %d <--\n", (uint32_t)test_me_val, (uint32_t)*(double*)result.argument);
    free(result.argument);
  }

  uint32_t frames = 0;
  while (!picasso_window_should_close()) {
    double tick = bedrock_time();
    double delta = tick - last_tick;
    last_tick = tick;

    boombox_update(boombox);

    muse_call_name(muse, "update", 1, (MuseArgument[]){
      {
        .type = MUSE_TYPE_NUMBER,
        .argument = &delta,
      },
    }, 0, NULL);

    screen_update(screen, delta);

    next_frame += delta;
    if (next_frame >= frame_timing) {
      next_frame = 0.0;
      frames++;

      picasso_window_clear();
      screen_draw(screen);
      picasso_window_swap();
    }

    current_second += delta;
    if (current_second >= 1) {
      current_second = 0;
      printf("FPS: %d\n", frames);
      frames = 0;
    }

    picasso_window_update();
  }

  screen_destroy(screen);
  input_kill();

  muse_destroy(muse);
  picasso_window_kill();

  boombox_cassette_destroy(sound);
  boombox_destroy(boombox);

#ifdef MEM_DEBUG
  occulus_print(false);
#endif

  return 0;
}
