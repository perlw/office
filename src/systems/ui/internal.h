#pragma once

typedef struct Widget Widget;

typedef struct {
  char *title;
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  uint32_t handle;

  Widget *widget;

  Surface *surface;
} UIWindow;

typedef void (*WidgetDestroy)(Widget **base_widget);
typedef void (*WidgetEvent)(Widget *const base_widget, uint32_t id, RectifyMap *const map);
typedef void (*WidgetDraw)(Widget *const base_widget, UIWindow *const window);

struct Widget {
  WidgetDestroy destroy;
  WidgetDraw draw;
  WidgetEvent event;
};

#define FOREACH_WIDGET(WIDGET) \
  WIDGET(WIDGET_EVENT_CLICK)

typedef enum {
  FOREACH_WIDGET(GENERATE_ENUM)
} WidgetEvents;

// +RuneSelWidget
typedef struct {
  Widget widget;
  uint32_t chosen_rune;
} RuneSelWidget;

RuneSelWidget *runesel_widget_create(void);
void runesel_widget_destroy(Widget **base_widget);
void runesel_widget_draw(Widget *const base_widget, UIWindow *const window);
void runesel_widget_event(Widget *const base_widget, uint32_t id, RectifyMap *const map);
// -RuneSelWidget

// +ColSelWidget
typedef struct {
  Widget widget;
  uint32_t chosen_color;
} ColSelWidget;

ColSelWidget *colsel_widget_create(void);
void colsel_widget_destroy(Widget **base_widget);
void colsel_widget_draw(Widget *const base_widget, UIWindow *const window);
void colsel_widget_event(Widget *const base_widget, uint32_t id, RectifyMap *const map);
// -ColSelWidget

// +TileSelWidget
typedef struct {
  char *id;
  Glyph glyph;
  bool collides;
} TileDef;

typedef struct {
  Widget widget;
  uint32_t chosen_tile;
  TileDef *tiledefs;
} TileSelWidget;

TileSelWidget *tilesel_widget_create(void);
void tilesel_widget_destroy(Widget **base_widget);
void tilesel_widget_draw(Widget *const base_widget, UIWindow *const window);
void tilesel_widget_event(Widget *const base_widget, uint32_t id, RectifyMap *const map);
// -TileSelWidget
