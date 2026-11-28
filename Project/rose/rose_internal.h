
/*
 *   Header [rose_internal.h]
 */

#ifndef ROSE_INTERNAL_HEADER_GUARD
#define ROSE_INTERNAL_HEADER_GUARD

#include "rose.h"
#include "png.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <dxgi1_5.h>
#include <stddef.h>
#include <string.h>
#include <d3dcompiler.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define WINDOW_CLASS_NAME L"WindowClassNameDirect3D11"

#ifndef NDEBUG
  #ifndef ROSE_DEBUG
    #define ROSE_DEBUG
  #endif
#else
  #ifndef ROSE_RELEASE
    #define ROSE_RELEASE
  #endif
#endif

#ifndef CHECK_HRESULT
  #define CHECK_HRESULT(result, func)                  \
  do { if (FAILED(result)) {                           \
    fprintf(stderr, "%s failed! (HRESULT: 0x%08lX)\n", \
    func, (unsigned long)(result)); abort();           \
  } } while (0)
#endif

typedef struct rose__constants_shape_t {
	ROSE_Vec2 screen_size;
	ROSE_Vec2 offset;
	ROSE_Color tint;
	float scale;
	float _padding[3];
} rose__constants_shape_t;

typedef struct rose__constants_mesh_t {
	ROSE_Matrix mvp;
	ROSE_Color tint;
} rose__constants_mesh_t;

typedef struct rose__constants_points_t {
	ROSE_Vec2 screen_size;
	float diameter;
	float _padding;
} rose__constants_points_t;

typedef struct rose__mouse_t {
	bool pressing[ROSE_MOUSE_COUNT];
	bool pressed[ROSE_MOUSE_COUNT];
	bool released[ROSE_MOUSE_COUNT];
	ROSE_Vec2 curr_pos;
	ROSE_Vec2 prev_pos;
	ROSE_Vec2 raw_delta;
	ROSE_Vec2 hidden_pos;
	bool hidden;
	float scroll;
} rose__mouse_t;

typedef struct rose__keyboard_t {
	bool pressing[ROSE_SCANCODE_COUNT];
	bool pressed[ROSE_SCANCODE_COUNT];
	bool released[ROSE_SCANCODE_COUNT];
} rose__keyboard_t;

#define ROSE_GLOBAL(type, name) \
extern type name;
#ifdef ROSE_DEFINE_GLOBALS
  #undef ROSE_GLOBAL
  #define ROSE_GLOBAL(type, name) \
  type name = { 0 };
#endif /* ROSE_DEFINE_GLOBALS */

ROSE_GLOBAL(bool,                    rose__quit)
ROSE_GLOBAL(HINSTANCE,               rose__hinstance)
ROSE_GLOBAL(HWND,                    rose__hwnd)
ROSE_GLOBAL(ID3D11Device*,           rose__device)
ROSE_GLOBAL(ID3D11DeviceContext*,    rose__context)
ROSE_GLOBAL(IDXGISwapChain1*,        rose__swapchain)
ROSE_GLOBAL(ID3D11Buffer*,           rose__constant_buffer_shape)
ROSE_GLOBAL(ID3D11InputLayout*,      rose__input_layout_shape)
ROSE_GLOBAL(ID3D11RenderTargetView*, rose__render_target)
ROSE_GLOBAL(D3D11_VIEWPORT,          rose__viewport)
ROSE_GLOBAL(ID3D11VertexShader*,     rose__vertex_shader_shape)
ROSE_GLOBAL(ID3D11PixelShader*,      rose__pixel_shader_shape)
ROSE_GLOBAL(ID3D11SamplerState*,     rose__sampler)
ROSE_GLOBAL(ID3D11BlendState*,       rose__blend_state)
ROSE_GLOBAL(ID3D11RasterizerState*,  rose__raster_shape)

ROSE_GLOBAL(ROSE_Texture*, rose__blank_texture)
ROSE_GLOBAL(ROSE_Texture*, rose__ascii_texture)

ROSE_GLOBAL(bool,   rose__resize_pending)
ROSE_GLOBAL(uInt32, rose__screen_width)
ROSE_GLOBAL(uInt32, rose__screen_height)

ROSE_GLOBAL(HANDLE, rose__timer)
ROSE_GLOBAL(Int64,  rose__qpc_frequency)
ROSE_GLOBAL(Int64,  rose__time_start)
ROSE_GLOBAL(bool,   rose__vsync)
ROSE_GLOBAL(bool,   rose__tearing)
ROSE_GLOBAL(float,  rose__max_fps)
ROSE_GLOBAL(Int64,  rose__frame_next)
ROSE_GLOBAL(Int64,  rose__frame_last)
ROSE_GLOBAL(float,  rose__frame_delta)
ROSE_GLOBAL(float,  rose__fps)
ROSE_GLOBAL(uInt32, rose__fps_frames)
ROSE_GLOBAL(Int64,  rose__fps_window_start)

ROSE_GLOBAL(rose__mouse_t,    rose__mouse)
ROSE_GLOBAL(rose__keyboard_t, rose__keyboard)

ROSE_GLOBAL(ID3D11DepthStencilView*,  rose__depth_view)
ROSE_GLOBAL(ID3D11DepthStencilState*, rose__depth_on)
ROSE_GLOBAL(ID3D11DepthStencilState*, rose__depth_off)

ROSE_GLOBAL(ID3D11VertexShader*, rose__vertex_shader_mesh)
ROSE_GLOBAL(ID3D11PixelShader*,  rose__pixel_shader_mesh)
ROSE_GLOBAL(ID3D11InputLayout*,  rose__input_layout_mesh)
ROSE_GLOBAL(ID3D11Buffer*,       rose__constant_buffer_mesh)

ROSE_GLOBAL(ID3D11GeometryShader*, rose__geometry_shader_points)
ROSE_GLOBAL(ID3D11Buffer*,         rose__constant_buffer_points)

typedef struct ROSE_Image {
	uInt32 width;
	uInt32 height;
	uInt32* pixels;
} ROSE_Image;

typedef struct ROSE_Texture {
	uInt32 width;
	uInt32 height;
	ID3D11ShaderResourceView* view;
} ROSE_Texture;

typedef struct ROSE_Shape {
	uInt32 count;
	uInt32 capacity;
	ID3D11Buffer* vertices;
} ROSE_Shape;

typedef struct ROSE_Mesh {
	uInt32 count;
	uInt32 capacity;
	ID3D11Buffer* vertices;
} ROSE_Mesh;

typedef struct ROSE_Sprite {
	float width;
	float height;
	float u0, v0;
	float u1, v1;
	ROSE_Texture* texture;
} ROSE_Sprite;

typedef struct ROSE_Text {
	char* string;
	uInt32 count;
	uInt32 capacity;
	ROSE_VertexShape* vertices;
	uInt32 vertex_count;
} ROSE_Text;

LRESULT CALLBACK  ROSE__WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
ID3DBlob*         ROSE__CompileShader(const wchar_t* path, const char* entry, const char* target);
void              ROSE__CreateRenderTarget(void);
void              ROSE__BindPipelineShape(ID3D11Buffer* vertices, ROSE_Texture* texture, ROSE_Vec2 position, float scale, ROSE_Color color, ROSE_PRIMITIVE primitive, float diameter);
void              ROSE__BindPrimitive(ROSE_PRIMITIVE primitive, float diameter);
Int64             ROSE__Ticks(void);
ROSE_VertexShape* ROSE__BatchAllocate(ROSE_Texture* texture, uInt32 count);
void              ROSE__BatchCreate(void);
void              ROSE__BatchDestroy(void);
void              ROSE__BatchFlush(void);

void ROSE__MouseClear(void);
void ROSE__MouseButton(ROSE_MouseButton button, bool pressing);
void ROSE__MouseRelease(void);
void ROSE__MouseRawInput(LPARAM lparam);

void ROSE__KeyboardClear(void);
void ROSE__KeyboardMessage(UINT msg, WPARAM wparam, LPARAM lparam);
void ROSE__KeyboardRelease(void);

#endif /* ROSE_INTERNAL_HEADER_GUARD */

