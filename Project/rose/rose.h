
/*
 *   Header [rose.h]
 */

#ifndef ROSE_ROSE_HEADER_GUARD
#define ROSE_ROSE_HEADER_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define ROSE_COLOR_RGBA(r, g, b, a) \
((ROSE_Color) {                     \
(((float)(r)) / 255.f),             \
(((float)(g)) / 255.f),             \
(((float)(b)) / 255.f),             \
(((float)(a)) / 255.f) })

#define ROSE_COLOR_WHITE  ROSE_COLOR_RGBA(255, 255, 255, 255)
#define ROSE_COLOR_GRAY   ROSE_COLOR_RGBA(127, 127, 127, 255)
#define ROSE_COLOR_BLACK  ROSE_COLOR_RGBA(  0,   0,   0, 255)
#define ROSE_COLOR_RED    ROSE_COLOR_RGBA(255,   0,   0, 255)
#define ROSE_COLOR_GREEN  ROSE_COLOR_RGBA(  0, 255,   0, 255)
#define ROSE_COLOR_BLUE   ROSE_COLOR_RGBA(  0,   0, 255, 255)
#define ROSE_COLOR_CYAN   ROSE_COLOR_RGBA(  0, 255, 255, 255)
#define ROSE_COLOR_PURPLE ROSE_COLOR_RGBA(255,   0, 255, 255)
#define ROSE_COLOR_YELLOW ROSE_COLOR_RGBA(255, 255,   0, 255)
#define ROSE_COLOR_TRANS  ROSE_COLOR_RGBA(  0,   0,   0,   0)

#ifdef TRUE
  #undef TRUE
#endif /* TRUE */

#ifdef FALSE
  #undef FALSE
#endif /* FALSE */

#define TRUE  ((_Bool) 1u)
#define FALSE ((_Bool) 0u)

typedef int8_t  Int8;
typedef int16_t Int16;
typedef int32_t Int32;
typedef int64_t Int64;

typedef uint8_t  uInt8;
typedef uint16_t uInt16;
typedef uint32_t uInt32;
typedef uint64_t uInt64;

typedef struct ROSE_Color {
	float r;
	float g;
	float b;
	float a;
} ROSE_Color;

typedef struct ROSE_Vertex2D {
	float xpos;
	float ypos;
	ROSE_Color color;
} ROSE_Vertex2D;

typedef struct ROSE_Image ROSE_Image;
typedef struct ROSE_Mesh2D ROSE_Mesh2D;

void ROSE_Init(const wchar_t* title, uInt32 width, uInt32 height);
void ROSE_Quit(void);

bool ROSE_WindowClosed(void);
bool ROSE_WindowMinimized(void);
void ROSE_FrameClear(ROSE_Color color);
void ROSE_FrameRender(void);

ROSE_Image*  ROSE_ImageCreate(uInt32 width, uInt32 height);
void         ROSE_ImageDestroy(ROSE_Image* image);
ROSE_Image*  ROSE_ImageLoad(const char* path);
void         ROSE_ImageSave(ROSE_Image* image, const char* path);
void         ROSE_ImageResize(ROSE_Image* image, uInt32 width, uInt32 height);
void         ROSE_ImageFillColor(ROSE_Image* image, ROSE_Color color);
ROSE_Color   ROSE_ImageGetPixel(ROSE_Image* image, uInt32 xpos, uInt32 ypos);
void         ROSE_ImageSetPixel(ROSE_Image* image, uInt32 xpos, uInt32 ypos, ROSE_Color color);
void         ROSE_ImageSize(ROSE_Image* image, uInt32* width, uInt32* height);
uInt32*      ROSE_ImageCopyPixels(ROSE_Image* image);

ROSE_Mesh2D* ROSE_MeshCreate(const ROSE_Vertex2D* vertices, uInt32 count);
void         ROSE_MeshDestroy(ROSE_Mesh2D* mesh);
void         ROSE_MeshUpdate(ROSE_Mesh2D* mesh, const ROSE_Vertex2D* vertices, uInt32 count);
void         ROSE_MeshDraw(ROSE_Mesh2D* mesh);

#endif /* ROSE_ROSE_HEADER_GUARD */

