typedef void (*MuseFunc)(void);

typedef struct {
  lua_State* state;
  uint8_t instance_id;
  MuseFunc funcs[256];
} Muse;

