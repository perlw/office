#include <assert.h>
#include <stdio.h>
#include <string.h>

#define USE_KRONOS
#define USE_PICASSO
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#define USE_CONFIG
#define USE_MESSAGES
#define USE_SCREEN
#include "main.h"

typedef struct {
  bool should_kill;
  PicassoWindow *window;
  double frame_timing;
  double next_frame;
} SystemWindow;

SystemWindow *system_window_start(void);
void system_window_stop(void **system);
void system_window_update(SystemWindow *system, double delta);
RectifyMap *system_window_message(SystemWindow *system, uint32_t id, RectifyMap *const map);

KronosSystem system_window = {
  .name = "window",
  .frames = 0,
  .prevent_stop = true,
  .autostart = true,
  .start = &system_window_start,
  .stop = &system_window_stop,
  .update = &system_window_update,
  .message = &system_window_message,
};

void system_window_internal_keyboard_callback(const PicassoWindowKeyboardEvent *event);
void system_window_internal_mousemove_callback(const PicassoWindowMouseEvent *event);
void system_window_internal_click_callback(const PicassoWindowMouseEvent *event);
void system_window_internal_mousescroll_callback(const PicassoWindowMouseScrollEvent *event);

SystemWindow *system_window_start(void) {
  Config *const config = config_get();

  SystemWindow *system = calloc(1, sizeof(SystemWindow));
  *system = (SystemWindow){
    .should_kill = false,
  };

  PicassoWindowInit window_init = {
    .width = config->res_width,
    .height = config->res_height,
    .fullscreen = config->fullscreen,
    .gl_debug = config->gl_debug,
  };
  if (picasso_window_init() != PICASSO_WINDOW_OK) {
    printf("Window: failed to init\n");
    return NULL;
  }
  system->window = picasso_window_create("Office", &window_init);
  if (!system->window) {
    return NULL;
  }
  picasso_window_keyboard_callback(system->window, &system_window_internal_keyboard_callback);
  picasso_window_mouse_move_callback(system->window, &system_window_internal_mousemove_callback);
  picasso_window_mouse_button_callback(system->window, &system_window_internal_click_callback);
  picasso_window_mouse_scroll_callback(system->window, &system_window_internal_mousescroll_callback);

  config->res_width = window_init.width;
  config->res_height = window_init.height;

  system->frame_timing = (config->frame_lock > 0 ? 1.0 / (double)config->frame_lock : 0);
  system->next_frame = system->frame_timing;

  screen_init(system->window);

  return system;
}

void system_window_stop(void **system) {
  SystemWindow *ptr = *system;
  assert(ptr && system);

  screen_kill();

  picasso_window_destroy(&ptr->window);
  picasso_window_kill();

  free(ptr);
  *system = NULL;
}

void system_window_update(SystemWindow *system, double delta) {
  assert(system);

  if (picasso_window_should_close(system->window) || system->should_kill) {
    kronos_halt();
  }

  system->next_frame += delta;
  if (system->next_frame >= system->frame_timing) {
    system->next_frame = 0.0;
    picasso_window_clear(system->window);
    screen_render();
    picasso_window_swap(system->window);
  }

  picasso_window_update();
}

RectifyMap *system_window_message(SystemWindow *system, uint32_t id, RectifyMap *const map) {
  assert(system);

  switch (id) {
    case MSG_GAME_KILL:
      system->should_kill = true;
      break;
  }

  return NULL;
}

void system_window_internal_keyboard_callback(const PicassoWindowKeyboardEvent *event) {
  RectifyMap *map = rectify_map_create();
  rectify_map_set_uint(map, "key", event->key);
  rectify_map_set_bool(map, "pressed", event->pressed);
  rectify_map_set_bool(map, "released", event->released);
  kronos_emit(MSG_INPUT_KEY, map);

  if (event->released) {
    return;
  }

  switch (event->key) {
    case PICASSO_KEY_ESCAPE: {
      printf("\n-=ABORT=-\n\n");
      kronos_emit(MSG_GAME_KILL, NULL);
      return;
    }

    case PICASSO_KEY_LEFT:
      kronos_emit(MSG_SCENE_PREV, NULL);
      return;

    case PICASSO_KEY_RIGHT:
      kronos_emit(MSG_SCENE_NEXT, NULL);
      return;
  }
}

void system_window_internal_mousemove_callback(const PicassoWindowMouseEvent *event) {
  Config *const config = config_get();

  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  static uint32_t o_x = 0;
  static uint32_t o_y = 0;

  if (m_x != o_x || m_y != o_y) {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "x", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_x);
    rectify_map_set(map, "y", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_y);
    kronos_emit(MSG_INPUT_MOUSEMOVE, map);

    o_x = m_x;
    o_y = m_y;
  }
}

void system_window_internal_click_callback(const PicassoWindowMouseEvent *event) {
  Config *const config = config_get();

  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  static uint32_t o_x = 0;
  static uint32_t o_y = 0;

  if (m_x != o_x || m_y != o_y) {
    RectifyMap *map = rectify_map_create();
    rectify_map_set(map, "button", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & event->button);
    rectify_map_set(map, "x", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_x);
    rectify_map_set(map, "y", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_y);
    rectify_map_set(map, "pressed", RECTIFY_MAP_TYPE_BOOL, sizeof(bool), (bool *const) & event->pressed);
    rectify_map_set(map, "released", RECTIFY_MAP_TYPE_BOOL, sizeof(bool), (bool *const) & event->released);
    kronos_emit(MSG_INPUT_CLICK, map);

    o_x = m_x;
    o_y = m_y;
  }
}

void system_window_internal_mousescroll_callback(const PicassoWindowMouseScrollEvent *event) {
  Config *const config = config_get();

  uint32_t m_x = (uint32_t)(event->x / config->grid_size_width);
  uint32_t m_y = (uint32_t)(event->y / config->grid_size_height);
  int32_t scroll_x = (int32_t)(event->offset_x < 0.0 ? event->offset_x - 0.5 : event->offset_x + 0.5);
  int32_t scroll_y = (int32_t)(event->offset_y < 0.0 ? event->offset_y - 0.5 : event->offset_y + 0.5);

  RectifyMap *map = rectify_map_create();
  rectify_map_set(map, "x", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_x);
  rectify_map_set(map, "y", RECTIFY_MAP_TYPE_UINT, sizeof(uint32_t), (uint32_t *const) & m_y);
  rectify_map_set(map, "scroll_x", RECTIFY_MAP_TYPE_INT, sizeof(int32_t), (int32_t *const) & scroll_x);
  rectify_map_set(map, "scroll_y", RECTIFY_MAP_TYPE_INT, sizeof(int32_t), (int32_t *const) & scroll_y);
  kronos_emit(MSG_INPUT_SCROLL, map);
}
