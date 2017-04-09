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

    mat4_t ortho = m4_ortho(0, (float)width, 0, (float)height, 1, 0);
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
// -Surface

// +Screen
Screen *screen_create(const Config *config) {
  Screen *screen = calloc(1, sizeof(Screen));

  screen->asciilayer = asciilayer_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);
  screen->surfaces = rectify_array_alloc(10, sizeof(Surface*));

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

// +Scene
typedef struct {
  double offset;
  double timing;
  double since_update;
  bool dirty;

  Screen *screen;

  Surface *surface;
  Surface *surface2;
  Surface *surface3;
} Scene;

Scene *scene_create(const Config *config) {
  Scene *scene = calloc(1, sizeof(Scene));

  scene->offset = 0.0;
  scene->timing = 1 / 30.0;
  scene->since_update = scene->timing;
  scene->dirty = true;
  scene->screen = screen_create(config);

  scene->surface = surface_create(scene->screen, 0, 0, config->ascii_width, config->ascii_height);
  scene->surface2 = surface_create(scene->screen, 0, 0, 32, 32);
  scene->surface3 = surface_create(scene->screen, 16, 16, 32, 32);

  return scene;
}

void scene_destroy(Scene *scene) {
  assert(scene);

  surface_destroy(scene->surface3);
  surface_destroy(scene->surface2);
  surface_destroy(scene->surface);
  screen_destroy(scene->screen);

  free(scene);
}

void scene_update(Scene *scene, double delta) {
  scene->since_update += delta;
  while (scene->since_update >= scene->timing) {
    scene->since_update -= scene->timing;

    // Wave
    {
      scene->offset += 0.1;

      double wave_depth = 0.25;
      double wave_thickness = M_PI * 4.0;
      double cx = scene->surface->width  / 2;
      double cy = scene->surface->height / 2;
      for (uintmax_t y = 0; y < scene->surface->height; y++) {
        for (uintmax_t x = 0; x < scene->surface->width; x++) {
          double dx = fabs((double)x - cx);
          double dy = fabs((double)y - cy);
          double dist = sqrt(pow(dx, 2) + pow(dy, 2));
          double ndist = dist / 50.0;

          double final_color = ((cos((ndist * wave_thickness) + scene->offset) + 1.0) / 4.0) + wave_depth;

          uintmax_t i = (y * scene->surface->width) + x;
          uint8_t color = (uintmax_t)(final_color * 255.0);
          if (color < 96) {
            scene->surface->asciimap[i].rune = '.';
          } else if (color < 178) {
            scene->surface->asciimap[i].rune = '+';
          } else {
            scene->surface->asciimap[i].rune = '*';
          }
          scene->surface->asciimap[i].fore = color;
          scene->surface->asciimap[i].back = 0;
        }
      }
    }

    scene->dirty = true;
  }
}

void scene_draw(Scene *scene) {
  screen_draw(scene->screen, scene->dirty);
  scene->dirty = false;
}
// -Scene

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

  Scene *scene = scene_create(&config);

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

    scene_update(scene, delta);

    next_frame += delta;
    if (next_frame >= frame_timing) {
      next_frame = 0.0;
      frames++;

      picasso_window_clear();
      scene_draw(scene);
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

  scene_destroy(scene);
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
