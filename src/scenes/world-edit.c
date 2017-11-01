#include <assert.h>

#define USE_KRONOS
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#define USE_ASCII
#define USE_CONFIG
#define USE_MESSAGES
#define USE_SCENES
#include "main.h"

typedef struct {
  Surface *world;
  Surface *overlay;

  uint8_t chosen_rune;
  uint32_t chosen_color;

  bool painting;

  uint32_t m_x, m_y;
  uint32_t o_x, o_y;
} SceneWorldEdit;

SceneWorldEdit *scene_world_edit_start(void);
void scene_world_edit_stop(void **scene);
void scene_world_edit_update(SceneWorldEdit *scene, double delta);
RectifyMap *scene_world_edit_message(SceneWorldEdit *scene, uint32_t id, RectifyMap *const map);

KronosSystem scene_world_edit = {
  .name = "scene_world-edit",
  .frames = 30,
  .start = &scene_world_edit_start,
  .stop = &scene_world_edit_stop,
  .update = &scene_world_edit_update,
  .message = &scene_world_edit_message,
};

SceneWorldEdit *scene_world_edit_start(void) {
  Config *const config = config_get();

  SceneWorldEdit *scene = calloc(1, sizeof(SceneWorldEdit));
  *scene = (SceneWorldEdit){
    .m_x = 0,
    .m_y = 0,
    .world = surface_create(0, 0, config->ascii_width - 20, config->ascii_height),
    .overlay = surface_create(0, 0, config->ascii_width - 20, config->ascii_height),
  };

  SurfaceRectTiles rect_tiles = {
    '+',
    '-',
    '+',
    '|',
    0,
    '|',
    '+',
    '-',
    '+',
  };
  surface_rect(scene->world, 0, 0, scene->world->width, scene->world->height, rect_tiles, false, (GlyphColor){ 200, 200, 200 }, (GlyphColor){ 0, 0, 0 });
  surface_textc(scene->world, 40, scene->world->height - 1, 0, " #{ffffff}S#{aaaaaa}ave/Save #{ffffff}a#{aaaaaa}s | #{ffffff}L#{aaaaaa}oad ");

  {
    scene->chosen_rune = 1;
    scene->chosen_color = 0xffffff;
  }

  return scene;
}

void scene_world_edit_stop(void **scene) {
  SceneWorldEdit *ptr = *scene;
  assert(ptr && scene);

  surface_destroy(&ptr->overlay);
  surface_destroy(&ptr->world);

  free(ptr);
  *scene = NULL;
}

void scene_world_edit_update(SceneWorldEdit *scene, double delta) {
  assert(scene);

  if (scene->m_x > 0 && scene->m_x < scene->world->width - 1
      && scene->m_y > 0 && scene->m_y < scene->world->height - 1) {
    if (scene->painting) {
      surface_line(scene->world,
        scene->m_x, scene->m_y,
        scene->o_x, scene->o_y,
        (Glyph){
          .rune = scene->chosen_rune,
          .fore = glyphcolor_hex(scene->chosen_color),
          .back = 0,
        });
    }
  }

  {
    surface_clear(scene->overlay, (Glyph){
                                    .rune = 0,
                                    .fore = 0,
                                    .back = 0,
                                  });
    if (scene->m_x > 0 && scene->m_x < scene->overlay->width - 1
        && scene->m_y > 0 && scene->m_y < scene->overlay->height - 1) {
      surface_glyph(scene->overlay,
        scene->m_x, scene->m_y,
        (Glyph){
          .rune = scene->chosen_rune,
          .fore = glyphcolor_hex(scene->chosen_color),
          .back = 0,
        });
    }
  }

  scene->o_x = scene->m_x;
  scene->o_y = scene->m_y;
}

RectifyMap *scene_world_edit_message(SceneWorldEdit *scene, uint32_t id, RectifyMap *const map) {
  assert(scene);

  switch (id) {
    case MSG_INPUT_MOUSEMOVE: {
      scene->m_x = rectify_map_get_uint(map, "x");
      scene->m_y = rectify_map_get_uint(map, "y");
      break;
    }

    case MSG_INPUT_CLICK: {
      uint32_t x = rectify_map_get_uint(map, "x");
      uint32_t y = rectify_map_get_uint(map, "y");
      bool pressed = rectify_map_get_bool(map, "pressed");

      if (pressed && x > 0 && y > 0 && x < scene->world->width - 1 && y < scene->world->height - 1) {
        scene->m_x = x;
        scene->m_y = y;
        scene->painting = pressed;
      } else {
        scene->painting = false;
      }
      break;
    }

    case MSG_WORLD_EDIT_RUNE_SELECTED: {
      scene->chosen_rune = rectify_map_get_byte(map, "rune");
      break;
    }

    case MSG_WORLD_EDIT_COLOR_SELECTED: {
      scene->chosen_color = rectify_map_get_uint(map, "color");
      break;
    }

    case MSG_SYSTEM_RENDER: {
      AsciiBuffer *screen = *(AsciiBuffer **)rectify_map_get(map, "screen");
      surface_draw(scene->world, screen);
      surface_draw(scene->overlay, screen);
      break;
    }
  }

  return NULL;
}
