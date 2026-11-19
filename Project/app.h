
/*
 *   Header [app.h]
 */

#ifndef APP_HEADER_GUARD
#define APP_HEADER_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define COLOR_RGBA(r, g, b, a) \
((color_t) {                   \
(((float)(r)) / 255.f),        \
(((float)(g)) / 255.f),        \
(((float)(b)) / 255.f),        \
(((float)(a)) / 255.f) })

#define COLOR_WHITE  COLOR_RGBA(255, 255, 255, 255)
#define COLOR_GRAY   COLOR_RGBA(127, 127, 127, 255)
#define COLOR_BLACK  COLOR_RGBA(  0,   0,   0, 255)
#define COLOR_RED    COLOR_RGBA(255,   0,   0, 255)
#define COLOR_GREEN  COLOR_RGBA(  0, 255,   0, 255)
#define COLOR_BLUE   COLOR_RGBA(  0,   0, 255, 255)
#define COLOR_CYAN   COLOR_RGBA(  0, 255, 255, 255)
#define COLOR_PURPLE COLOR_RGBA(255,   0, 255, 255)
#define COLOR_YELLOW COLOR_RGBA(255, 255,   0, 255)
#define COLOR_TRANS  COLOR_RGBA(  0,   0,   0,   0)

typedef struct color_t {
	float r;
	float g;
	float b;
	float a;
} color_t;

typedef struct vertex_t {
	float xpos;
	float ypos;
	color_t color;
} vertex_t;

typedef struct mesh_t mesh_t;

void app_init(const wchar_t* title, int width, int height);
void app_quit(void);
bool app_closed(void);
bool app_minimized(void);
void app_clear(color_t color);
void app_draw(mesh_t* mesh);
void app_render(void);

mesh_t* mesh_create(const vertex_t* vertices, size_t count);
void    mesh_destroy(mesh_t* mesh);

#endif /* APP_HEADER_GUARD */

