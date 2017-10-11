#include <assert.h>

#define USE_PICASSO
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "screen.h"

typedef struct {
  ScreenRender render_func;
  uint32_t layer;
  void *userdata;
} ScreenRenderHook;

typedef struct {
  PicassoWindow *window;
  AsciiBuffer *ascii;

  ScreenRenderHook *hooks;
} Screen;

Screen *screen_internal = NULL;
void screen_init(PicassoWindow *window) {
  assert(window);
  if (screen_internal) {
    return;
  }

  Config *const config = config_get();

  screen_internal = calloc(1, sizeof(Screen));

  screen_internal->window = window;
  screen_internal->ascii = ascii_buffer_create(window, config->res_width, config->res_height, config->ascii_width, config->ascii_height);
  screen_internal->hooks = rectify_array_alloc(10, sizeof(ScreenRenderHook));
}

void screen_kill(void) {
  if (!screen_internal) {
    return;
  }

  rectify_array_free((void **)&screen_internal->hooks);
  ascii_buffer_destroy(&screen_internal->ascii);

  free(screen_internal);
  screen_internal = NULL;
}

void screen_hook_render(ScreenRender render_func, void *const userdata, uint32_t layer) {
  if (!screen_internal) {
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(screen_internal->hooks); t++) {
    ScreenRenderHook *hook = &screen_internal->hooks[t];

    if (hook->render_func == render_func && hook->userdata == userdata) {
      // Already registered error?
      return;
    }
  }

  bool inserted = false;
  ScreenRenderHook *new_list = rectify_array_alloc(10, sizeof(ScreenRenderHook));
  for (uint32_t t = 0; t < rectify_array_size(screen_internal->hooks); t++) {
    if (!inserted && screen_internal->hooks[t].layer >= layer) {
      inserted = true;
      new_list = rectify_array_push(new_list, &(ScreenRenderHook){
                                                .render_func = render_func,
                                                .layer = layer,
                                                .userdata = userdata,
                                              });
    }
    new_list = rectify_array_push(new_list, &screen_internal->hooks[t]);
  }
  if (!inserted) {
    new_list = rectify_array_push(new_list, &(ScreenRenderHook){
                                              .render_func = render_func,
                                              .layer = layer,
                                              .userdata = userdata,
                                            });
  }

  rectify_array_free((void **)&screen_internal->hooks);
  screen_internal->hooks = new_list;
}

void screen_unhook_render(ScreenRender render_func, void *const userdata) {
  if (!screen_internal) {
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(screen_internal->hooks); t++) {
    ScreenRenderHook *hook = &screen_internal->hooks[t];

    if (hook->render_func == render_func && hook->userdata == userdata) {
      screen_internal->hooks = rectify_array_delete(screen_internal->hooks, t);
      return;
    }
  }
}

void screen_render(void) {
  if (!screen_internal) {
    return;
  }

  for (uint32_t t = 0; t < rectify_array_size(screen_internal->hooks); t++) {
    ScreenRenderHook *hook = &screen_internal->hooks[t];

    hook->render_func(screen_internal->ascii, hook->userdata);
  }

  ascii_buffer_draw(screen_internal->ascii);
}
