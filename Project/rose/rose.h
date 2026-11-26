
/*
 *   Header [rose.h]
 */

#ifndef ROSE_ROSE_HEADER_GUARD
#define ROSE_ROSE_HEADER_GUARD

#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>
#include "rose_math.h"
#include "rose_enums.h"

#define ROSE_CAMERA_DEFAULT       \
((ROSE_Camera) {                  \
.fov = 70.0f,                     \
.near_plane = 0.1f,               \
.far_plane = 100.0f,              \
.position = { 0.0f, 0.0f, 0.0f }, \
.orientation = ROSE_QUAT_IDENTITY })

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

typedef int8_t  Int8;
typedef int16_t Int16;
typedef int32_t Int32;
typedef int64_t Int64;

typedef uint8_t  uInt8;
typedef uint16_t uInt16;
typedef uint32_t uInt32;
typedef uint64_t uInt64;

typedef struct ROSE_Color {
	float r, g, b, a;
} ROSE_Color;

typedef struct ROSE_Vertex2D {
	ROSE_Vec2 position;
	ROSE_Vec2 texcoord;
	ROSE_Color color;
} ROSE_Vertex2D;

typedef struct ROSE_Vertex3D {
	ROSE_Vec3 position;
	ROSE_Color color;
} ROSE_Vertex3D;

typedef struct ROSE_Camera {
	float fov;
	float near_plane;
	float far_plane;
	ROSE_Vec3 position;
	ROSE_Quat orientation;
} ROSE_Camera;

typedef struct ROSE_Image   ROSE_Image;
typedef struct ROSE_Texture ROSE_Texture;

typedef struct ROSE_Mesh2D ROSE_Mesh2D;
typedef struct ROSE_Mesh3D ROSE_Mesh3D;

typedef struct ROSE_Sprite ROSE_Sprite;
typedef struct ROSE_Text   ROSE_Text;

void      ROSE_Init(const wchar_t* title, uInt32 width, uInt32 height);
void      ROSE_Quit(void);
bool      ROSE_WindowClosed(void);
bool      ROSE_WindowMinimized(void);
void      ROSE_WindowVSync(bool vsync);
void      ROSE_WindowMaxFPS(float max);
float     ROSE_WindowFPS(void);
float     ROSE_FrameDelta(void);
double    ROSE_Time(void);
void      ROSE_FrameClear(ROSE_Color color);
void      ROSE_FrameRender(void);

bool      ROSE_MousePressing(ROSE_MouseButton button);
bool      ROSE_MousePressed(ROSE_MouseButton button);
bool      ROSE_MouseReleased(ROSE_MouseButton button);
float     ROSE_MouseScroll(void);
ROSE_Vec2 ROSE_MousePosition(void);
ROSE_Vec2 ROSE_MouseVelocity(void);

bool      ROSE_KeyboardPressing(ROSE_Scancode scancode);
bool      ROSE_KeyboardPressed(ROSE_Scancode scancode);
bool      ROSE_KeyboardReleased(ROSE_Scancode scancode);

ROSE_Image*   ROSE_ImageCreate(uInt32 width, uInt32 height);
void          ROSE_ImageDestroy(ROSE_Image* image);
ROSE_Image*   ROSE_ImageLoad(const char* path);
void          ROSE_ImageSave(ROSE_Image* image, const char* path);
void          ROSE_ImageResize(ROSE_Image* image, uInt32 width, uInt32 height);
void          ROSE_ImageFill(ROSE_Image* image, ROSE_Color color);
ROSE_Color    ROSE_ImageGetPixel(ROSE_Image* image, uInt32 xpos, uInt32 ypos);
void          ROSE_ImageSetPixel(ROSE_Image* image, uInt32 xpos, uInt32 ypos, ROSE_Color color);
void          ROSE_ImageSize(ROSE_Image* image, uInt32* width, uInt32* height);
uInt32*       ROSE_ImageCopyPixels(ROSE_Image* image);

ROSE_Texture* ROSE_TextureCreate(ROSE_Image* image);
void          ROSE_TextureDestroy(ROSE_Texture* texture);

ROSE_Sprite*  ROSE_SpriteCreate(ROSE_Texture* texture);
void          ROSE_SpriteDestroy(ROSE_Sprite* sprite);
void          ROSE_SpriteRegion(ROSE_Sprite* sprite, uInt32 xpos, uInt32 ypos, uInt32 width, uInt32 height);
void          ROSE_SpriteDraw(ROSE_Sprite* sprite, ROSE_Vec2 position, float scale, ROSE_Color color);

ROSE_Text*    ROSE_TextCreate(const char* format, ...);
void          ROSE_TextDestroy(ROSE_Text* text);
void          ROSE_TextUpdate(ROSE_Text* text, const char* format, ...);
void          ROSE_TextDraw(ROSE_Text* text, ROSE_Vec2 position, float scale, ROSE_Color color);

ROSE_Mesh2D*  ROSE_Mesh2DCreate(const ROSE_Vertex2D* vertices, uInt32 count);
void          ROSE_Mesh2DDestroy(ROSE_Mesh2D* mesh);
void          ROSE_Mesh2DUpdate(ROSE_Mesh2D* mesh, const ROSE_Vertex2D* vertices, uInt32 count);
void          ROSE_Mesh2DDraw(ROSE_Mesh2D* mesh, ROSE_Texture* texture);

ROSE_Mesh3D*  ROSE_Mesh3DCreate(const ROSE_Vertex3D* vertices, uInt32 count);
void          ROSE_Mesh3DDestroy(ROSE_Mesh3D* mesh);
void          ROSE_Mesh3DUpdate(ROSE_Mesh3D* mesh, const ROSE_Vertex3D* vertices, uInt32 count);
void          ROSE_Mesh3DDraw(ROSE_Mesh3D* mesh, ROSE_Camera camera, ROSE_Vec3 position, ROSE_Quat orientation, float scale, ROSE_Color color);

#endif /* ROSE_ROSE_HEADER_GUARD */

