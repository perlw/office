#include <assert.h>

#include "bedrock/bedrock.h"

#include "ascii/ascii.h"
#include "config.h"
#include "messages.h"
#include "screen.h"

bool scene_world_edit_start(void);
void scene_world_edit_stop(void);
void scene_world_edit_update(void);
void scene_world_edit_message(uint32_t id, RectifyMap *const map);

KronosSystem scene_world_edit = {
  .name = "scene_world-edit",
  .frames = 30,
  .start = &scene_world_edit_start,
  .stop = &scene_world_edit_stop,
  .update = &scene_world_edit_update,
  .message = &scene_world_edit_message,
};

typedef struct {
  Surface *world;
  Surface *overlay;

  uint8_t chosen_rune;
  uint32_t chosen_color;

  bool painting;

  uint32_t m_x, m_y;
  uint32_t o_x, o_y;
} SceneWorldEdit;

void scene_world_edit_internal_render_hook(AsciiBuffer *const screen, void *const userdata);

SceneWorldEdit *scene_world_edit_internal = NULL;
bool scene_world_edit_start(void) {
  if (scene_world_edit_internal) {
    return false;
  }

  const Config *const config = config_get();

  scene_world_edit_internal = calloc(1, sizeof(SceneWorldEdit));
  *scene_world_edit_internal = (SceneWorldEdit){
    .m_x = 0,
    .m_y = 0,
    .world = surface_create(0, 0, config->ascii_width - 20, config->ascii_height),
    .overlay = surface_create(0, 0, config->ascii_width - 20, config->ascii_height),
  };

  SurfaceRectTiles rect_tiles = {
    '+', '-', '+',
    '|', 0, '|',
    '+', '-', '+',
  };
  surface_rect(scene_world_edit_internal->world, 0, 0, scene_world_edit_internal->world->width, scene_world_edit_internal->world->height, rect_tiles, false, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });
  surface_textc(scene_world_edit_internal->world, 40, scene_world_edit_internal->world->height - 1, 0, " #{ffffff}S#{aaaaaa}ave/Save #{ffffff}a#{aaaaaa}s | #{ffffff}L#{aaaaaa}oad ");

  {
    scene_world_edit_internal->chosen_rune = 1;
    scene_world_edit_internal->chosen_color = 0xffffff;
  }

  screen_hook_render(&scene_world_edit_internal_render_hook, NULL, 0);

  return true;
}

void scene_world_edit_stop(void) {
  if (!scene_world_edit_internal) {
    return;
  }

  screen_unhook_render(&scene_world_edit_internal_render_hook, NULL);

  surface_destroy(scene_world_edit_internal->overlay);
  surface_destroy(scene_world_edit_internal->world);

  free(scene_world_edit_internal);
  scene_world_edit_internal = NULL;
}

void scene_world_edit_update(void) {
  if (!scene_world_edit_internal) {
    return;
  }

  if (scene_world_edit_internal->m_x > 0 && scene_world_edit_internal->m_x < scene_world_edit_internal->world->width - 1
      && scene_world_edit_internal->m_y > 0 && scene_world_edit_internal->m_y < scene_world_edit_internal->world->height - 1) {
    if (scene_world_edit_internal->painting) {
      surface_line(scene_world_edit_internal->world,
        scene_world_edit_internal->m_x, scene_world_edit_internal->m_y,
        scene_world_edit_internal->o_x, scene_world_edit_internal->o_y,
        (Glyph){
          .rune = scene_world_edit_internal->chosen_rune,
          .fore = glyphcolor_hex(scene_world_edit_internal->chosen_color),
          .back = 0,
        });
    }
  }

  {
    surface_clear(scene_world_edit_internal->overlay, (Glyph){
                                                        .rune = 0,
                                                        .fore = 0,
                                                        .back = 0,
                                                      });
    if (scene_world_edit_internal->m_x > 0 && scene_world_edit_internal->m_x < scene_world_edit_internal->overlay->width - 1
        && scene_world_edit_internal->m_y > 0 && scene_world_edit_internal->m_y < scene_world_edit_internal->overlay->height - 1) {
      surface_glyph(scene_world_edit_internal->overlay,
        scene_world_edit_internal->m_x, scene_world_edit_internal->m_y,
        (Glyph){
          .rune = scene_world_edit_internal->chosen_rune,
          .fore = glyphcolor_hex(scene_world_edit_internal->chosen_color),
          .back = 0,
        });
    }
  }

  scene_world_edit_internal->o_x = scene_world_edit_internal->m_x;
  scene_world_edit_internal->o_y = scene_world_edit_internal->m_y;
}

void scene_world_edit_message(uint32_t id, RectifyMap *const map) {
  if (!scene_world_edit_internal) {
    return;
  }

  switch (id) {
    case MSG_INPUT_MOUSEMOVE:
      scene_world_edit_internal->m_x = rectify_map_get_uint(map, "x");
      scene_world_edit_internal->m_y = rectify_map_get_uint(map, "y");
      break;

    case MSG_INPUT_CLICK: {
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");
      bool pressed = rectify_map_get_bool(map, "pressed");

      if (pressed && x > 0 && y > 0 && x < scene_world_edit_internal->world->width - 1 && y < scene_world_edit_internal->world->height - 1) {
        scene_world_edit_internal->m_x = x;
        scene_world_edit_internal->m_y = y;
        scene_world_edit_internal->painting = pressed;
      } else {
        scene_world_edit_internal->painting = false;
      }
      break;
    }
  }

  /*
  SceneWorldEdit *scene = (SceneWorldEdit *)subscriberdata;
  scene->chosen_rune = *(uint32_t *)userdata;
  */

  /*
  SceneWorldEdit *scene = (SceneWorldEdit *)subscriberdata;
  scene->chosen_color = *(uint32_t *)userdata;
  */
}

void scene_world_edit_internal_render_hook(AsciiBuffer *const screen, void *const userdata) {
  if (!scene_world_edit_internal) {
    return;
  }
  surface_draw(scene_world_edit_internal->world, screen);
  surface_draw(scene_world_edit_internal->overlay, screen);
}
