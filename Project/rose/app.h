
/*
 *   Header [app.h]
 */

#ifndef APP_HEADER_GUARD
#define APP_HEADER_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef NDEBUG
  #ifndef DEBUG
    #define DEBUG
  #endif /* DEBUG */
#else
  #ifndef RELEASE
    #define RELEASE
  #endif /* RELEASE */
#endif

#ifdef DEBUG
  #ifdef RELEASE
    #warning("D/R CONFLICT")
    #undef RELEASE
  #endif /* RELEASE */
#endif /* DEBUG */

#ifndef ASSERT_PANIC
  #ifdef RELEASE
    #ifndef EXIT_TYPE
      #define EXIT_TYPE exit(EXIT_FAILURE);
    #endif /* EXIT_TYPE */
  #endif /* RELEASE */
  #ifdef DEBUG
    #ifndef EXIT_TYPE
      #define EXIT_TYPE __debugbreak();
    #endif /* EXIT_TYPE */
  #endif /* DEBUG */
  #define ASSERT_PANIC(x, ...)                   \
  do { if (!(x)) {                               \
    fprintf(stderr, ">> PANIC!!!\n");            \
    fprintf(stderr, ">> FILE: %s\n",             \
    (strrchr(__FILE__, '\\')) ?                  \
    (strrchr(__FILE__, '\\') + 1) : (__FILE__)); \
    fprintf(stderr, ">> LINE: %d\n", __LINE__);  \
    fprintf(stderr, ">> ERROR: ");               \
    fprintf(stderr, __VA_ARGS__);                \
    fprintf(stderr, "\n");                       \
    EXIT_TYPE                                    \
  } } while (0)
#endif /* ASSERT_PANIC */

#ifndef CHECK_HRESULT
  #define CHECK_HRESULT(hr, fn)      \
  do {                               \
    ASSERT_PANIC(!(FAILED(hr)),      \
    "%s failed! (HRESULT: 0x%08lX)", \
    fn, (unsigned long)(hr));        \
  } while (0)
#endif /* CHECK_HRESULT */

#define COLOR_RGBA(r, g, b, a) \
((color_t) {                   \
(((float)(r)) / 255.f),        \
(((float)(g)) / 255.f),        \
(((float)(b)) / 255.f),        \
(((float)(a)) / 255.f) })

#define ROSE_COLOR_WHITE  COLOR_RGBA(255, 255, 255, 255)
#define ROSE_COLOR_GRAY   COLOR_RGBA(127, 127, 127, 255)
#define ROSE_COLOR_BLACK  COLOR_RGBA(  0,   0,   0, 255)
#define ROSE_COLOR_RED    COLOR_RGBA(255,   0,   0, 255)
#define ROSE_COLOR_GREEN  COLOR_RGBA(  0, 255,   0, 255)
#define ROSE_COLOR_BLUE   COLOR_RGBA(  0,   0, 255, 255)
#define ROSE_COLOR_CYAN   COLOR_RGBA(  0, 255, 255, 255)
#define ROSE_COLOR_PURPLE COLOR_RGBA(255,   0, 255, 255)
#define ROSE_COLOR_YELLOW COLOR_RGBA(255, 255,   0, 255)
#define ROSE_COLOR_TRANS  COLOR_RGBA(  0,   0,   0,   0)

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

typedef struct ROSE_Mesh ROSE_Mesh;

void ROSE_Init(const wchar_t* title, int width, int height);
void ROSE_Quit(void);

bool ROSE_WindowClosed(void);
bool ROSE_WindowMinimized(void);
void ROSE_FrameClear(color_t color);
void ROSE_FrameRender(void);

ROSE_Mesh* ROSE_MeshCreate(const vertex_t* vertices, size_t count);
void       ROSE_MeshDestroy(ROSE_Mesh* mesh);
void       ROSE_MeshUpdate(ROSE_Mesh* mesh, const vertex_t* vertices, size_t count);
void       ROSE_MeshDraw(ROSE_Mesh* mesh);

#endif /* APP_HEADER_GUARD */

