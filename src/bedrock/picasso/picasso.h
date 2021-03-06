/**
 * Picasso - OpenGL frontend and window management
 */

#pragma once

// +Window
typedef enum {
  PICASSO_KEY_SPACE = 32,
  PICASSO_KEY_APOSTROPHE = 39,
  PICASSO_KEY_COMMA = 44,
  PICASSO_KEY_MINUS = 45,
  PICASSO_KEY_PERIOD = 46,
  PICASSO_KEY_SLASH = 47,
  PICASSO_KEY_0 = 48,
  PICASSO_KEY_1 = 49,
  PICASSO_KEY_2 = 50,
  PICASSO_KEY_3 = 51,
  PICASSO_KEY_4 = 52,
  PICASSO_KEY_5 = 53,
  PICASSO_KEY_6 = 54,
  PICASSO_KEY_7 = 55,
  PICASSO_KEY_8 = 56,
  PICASSO_KEY_9 = 57,
  PICASSO_KEY_SEMICOLON = 59,
  PICASSO_KEY_EQUAL = 61,
  PICASSO_KEY_A = 65,
  PICASSO_KEY_B = 66,
  PICASSO_KEY_C = 67,
  PICASSO_KEY_D = 68,
  PICASSO_KEY_E = 69,
  PICASSO_KEY_F = 70,
  PICASSO_KEY_G = 71,
  PICASSO_KEY_H = 72,
  PICASSO_KEY_I = 73,
  PICASSO_KEY_J = 74,
  PICASSO_KEY_K = 75,
  PICASSO_KEY_L = 76,
  PICASSO_KEY_M = 77,
  PICASSO_KEY_N = 78,
  PICASSO_KEY_O = 79,
  PICASSO_KEY_P = 80,
  PICASSO_KEY_Q = 81,
  PICASSO_KEY_R = 82,
  PICASSO_KEY_S = 83,
  PICASSO_KEY_T = 84,
  PICASSO_KEY_U = 85,
  PICASSO_KEY_V = 86,
  PICASSO_KEY_W = 87,
  PICASSO_KEY_X = 88,
  PICASSO_KEY_Y = 89,
  PICASSO_KEY_Z = 90,
  PICASSO_KEY_LEFT_BRACKET = 91,
  PICASSO_KEY_BACKSLASH = 92,
  PICASSO_KEY_RIGHT_BRACKET = 93,
  PICASSO_KEY_GRAVE_ACCENT = 96,
  PICASSO_KEY_WORLD_1 = 161,
  PICASSO_KEY_WORLD_2 = 162,

  PICASSO_KEY_ESCAPE = 256,
  PICASSO_KEY_ENTER = 257,
  PICASSO_KEY_TAB = 258,
  PICASSO_KEY_BACKSPACE = 259,
  PICASSO_KEY_INSERT = 260,
  PICASSO_KEY_DELETE = 261,
  PICASSO_KEY_RIGHT = 262,
  PICASSO_KEY_LEFT = 263,
  PICASSO_KEY_DOWN = 264,
  PICASSO_KEY_UP = 265,
  PICASSO_KEY_PAGE_UP = 266,
  PICASSO_KEY_PAGE_DOWN = 267,
  PICASSO_KEY_HOME = 268,
  PICASSO_KEY_END = 269,
  PICASSO_KEY_CAPS_LOCK = 280,
  PICASSO_KEY_SCROLL_LOCK = 281,
  PICASSO_KEY_NUM_LOCK = 282,
  PICASSO_KEY_PRINT_SCREEN = 283,
  PICASSO_KEY_PAUSE = 284,
  PICASSO_KEY_F1 = 290,
  PICASSO_KEY_F2 = 291,
  PICASSO_KEY_F3 = 292,
  PICASSO_KEY_F4 = 293,
  PICASSO_KEY_F5 = 294,
  PICASSO_KEY_F6 = 295,
  PICASSO_KEY_F7 = 296,
  PICASSO_KEY_F8 = 297,
  PICASSO_KEY_F9 = 298,
  PICASSO_KEY_F10 = 299,
  PICASSO_KEY_F11 = 300,
  PICASSO_KEY_F12 = 301,
  PICASSO_KEY_F13 = 302,
  PICASSO_KEY_F14 = 303,
  PICASSO_KEY_F15 = 304,
  PICASSO_KEY_F16 = 305,
  PICASSO_KEY_F17 = 306,
  PICASSO_KEY_F18 = 307,
  PICASSO_KEY_F19 = 308,
  PICASSO_KEY_F20 = 309,
  PICASSO_KEY_F21 = 310,
  PICASSO_KEY_F22 = 311,
  PICASSO_KEY_F23 = 312,
  PICASSO_KEY_F24 = 313,
  PICASSO_KEY_F25 = 314,
  PICASSO_KEY_KP_0 = 320,
  PICASSO_KEY_KP_1 = 321,
  PICASSO_KEY_KP_2 = 322,
  PICASSO_KEY_KP_3 = 323,
  PICASSO_KEY_KP_4 = 324,
  PICASSO_KEY_KP_5 = 325,
  PICASSO_KEY_KP_6 = 326,
  PICASSO_KEY_KP_7 = 327,
  PICASSO_KEY_KP_8 = 328,
  PICASSO_KEY_KP_9 = 329,
  PICASSO_KEY_KP_DECIMAL = 330,
  PICASSO_KEY_KP_DIVIDE = 331,
  PICASSO_KEY_KP_MULTIPLY = 332,
  PICASSO_KEY_KP_SUBTRACT = 333,
  PICASSO_KEY_KP_ADD = 334,
  PICASSO_KEY_KP_ENTER = 335,
  PICASSO_KEY_KP_EQUAL = 336,
  PICASSO_KEY_LEFT_SHIFT = 340,
  PICASSO_KEY_LEFT_CONTROL = 341,
  PICASSO_KEY_LEFT_ALT = 342,
  PICASSO_KEY_LEFT_SUPER = 343,
  PICASSO_KEY_RIGHT_SHIFT = 344,
  PICASSO_KEY_RIGHT_CONTROL = 345,
  PICASSO_KEY_RIGHT_ALT = 346,
  PICASSO_KEY_RIGHT_SUPER = 347,
  PICASSO_KEY_MENU = 348,
} PicassoKey;

typedef enum {
  PICASSO_MOUSE_BUTTON_LEFT = 0,
  PICASSO_MOUSE_BUTTON_RIGHT,
  PICASSO_MOUSE_BUTTON_MIDDLE,
  PICASSO_MOUSE_BUTTON_FORWARD,
  PICASSO_MOUSE_BUTTON_BACK,
} PicassoMouseButton;

typedef enum {
  PICASSO_WINDOW_OK = 1,
  PICASSO_WINDOW_INIT_FAIL,
  PICASSO_WINDOW_CREATION_FAIL,
  PICASSO_WINDOW_GL_CONTEXT_FAIL,
} PicassoWindowResult;

typedef struct {
  PicassoKey key;
  int32_t scancode;
  bool pressed;
  bool released;
  bool shift;
} PicassoWindowKeyboardEvent;
typedef struct {
  int32_t button;
  double x;
  double y;
  bool pressed;
  bool released;
} PicassoWindowMouseEvent;
typedef struct {
  double offset_x;
  double offset_y;
  double x;
  double y;
} PicassoWindowMouseScrollEvent;
typedef void (*PicassoWindowKeyboardCallback)(const PicassoWindowKeyboardEvent *const);
typedef void (*PicassoWindowMouseCallback)(const PicassoWindowMouseEvent *const);
typedef void (*PicassoWindowMouseScrollCallback)(const PicassoWindowMouseScrollEvent *const);

typedef struct {
  uint32_t width;
  uint32_t height;
  bool fullscreen;
  bool gl_debug;
} PicassoWindowInit;

typedef struct PicassoWindow PicassoWindow;

PicassoWindowResult picasso_window_init(void);
void picasso_window_kill(void);

PicassoWindow *picasso_window_create(const char *title, PicassoWindowInit *const window_init);
void picasso_window_destroy(PicassoWindow **window);
void picasso_window_update(void);

void picasso_window_clear(PicassoWindow *window);
void picasso_window_clearcolor(PicassoWindow *window, float r, float g, float b, float a);
void picasso_window_swap(PicassoWindow *window);
bool picasso_window_should_close(PicassoWindow *window);
void picasso_window_make_context_current(PicassoWindow *window);
void picasso_window_keyboard_callback(PicassoWindow *window, PicassoWindowKeyboardCallback callback);
void picasso_window_mouse_move_callback(PicassoWindow *window, PicassoWindowMouseCallback callback);
void picasso_window_mouse_button_callback(PicassoWindow *window, PicassoWindowMouseCallback callback);
void picasso_window_mouse_scroll_callback(PicassoWindow *window, PicassoWindowMouseScrollCallback callback);
void picasso_window_viewport(PicassoWindow *window, int32_t x, int32_t y, int32_t width, int32_t height);
// -Window

// +Shaders
typedef enum {
  PICASSO_SHADER_VERTEX = 1,
  PICASSO_SHADER_FRAGMENT,
} PicassoShaderType;

typedef enum {
  PICASSO_SHADER_OK = 1,
  PICASSO_SHADER_COMPILE_FAILED,
} PicassoShaderResult;

typedef struct {
  PicassoShaderResult result;
  uint8_t length;
  char detail[256];
} PicassoShaderDetailResult;

typedef struct PicassoShader PicassoShader;
typedef struct PicassoProgram PicassoProgram;

PicassoShader *picasso_shader_create(PicassoShaderType type);
void picasso_shader_destroy(PicassoShader **shader);
PicassoShaderDetailResult picasso_shader_compile(PicassoShader *const shader, uintmax_t length, const uint8_t *const source);

PicassoProgram *picasso_program_create(void);
void picasso_program_destroy(PicassoProgram **program);
PicassoShaderDetailResult picasso_program_link_shaders(PicassoProgram *const program, uintmax_t num_shaders, const PicassoShader **shaders);
void picasso_program_use(PicassoProgram *const program);
int32_t picasso_program_attrib_location(PicassoProgram *const program, const char *name);
int32_t picasso_program_uniform_location(PicassoProgram *const program, const char *name);
void picasso_program_uniform_int(PicassoProgram *const program, int32_t uniform, int32_t val);
void picasso_program_uniform_float(PicassoProgram *const program, int32_t uniform, float val);
void picasso_program_uniform_ivec2(PicassoProgram *const program, int32_t uniform, const int32_t *vec);
void picasso_program_uniform_mat4(PicassoProgram *const program, int32_t uniform, const float *mat);
// -Shaders

// +Buffers
typedef enum {
  PICASSO_BUFFER_TYPE_ARRAY = 1,
  PICASSO_BUFFER_TYPE_ELEMENT,
} PicassoBufferType;

typedef enum {
  PICASSO_BUFFER_USAGE_STREAM = 1,
  PICASSO_BUFFER_USAGE_STATIC,
  PICASSO_BUFFER_USAGE_DYNAMIC,
} PicassoBufferUsage;

typedef enum {
  PICASSO_TYPE_INT = 1,
  PICASSO_TYPE_FLOAT,
} PicassoDataType;

typedef enum {
  PICASSO_BUFFER_MODE_LINES = 1,
  PICASSO_BUFFER_MODE_TRIANGLES,
} PicassoBufferMode;

typedef struct PicassoBufferGroup PicassoBufferGroup;
typedef struct PicassoBuffer PicassoBuffer;

PicassoBufferGroup *picasso_buffergroup_create(void);
void picasso_buffergroup_destroy(PicassoBufferGroup **buffergroup);
void picasso_buffergroup_draw(PicassoBufferGroup *const buffergroup, PicassoBufferMode mode, uintmax_t num_vertices);

PicassoBuffer *picasso_buffer_create(PicassoBufferGroup *const buffergroup, PicassoBufferType type, PicassoBufferUsage usage);
void picasso_buffer_set_data(PicassoBuffer *const buffer, uintmax_t num_fields, PicassoDataType type, uintmax_t size, const void *data);
void picasso_buffer_shader_attrib(PicassoBuffer *const buffer, int32_t attr_pos);
// -Buffers

// +Textures
typedef enum {
  PICASSO_TEXTURE_TARGET_1D = 1,
  PICASSO_TEXTURE_TARGET_2D,
} PicassoTextureTarget;

typedef enum {
  PICASSO_TEXTURE_R = 1,
  PICASSO_TEXTURE_G = 1,
  PICASSO_TEXTURE_B = 1,
  PICASSO_TEXTURE_RGB = 3,
  PICASSO_TEXTURE_RGBA = 4,
} PicassoTextureChannels;

typedef enum {
  PICASSO_TEXTURE_OK = 1,
  PICASSO_TEXTURE_OUT_OF_BOUNDS,
} PicassoTextureResult;

typedef struct PicassoTexture PicassoTexture;

PicassoTexture *picasso_texture_create(PicassoTextureTarget target, uintmax_t width, uintmax_t height, PicassoTextureChannels channels, bool filtered);
PicassoTexture *picasso_texture_load(PicassoTextureTarget target, PicassoTextureChannels channels, uintmax_t size, const uint8_t *data);
void picasso_texture_destroy(PicassoTexture **texture);

PicassoTextureResult picasso_texture_set_data(PicassoTexture *const texture, uintmax_t offset_x, uintmax_t offset_y, uintmax_t width, uintmax_t height, const void *data);
void picasso_texture_bind_to(PicassoTexture *const texture, uint32_t index);
// -Textures

// +Framebuffers
typedef enum {
  PICASSO_FRAMEBUFFER_OK = 1,
  PICASSO_FRAMEBUFFER_INCOMPLETE,
} PicassoFramebufferResult;

typedef struct PicassoFramebuffer PicassoFramebuffer;

PicassoFramebuffer *picasso_framebuffer_create(void);
void picasso_framebuffer_destroy(PicassoFramebuffer **buffer);

PicassoFramebufferResult picasso_framebuffer_attach_texture(PicassoFramebuffer *const buffer, PicassoTexture *const texture);

void picasso_framebuffer_bind(PicassoFramebuffer *const buffer);
void picasso_framebuffer_unbind(PicassoFramebuffer *const buffer);
// -Framebuffers
