#include <stdio.h>
#include <stdint.h>

#include "glad/glad.h"

#define MATH_3D_IMPLEMENTATION
#include "arkanis/math_3d.h"

#include "bedrock/bedrock.h"

#include "config.h"

typedef struct {
  PicassoBufferGroup *buffergroup;
  PicassoProgram *program;
  PicassoTexture *texture;
} Screen;

Screen *screen_create(const Config *config) {
  Screen *screen = calloc(1, sizeof(Screen));

  screen->buffergroup = picasso_buffergroup_create();

  int32_t w = 128; //config.res_width;
  int32_t h = 128; //config.res_height;
  int32_t hw = w / 2;
  int32_t hh = h / 2;
  int32_t ow = config->res_width / 2;
  int32_t oh = config->res_height / 2;
  int32_t vertex_data[] = {
    ow - hw, oh - hh,
    ow + hw, oh + hh,
    ow - hw, oh + hh,

    ow - hw, oh - hh,
    ow + hw, oh - hh,
    ow + hw, oh + hh,
  };
  float coord_data[] = {
    0, 1,
    1, 0,
    0, 0,

    0, 1,
    1, 1,
    1, 0,
  };

  PicassoBuffer *vertex_buffer = picasso_buffer_create(screen->buffergroup, PICASSO_BUFFER_TYPE_ARRAY, PICASSO_BUFFER_USAGE_STATIC);
  picasso_buffer_set_data(vertex_buffer, 2, PICASSO_TYPE_INT, sizeof(*vertex_data) * 12, vertex_data);

  PicassoBuffer *coord_buffer = picasso_buffer_create(screen->buffergroup, PICASSO_BUFFER_TYPE_ARRAY, PICASSO_BUFFER_USAGE_STATIC);
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

    screen->program = picasso_program_create();
    picasso_program_link_shaders(screen->program, 2, (const PicassoShader*[]){
      vertex_shader,
      fragment_shader,
    });

    picasso_shader_destroy(vertex_shader);
    picasso_shader_destroy(fragment_shader);

    free(vert_source);
    free(frag_source);
  }

  picasso_program_use(screen->program);

  {
    int32_t vertex_attr = picasso_program_attrib_location(screen->program, "vertex");
    picasso_buffer_shader_attrib(vertex_buffer, vertex_attr);

    int32_t coord_attr = picasso_program_attrib_location(screen->program, "coord");
    picasso_buffer_shader_attrib(coord_buffer, coord_attr);

    mat4_t ortho = m4_ortho(0, config->res_width, 0, config->res_height, 1, 0);
    mat4_t model = m4_identity();

    int32_t pmatrix_uniform = picasso_program_uniform_location(screen->program, "pMatrix");
    int32_t mvmatrix_uniform = picasso_program_uniform_location(screen->program, "mvMatrix");
    picasso_program_uniform_mat4(screen->program, pmatrix_uniform, (float*)&ortho);
    picasso_program_uniform_mat4(screen->program, mvmatrix_uniform, (float*)&model);
  }

  picasso_program_use(NULL);

  {
    uintmax_t buffer_size = 0;
    uint8_t *buffer;

    screen->texture = picasso_texture_create(PICASSO_TEXTURE_TARGET_2D);

    archivist_read_file("fonts/cp437_8x8.png", &buffer_size, &buffer);
    picasso_texture_load(screen->texture, buffer, buffer_size, PICASSO_TEXTURE_RGB);
    picasso_texture_bind_to(screen->texture, 0);

    int32_t image_uniform = picasso_program_uniform_location(screen->program, "image");
    picasso_program_uniform_int(screen->program, image_uniform, 0);

    free(buffer);
  }

  return screen;
}

void screen_update(Screen *screen, double delta) {
}

void screen_draw(Screen *screen) {
  picasso_program_use(screen->program);
  picasso_buffergroup_draw(screen->buffergroup, PICASSO_BUFFER_MODE_TRIANGLES, 6);
}

void screen_kill(Screen *screen) {
  picasso_texture_destroy(screen->texture);
  picasso_program_destroy(screen->program);
  picasso_buffergroup_destroy(screen->buffergroup);

  free(screen);
}

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
