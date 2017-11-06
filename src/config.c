#include <assert.h>
#include <stdio.h>
#include <string.h>

#define USE_ARCHIVIST
#define USE_KRONOS
#define USE_PICASSO
#define USE_RECTIFY
#include "bedrock/bedrock.h"

#define USE_CONFIG
#define USE_MESSAGES
#include "main.h"

struct {
  char *name;
  uint32_t val;
} key_names[] = {
  {
    .name = "SPACE",
    .val = PICASSO_KEY_SPACE,
  },
  {
    .name = "'",
    .val = PICASSO_KEY_APOSTROPHE,
  },
  {
    .name = ",",
    .val = PICASSO_KEY_COMMA,
  },
  {
    .name = "-",
    .val = PICASSO_KEY_MINUS,
  },
  {
    .name = ".",
    .val = PICASSO_KEY_PERIOD,
  },
  {
    .name = "/",
    .val = PICASSO_KEY_SLASH,
  },
  {
    .name = "0",
    .val = PICASSO_KEY_0,
  },
  {
    .name = "1",
    .val = PICASSO_KEY_1,
  },
  {
    .name = "2",
    .val = PICASSO_KEY_2,
  },
  {
    .name = "3",
    .val = PICASSO_KEY_3,
  },
  {
    .name = "4",
    .val = PICASSO_KEY_4,
  },
  {
    .name = "5",
    .val = PICASSO_KEY_5,
  },
  {
    .name = "6",
    .val = PICASSO_KEY_6,
  },
  {
    .name = "7",
    .val = PICASSO_KEY_7,
  },
  {
    .name = "8",
    .val = PICASSO_KEY_8,
  },
  {
    .name = "9",
    .val = PICASSO_KEY_9,
  },
  {
    .name = ";",
    .val = PICASSO_KEY_SEMICOLON,
  },
  {
    .name = "=",
    .val = PICASSO_KEY_EQUAL,
  },
  {
    .name = "A",
    .val = PICASSO_KEY_A,
  },
  {
    .name = "B",
    .val = PICASSO_KEY_B,
  },
  {
    .name = "C",
    .val = PICASSO_KEY_C,
  },
  {
    .name = "D",
    .val = PICASSO_KEY_D,
  },
  {
    .name = "E",
    .val = PICASSO_KEY_E,
  },
  {
    .name = "F",
    .val = PICASSO_KEY_F,
  },
  {
    .name = "G",
    .val = PICASSO_KEY_G,
  },
  {
    .name = "H",
    .val = PICASSO_KEY_H,
  },
  {
    .name = "I",
    .val = PICASSO_KEY_I,
  },
  {
    .name = "J",
    .val = PICASSO_KEY_J,
  },
  {
    .name = "K",
    .val = PICASSO_KEY_K,
  },
  {
    .name = "L",
    .val = PICASSO_KEY_L,
  },
  {
    .name = "M",
    .val = PICASSO_KEY_M,
  },
  {
    .name = "N",
    .val = PICASSO_KEY_N,
  },
  {
    .name = "O",
    .val = PICASSO_KEY_O,
  },
  {
    .name = "P",
    .val = PICASSO_KEY_P,
  },
  {
    .name = "Q",
    .val = PICASSO_KEY_Q,
  },
  {
    .name = "R",
    .val = PICASSO_KEY_R,
  },
  {
    .name = "S",
    .val = PICASSO_KEY_S,
  },
  {
    .name = "T",
    .val = PICASSO_KEY_T,
  },
  {
    .name = "U",
    .val = PICASSO_KEY_U,
  },
  {
    .name = "V",
    .val = PICASSO_KEY_V,
  },
  {
    .name = "W",
    .val = PICASSO_KEY_W,
  },
  {
    .name = "X",
    .val = PICASSO_KEY_X,
  },
  {
    .name = "Y",
    .val = PICASSO_KEY_Y,
  },
  {
    .name = "Z",
    .val = PICASSO_KEY_Z,
  },
  {
    .name = "[",
    .val = PICASSO_KEY_LEFT_BRACKET,
  },
  {
    .name = "\\",
    .val = PICASSO_KEY_BACKSLASH,
  },
  {
    .name = "]",
    .val = PICASSO_KEY_RIGHT_BRACKET,
  },
  {
    .name = "ESCAPE",
    .val = PICASSO_KEY_ESCAPE,
  },
  {
    .name = "ENTER",
    .val = PICASSO_KEY_ENTER,
  },
  {
    .name = "TAB",
    .val = PICASSO_KEY_TAB,
  },
  {
    .name = "BACKSPACE",
    .val = PICASSO_KEY_BACKSPACE,
  },
  {
    .name = "INSERT",
    .val = PICASSO_KEY_INSERT,
  },
  {
    .name = "DELETE",
    .val = PICASSO_KEY_DELETE,
  },
  {
    .name = "RIGHT",
    .val = PICASSO_KEY_RIGHT,
  },
  {
    .name = "LEFT",
    .val = PICASSO_KEY_LEFT,
  },
  {
    .name = "DOWN",
    .val = PICASSO_KEY_DOWN,
  },
  {
    .name = "UP",
    .val = PICASSO_KEY_UP,
  },
  {
    .name = "PAGE_UP",
    .val = PICASSO_KEY_PAGE_UP,
  },
  {
    .name = "PAGE_DOWN",
    .val = PICASSO_KEY_PAGE_DOWN,
  },
  {
    .name = "HOME",
    .val = PICASSO_KEY_HOME,
  },
  {
    .name = "END",
    .val = PICASSO_KEY_END,
  },
  {
    .name = "CAPS_LOCK",
    .val = PICASSO_KEY_CAPS_LOCK,
  },
  {
    .name = "SCROLL_LOCK",
    .val = PICASSO_KEY_SCROLL_LOCK,
  },
  {
    .name = "NUM_LOCK",
    .val = PICASSO_KEY_NUM_LOCK,
  },
  {
    .name = "PRINT_SCREEN",
    .val = PICASSO_KEY_PRINT_SCREEN,
  },
  {
    .name = "PAUSE",
    .val = PICASSO_KEY_PAUSE,
  },
  {
    .name = "F1",
    .val = PICASSO_KEY_F1,
  },
  {
    .name = "F2",
    .val = PICASSO_KEY_F2,
  },
  {
    .name = "F3",
    .val = PICASSO_KEY_F3,
  },
  {
    .name = "F4",
    .val = PICASSO_KEY_F4,
  },
  {
    .name = "F5",
    .val = PICASSO_KEY_F5,
  },
  {
    .name = "F6",
    .val = PICASSO_KEY_F6,
  },
  {
    .name = "F7",
    .val = PICASSO_KEY_F7,
  },
  {
    .name = "F8",
    .val = PICASSO_KEY_F8,
  },
  {
    .name = "F9",
    .val = PICASSO_KEY_F9,
  },
  {
    .name = "F10",
    .val = PICASSO_KEY_F10,
  },
  {
    .name = "F11",
    .val = PICASSO_KEY_F11,
  },
  {
    .name = "F12",
    .val = PICASSO_KEY_F12,
  },
  {
    .name = "F13",
    .val = PICASSO_KEY_F13,
  },
  {
    .name = "F14",
    .val = PICASSO_KEY_F14,
  },
  {
    .name = "F15",
    .val = PICASSO_KEY_F15,
  },
  {
    .name = "F16",
    .val = PICASSO_KEY_F16,
  },
  {
    .name = "F17",
    .val = PICASSO_KEY_F17,
  },
  {
    .name = "F18",
    .val = PICASSO_KEY_F18,
  },
  {
    .name = "F19",
    .val = PICASSO_KEY_F19,
  },
  {
    .name = "F20",
    .val = PICASSO_KEY_F20,
  },
  {
    .name = "F21",
    .val = PICASSO_KEY_F21,
  },
  {
    .name = "F22",
    .val = PICASSO_KEY_F22,
  },
  {
    .name = "F23",
    .val = PICASSO_KEY_F23,
  },
  {
    .name = "F24",
    .val = PICASSO_KEY_F24,
  },
  {
    .name = "F25",
    .val = PICASSO_KEY_F25,
  },
  {
    .name = "KP0",
    .val = PICASSO_KEY_KP_0,
  },
  {
    .name = "KP1",
    .val = PICASSO_KEY_KP_1,
  },
  {
    .name = "KP2",
    .val = PICASSO_KEY_KP_2,
  },
  {
    .name = "KP3",
    .val = PICASSO_KEY_KP_3,
  },
  {
    .name = "KP4",
    .val = PICASSO_KEY_KP_4,
  },
  {
    .name = "KP5",
    .val = PICASSO_KEY_KP_5,
  },
  {
    .name = "KP6",
    .val = PICASSO_KEY_KP_6,
  },
  {
    .name = "KP7",
    .val = PICASSO_KEY_KP_7,
  },
  {
    .name = "KP8",
    .val = PICASSO_KEY_KP_8,
  },
  {
    .name = "KP9",
    .val = PICASSO_KEY_KP_9,
  },
  {
    .name = "KPDECIMAL",
    .val = PICASSO_KEY_KP_DECIMAL,
  },
  {
    .name = "KPDIVIDE",
    .val = PICASSO_KEY_KP_DIVIDE,
  },
  {
    .name = "KPMULTIPLY",
    .val = PICASSO_KEY_KP_MULTIPLY,
  },
  {
    .name = "KPSUBTRACT",
    .val = PICASSO_KEY_KP_SUBTRACT,
  },
  {
    .name = "KPADD",
    .val = PICASSO_KEY_KP_ADD,
  },
  {
    .name = "KPENTER",
    .val = PICASSO_KEY_KP_ENTER,
  },
  {
    .name = "KPEQUAL",
    .val = PICASSO_KEY_KP_EQUAL,
  },
  {
    .name = NULL,
    .val = 0,
  },
};

Config config_internal = {
  .res_width = 1280,
  .res_height = 720,
  .fullscreen = false,
  .gl_debug = false,
  .frame_lock = 0,
  .ascii_width = 160,
  .ascii_height = 90,
  .grid_size_width = 8,
  .grid_size_height = 8,
};

void config_internal_handle(void *const userdata, const char *section, const char *key, const char *value);

Config *const config_init(void) {
  archivist_read_ini_file("config.ini", &config_internal_handle, &config_internal);
  return &config_internal;
}

Config *const config_get(void) {
  return &config_internal;
}

void config_internal_handle(void *const userdata, const char *section, const char *key, const char *value) {
  Config *const config = (Config *const)userdata;

  // TODO: Break down into section handlers etc
  if (strncmp(section, "window", 1024) == 0) {
    if (strncmp(key, "width", 1024) == 0) {
      int val = atoi(value);
      if (val == 0) {
        return;
      }
      config->res_width = val;
      config->grid_size_width = config->res_width / config->ascii_width;

      printf("Config: Resolution width set to %d, grid size width updated to %d\n", config->res_width, config->grid_size_width);
    } else if (strncmp(key, "height", 1024) == 0) {
      int val = atoi(value);
      if (val == 0) {
        return;
      }
      config->res_height = val;
      config->grid_size_height = config->res_height / config->ascii_height;

      printf("Config: Resolution height set to %d, grid size height updated to %d\n", config->res_height, config->grid_size_height);
    } else if (strncmp(key, "fullscreen", 1024) == 0) {
      int val = atoi(value);
      config->fullscreen = (val > 0 || strncmp(value, "true", 1024) == 0);
      printf("Config: Set fullscreen to %s\n", (config->fullscreen ? "on" : "off"));
    }
  } else if (strncmp(section, "renderer", 1024) == 0) {
    if (strncmp(key, "gl_debug", 1024) == 0) {
      int val = atoi(value);
      config->gl_debug = (val > 0 || strncmp(value, "true", 1024) == 0);
      printf("Config: Set OpenGL debugging to %s\n", (config->gl_debug ? "on" : "off"));
    } else if (strncmp(key, "frame_lock", 1024) == 0) {
      config->frame_lock = atoi(value);
      printf("Config: Frame limit set to %d fps\n", config->frame_lock);
    } else if (strncmp(key, "ascii_width", 1024) == 0) {
      int val = atoi(value);
      if (val == 0) {
        return;
      }
      config->grid_size_width = val;
      config->ascii_width = config->res_width / config->grid_size_width;

      printf("Config: Ascii width set to %d, fit %d characters\n", val, config->ascii_width);
    } else if (strncmp(key, "ascii_height", 1024) == 0) {
      int val = atoi(value);
      if (val == 0) {
        return;
      }
      config->grid_size_height = val;
      config->ascii_height = config->res_height / config->grid_size_height;

      printf("Config: Ascii height set to %d, fit %d characters\n", val, config->ascii_height);
    }
  } else if (strncmp(section, "binds", 1024) == 0) {
    for (uint32_t t = 0; key_names[t].name; t++) {
      if (strncmp(value, key_names[t].name, 1024) == 0) {
        printf("Config: Binding %s to %s\n", key_names[t].name, key);
        RectifyMap *map = rectify_map_create();
        rectify_map_set_uint(map, "key", key_names[t].val);
        rectify_map_set_string(map, "action", (char *)key);
        kronos_post("input", MSG_INPUT_BIND, map, "config");
        break;
      }
    }
  }
}
