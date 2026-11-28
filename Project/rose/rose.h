
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

typedef int8_t  Int8;
typedef int16_t Int16;
typedef int32_t Int32;
typedef int64_t Int64;

typedef uint8_t  uInt8;
typedef uint16_t uInt16;
typedef uint32_t uInt32;
typedef uint64_t uInt64;

typedef struct ROSE_Image   ROSE_Image;
typedef struct ROSE_Texture ROSE_Texture;
typedef struct ROSE_Sprite  ROSE_Sprite;
typedef struct ROSE_Text    ROSE_Text;
typedef struct ROSE_Shape   ROSE_Shape;
typedef struct ROSE_Mesh    ROSE_Mesh;
typedef struct ROSE_Slider  ROSE_Slider;

typedef struct ROSE_Color {
	float r;
	float g;
	float b;
	float a;
} ROSE_Color;

typedef struct ROSE_VertexShape {
	ROSE_Vec2  position;
	ROSE_Vec2  texcoord;
	ROSE_Color color;
} ROSE_VertexShape;

typedef struct ROSE_VertexMesh {
	ROSE_Vec3  position;
	ROSE_Vec2  texcoord;
	ROSE_Color color;
} ROSE_VertexMesh;

#define ROSE_COLOR_RGBA(r, g, b, a) \
((ROSE_Color) {                     \
  (((float)(r)) / 255.f),           \
  (((float)(g)) / 255.f),           \
  (((float)(b)) / 255.f),           \
  (((float)(a)) / 255.f),           \
})

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

typedef enum ROSE_PRIMITIVE {
	ROSE_TRIANGLES,
	ROSE_LINES,
	ROSE_POINTS,
} ROSE_PRIMITIVE;

typedef struct ROSE_Camera {
	float     fov_degrees;
	float     near_plane;
	float     far_plane;
	ROSE_Vec3 position;
	ROSE_Quat orientation;
} ROSE_Camera;

typedef struct ROSE_DrawInfo {
	ROSE_Vec2  position;
	float      scale;
	ROSE_Color color;
} ROSE_DrawInfo;

typedef struct ROSE_ShapeDrawInfo {
	ROSE_Vec2      position;
	float          scale;
	ROSE_Color     color;
	ROSE_Texture*  texture;
	ROSE_PRIMITIVE primitive;
	float          point_diameter;
} ROSE_ShapeDrawInfo;

typedef struct ROSE_MeshDrawInfo {
	ROSE_Vec3      position;
	float          scale;
	ROSE_Color     color;
	ROSE_Texture*  texture;
	ROSE_PRIMITIVE primitive;
	float          point_diameter;
	ROSE_Quat      orientation;
} ROSE_MeshDrawInfo;

#define ROSE_CAMERA_DEFAULT             \
((ROSE_Camera) {                        \
  .fov_degrees = 70.0f,                 \
  .near_plane  = 0.1f,                  \
  .far_plane   = 100.0f,                \
  .position    = ROSE_VEC3_ZERO,        \
  .orientation = ROSE_QUAT_IDENTITY,    \
})

#define ROSE_DRAW_DEFAULT               \
((ROSE_DrawInfo) {                      \
  .position    = ROSE_VEC2_ZERO,        \
  .scale       = 1.0f,                  \
  .color       = ROSE_COLOR_WHITE,      \
})

#define ROSE_SHAPE_DRAW_DEFAULT         \
((ROSE_ShapeDrawInfo) {                 \
  .position       = ROSE_VEC2_ZERO,     \
  .scale          = 1.0f,               \
  .color          = ROSE_COLOR_WHITE,   \
  .texture        = NULL,               \
  .primitive      = ROSE_TRIANGLES,     \
  .point_diameter = 1.0f,               \
})

#define ROSE_MESH_DRAW_DEFAULT          \
((ROSE_MeshDrawInfo) {                  \
  .position       = ROSE_VEC3_ZERO,     \
  .scale          = 1.0f,               \
  .color          = ROSE_COLOR_WHITE,   \
  .texture        = NULL,               \
  .primitive      = ROSE_TRIANGLES,     \
  .point_diameter = 1.0f,               \
  .orientation    = ROSE_QUAT_IDENTITY, \
})

void   ROSE_Init(const wchar_t* title, uInt32 width, uInt32 height);
void   ROSE_Quit(void);
bool   ROSE_WindowClosed(void);
bool   ROSE_WindowMinimized(void);
void   ROSE_WindowVSync(bool vsync);
void   ROSE_WindowMaxFPS(float max);
float  ROSE_WindowFPS(void);
float  ROSE_FrameDelta(void);
double ROSE_Time(void);
void   ROSE_FrameClear(ROSE_Color color);
void   ROSE_FrameRender(void);

bool      ROSE_MousePressing(ROSE_MouseButton button);
bool      ROSE_MousePressed(ROSE_MouseButton button);
bool      ROSE_MouseReleased(ROSE_MouseButton button);
float     ROSE_MouseScroll(void);
ROSE_Vec2 ROSE_MousePosition(void);
ROSE_Vec2 ROSE_MouseVelocity(void);
void      ROSE_MouseCursorHide(void);
void      ROSE_MouseCursorReveal(void);

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
void          ROSE_SpriteDraw(ROSE_Sprite* sprite, ROSE_DrawInfo draw);

ROSE_Text*    ROSE_TextCreate(const char* format, ...);
void          ROSE_TextDestroy(ROSE_Text* text);
void          ROSE_TextUpdate(ROSE_Text* text, const char* format, ...);
void          ROSE_TextDraw(ROSE_Text* text, ROSE_DrawInfo draw);

ROSE_Shape*   ROSE_ShapeCreate(const ROSE_VertexShape* vertices, uInt32 count);
void          ROSE_ShapeDestroy(ROSE_Shape* shape);
void          ROSE_ShapeUpdate(ROSE_Shape* shape, const ROSE_VertexShape* vertices, uInt32 count);
void          ROSE_ShapeDraw(ROSE_Shape* shape, ROSE_ShapeDrawInfo draw);

ROSE_Mesh*    ROSE_MeshCreate(const ROSE_VertexMesh* vertices, uInt32 count);
void          ROSE_MeshDestroy(ROSE_Mesh* mesh);
void          ROSE_MeshUpdate(ROSE_Mesh* mesh, const ROSE_VertexMesh* vertices, uInt32 count);
void          ROSE_MeshDraw(ROSE_Mesh* mesh, ROSE_MeshDrawInfo draw, ROSE_Camera camera);

ROSE_Slider*  ROSE_SliderCreate(ROSE_Vec2 dimensions, float min, float max, float* value);
void          ROSE_SliderDestroy(ROSE_Slider* slider);
bool          ROSE_SliderDraw(ROSE_Slider* slider, ROSE_DrawInfo draw);


#endif /* ROSE_ROSE_HEADER_GUARD */

