// +Dependencies
#ifdef USE_PICASSO
#ifndef USE_ASCII
#define USE_ASCII
#endif
#endif
// -Dependencies

#ifdef USE_ASCII
#ifndef ASCII_LOADED
#define ASCII_LOADED
#include <stdbool.h>

#include "arkanis/math_3d.h"

#define USE_PICASSO
#include "bedrock/bedrock.h"

// +AsciiBuffer
typedef union {
  struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };
} GlyphColor;

typedef struct {
  uint8_t rune;
  GlyphColor fore;
  GlyphColor back;
} Glyph;

GlyphColor glyphcolor(uint8_t r, uint8_t g, uint8_t b);
GlyphColor glyphcolor_hex(uint32_t hex);
GlyphColor glyphcolor_hsl(double h, double s, double l);
GlyphColor glyphcolor_add(GlyphColor a, GlyphColor b);
GlyphColor glyphcolor_sub(GlyphColor a, GlyphColor b);
GlyphColor glyphcolor_adds(GlyphColor c, float s);
GlyphColor glyphcolor_subs(GlyphColor c, float s);
GlyphColor glyphcolor_muls(GlyphColor c, float s);
GlyphColor glyphcolor_divs(GlyphColor c, float s);
bool glyphcolor_eq(GlyphColor a, GlyphColor b);
uint32_t glyphcolor_to_uint(GlyphColor c);

typedef struct {
  uint8_t *rune_buffer;
  GlyphColor *fore_buffer;
  GlyphColor *back_buffer;
} DisplayBuffer;

typedef struct {
  PicassoWindow *window;

  PicassoBufferGroup *quad;
  PicassoProgram *program;
  PicassoTexture *font_texture;

  uint32_t width;
  uint32_t height;
  uint32_t size;

  bool dirty;
  DisplayBuffer buffers[2];
  DisplayBuffer *front_buffer;
  DisplayBuffer *back_buffer;

  PicassoTexture *asciimap_texture;
  PicassoTexture *forecolors_texture;
  PicassoTexture *backcolors_texture;

  struct {
    PicassoFramebuffer *framebuffer;
    PicassoTexture *texture;
    PicassoProgram *program;
    PicassoBufferGroup *quad;
  } fbo;
} AsciiBuffer;

AsciiBuffer *ascii_buffer_create(PicassoWindow *window, uint32_t width, uint32_t height, uint32_t ascii_width, uint32_t ascii_height);
void ascii_buffer_destroy(AsciiBuffer **ascii);

void ascii_buffer_glyph(AsciiBuffer *const ascii, uint32_t x, uint32_t y, Glyph glyph);
void ascii_buffer_draw(AsciiBuffer *const ascii);
// -AsciiBuffer

// +Surface
typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  uint32_t size;
  Glyph *buffer;
} Surface;

typedef struct {
  uint8_t tl, t, tr;
  uint8_t l, c, r;
  uint8_t bl, b, br;
} SurfaceRectTiles;

Surface *surface_create(uint32_t pos_x, uint32_t pos_y, uint32_t width, uint32_t height);
Surface *surface_clone(Surface *const original);
void surface_destroy(Surface **surface);

void surface_clear(Surface *const surface, Glyph glyph);
void surface_text(Surface *const surface, uint32_t x, uint32_t y, uint32_t length, const char *string, GlyphColor fore_color, GlyphColor back_color);
void surface_textc(Surface *const surface, uint32_t x, uint32_t y, uint32_t length, const char *string);
void surface_rect(Surface *const surface, uint32_t x, uint32_t y, uint32_t width, uint32_t height, SurfaceRectTiles rect_tiles, bool filled, GlyphColor fore_color, GlyphColor back_color);
void surface_line(Surface *const surface, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, Glyph glyph);
void surface_graph(Surface *const surface, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t num_values, const float *values);
void surface_glyph(Surface *const surface, uint32_t x, uint32_t y, Glyph glyph);

void surface_draw(Surface *const surface, AsciiBuffer *const tiles);
// -Surface
#endif // ASCII_LOADED
#endif // USE_ASCII

#ifdef USE_ASSETS
#ifndef ASSETS_LOADED
#define ASSETS_LOADED
typedef enum {
  ASSET_SHADER = 1,
  ASSET_TEXTURE,
  ASSET_TILEDEFS,
} AssetTypes;

void setup_asset_loaders(void);
#endif // ASSETS_LOADED
#endif // USE_ASSETS

#ifdef USE_CONFIG
#ifndef CONFIG_LOADED
#define CONFIG_LOADED
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint32_t res_width;
  uint32_t res_height;
  bool fullscreen;
  bool gl_debug;
  uint32_t frame_lock;
  uint32_t ascii_width;
  uint32_t ascii_height;
  uint32_t grid_size_width;
  uint32_t grid_size_height;
} Config;

Config *const config_init(void);
Config *const config_get(void);
#endif // CONFIG_LOADED
#endif // USE_CONFIG

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

#ifdef USE_MESSAGES
#ifndef MESSAGES_LOADED
#define MESSAGES_LOADED
#include <stdio.h>

#define FOREACH_MESSAGE(MSG)         \
  MSG(MSG_CONFIG_UPDATE)             \
  MSG(MSG_RENDER_SETTINGS_UPDATE)    \
                                     \
  MSG(MSG_SYSTEM_START)              \
  MSG(MSG_SYSTEM_STOP)               \
  MSG(MSG_SYSTEM_SPUN_UP)            \
  MSG(MSG_SYSTEM_SHUT_DOWN)          \
  MSG(MSG_SYSTEM_RENDER)             \
  MSG(MSG_SYSTEM_RENDER_TOP)         \
                                     \
  MSG(MSG_GAME_INIT)                 \
  MSG(MSG_GAME_KILL)                 \
                                     \
  MSG(MSG_INPUT_BIND)                \
  MSG(MSG_INPUT_ACTION)              \
  MSG(MSG_INPUT_KEY)                 \
  MSG(MSG_INPUT_MOUSEMOVE)           \
  MSG(MSG_INPUT_CLICK)               \
  MSG(MSG_INPUT_SCROLL)              \
                                     \
  MSG(MSG_SCENE_GOTO)                \
  MSG(MSG_SCENE_PREV)                \
  MSG(MSG_SCENE_NEXT)                \
  MSG(MSG_SCENE_SETUP)               \
  MSG(MSG_SCENE_TEARDOWN)            \
  MSG(MSG_SCENE_CHANGED)             \
                                     \
  MSG(MSG_SOUND_PLAY)                \
  MSG(MSG_SOUND_PLAY_SONG)           \
  MSG(MSG_SOUND_STOP_SONG)           \
  MSG(MSG_SOUND_SPECTRUM)            \
  MSG(MSG_SOUND_LIST)                \
                                     \
  MSG(MSG_UI_WINDOW_CREATE)          \
  MSG(MSG_UI_WINDOW_DESTROY)         \
  MSG(MSG_UI_WINDOW_GLYPH)           \
  MSG(MSG_UI_WINDOW_GLYPHS)          \
  MSG(MSG_UI_WINDOW_MOUSEMOVE)       \
  MSG(MSG_UI_WINDOW_CLICK)           \
  MSG(MSG_UI_WINDOW_SCROLL)          \
                                     \
  MSG(MSG_WORLD_EDIT_RUNE_SELECTED)  \
  MSG(MSG_WORLD_EDIT_COLOR_SELECTED) \
                                     \
  MSG(MSG_DEBUG_TEST)

typedef enum {
  FOREACH_MESSAGE(GENERATE_ENUM)
} Messages;

#ifdef USE_MESSAGES_NAMES
static char *MSG_NAMES[] = {
  FOREACH_MESSAGE(GENERATE_STRING)
    NULL,
};
#endif // USE_MESSAGES_NAMES
#endif // MESSAGES_LOADED
#endif // USE_MESSAGES

#ifdef USE_QUEUES
#ifndef QUEUES_LOADED
#define QUEUES_LOADED

#define FOREACH_QUEUE(QUEUE) \
  QUEUE(QUEUE_BASE)          \
  QUEUE(QUEUE_RENDER)

typedef enum {
  FOREACH_QUEUE(GENERATE_ENUM)
} Queues;

#ifdef USE_QUEUES_NAMES
static char *QUEUE_NAMES[] = {
  FOREACH_QUEUES(GENERATE_STRING)
    NULL,
};
#endif // USE_QUEUES_NAMES
#endif // QUEUES_LOADED
#endif // USE_QUEUES

#ifdef USE_SCENES
#ifndef SCENES_LOADED
#define SCENES_LOADED
#define USE_KRONOS
#include "bedrock/bedrock.h"

extern KronosSystem scene_drips;
extern KronosSystem scene_game;
extern KronosSystem scene_sound_test;
extern KronosSystem scene_test;
extern KronosSystem scene_world_edit;
extern KronosSystem scene_title;

extern KronosSystem scenes;
#endif // SCENES_LOADED
#endif // USE_SCENES

#ifdef USE_SYSTEMS
#ifndef SYSTEMS_LOADED
#define SYSTEMS_LOADED
#define USE_KRONOS
#define USE_PICASSO
#include "bedrock/bedrock.h"

extern KronosSystem system_debug;
extern KronosSystem system_render;
extern KronosSystem system_input;
extern KronosSystem system_lua_bridge;
extern KronosSystem system_sound;
extern KronosSystem system_test;
extern KronosSystem system_ui;
extern KronosSystem system_window;

extern KronosSystem systems;
#endif // SYSTEMS_LOADED
#endif // USE_SYSTEMS
