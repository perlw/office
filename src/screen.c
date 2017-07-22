#include <assert.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "screen.h"

typedef struct {
  ScreenRender render_func;
  void *userdata;
} ScreenRenderHook;

typedef struct {
  AsciiBuffer *ascii;

  ScreenRenderHook *hooks;
} Screen;

Screen *screen_internal = NULL;
void screen_init(void) {
  assert(!screen_internal);

  const Config *const config = config_get();

  screen_internal = calloc(1, sizeof(Screen));

  screen_internal->ascii = ascii_buffer_create(config->res_width, config->res_height, config->ascii_width, config->ascii_height);
  screen_internal->hooks = rectify_array_alloc(10, sizeof(ScreenRenderHook));
}

void screen_kill(void) {
  assert(screen_internal);

  rectify_array_free(screen_internal->hooks);
  ascii_buffer_destroy(screen_internal->ascii);

  free(screen_internal);
}

void screen_hook_render(ScreenRender render_func, void *const userdata /*, uint32_t prio/order */) {
  assert(screen_internal);

  for (uint32_t t = 0; t < rectify_array_size(screen_internal->hooks); t++) {
    ScreenRenderHook *hook = &screen_internal->hooks[t];

    if (hook->render_func == render_func && hook->userdata == userdata) {
      // Already registered error?
      return;
    }
  }

  screen_internal->hooks = rectify_array_push(screen_internal->hooks, &(ScreenRenderHook){
                                                                        .render_func = render_func,
                                                                        .userdata = userdata,
                                                                      });
}

void screen_unhook_render(ScreenRender render_func, void *const userdata) {
  assert(screen_internal);

  for (uint32_t t = 0; t < rectify_array_size(screen_internal->hooks); t++) {
    ScreenRenderHook *hook = &screen_internal->hooks[t];

    if (hook->render_func == render_func && hook->userdata == userdata) {
      screen_internal->hooks = rectify_array_delete(screen_internal->hooks, t);
      return;
    }
  }
}

void screen_render(void) {
  assert(screen_internal);

  for (uint32_t t = 0; t < rectify_array_size(screen_internal->hooks); t++) {
    ScreenRenderHook *hook = &screen_internal->hooks[t];

    hook->render_func(screen_internal->ascii, hook->userdata);
  }

  ascii_buffer_draw(screen_internal->ascii);
}
