
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

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
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

#ifdef ROSE_DEBUG
  #ifdef ROSE_RELEASE
    #warning("D/R CONFLICT")
    #undef ROSE_RELEASE
  #endif
#endif

#ifndef CHECK_HRESULT
  #define CHECK_HRESULT(result, func)                  \
  do { if (FAILED(result)) {                           \
    fprintf(stderr, "%s failed! (HRESULT: 0x%08lX)\n", \
    func, (unsigned long)(result)); abort();           \
  } } while (0)
#endif

typedef struct constants_t {
	float screen_size[2];
	float _padding[2];
} constants_t;

typedef struct ROSE_Image {
    uInt32 width;
    uInt32 height;
    uInt32* pixels;
} ROSE_Image;

typedef struct ROSE_Texture {
    ID3D11ShaderResourceView* view;
    uInt32 width;
    uInt32 height;
} ROSE_Texture;

typedef struct ROSE_Mesh2D {
    ID3D11Buffer* vertices;
    uInt32 count;
} ROSE_Mesh2D;

#define ROSE_GLOBAL(type, name) \
type name = { 0 };
#ifdef ROSE_EXTERN
  #undef  ROSE_GLOBAL
  #define ROSE_GLOBAL(type, name) \
  extern type name;
#endif /* ROSE_EXTERN */

ROSE_GLOBAL(bool,      rose__quit)
ROSE_GLOBAL(HINSTANCE, rose__hinstance)
ROSE_GLOBAL(HWND,      rose__hwnd)

ROSE_GLOBAL(ID3D11Device*,           rose__device)
ROSE_GLOBAL(ID3D11DeviceContext*,    rose__context)
ROSE_GLOBAL(IDXGISwapChain1*,        rose__swapchain)
ROSE_GLOBAL(ID3D11Buffer*,           rose__constant_buffer)
ROSE_GLOBAL(ID3D11InputLayout*,      rose__input_layout)
ROSE_GLOBAL(ID3D11RenderTargetView*, rose__render_target)
ROSE_GLOBAL(D3D11_VIEWPORT,          rose__viewport)

ROSE_GLOBAL(ID3D11VertexShader*, rose__vertex_shader)
ROSE_GLOBAL(ID3D11PixelShader*,  rose__pixel_shader)

ROSE_GLOBAL(ID3D11SamplerState*, rose__sampler)
ROSE_GLOBAL(ID3D11BlendState*,   rose__blend_state)
ROSE_GLOBAL(ROSE_Texture*,       rose__blank_texture)

ROSE_GLOBAL(bool,   rose__resize_pending)
ROSE_GLOBAL(uInt32, rose__screen_width)
ROSE_GLOBAL(uInt32, rose__screen_height)

LRESULT CALLBACK rose__proc(HWND, UINT, WPARAM, LPARAM);
ID3DBlob* rose__compile_shader(const wchar_t* path, const char* entry, const char* target);
void rose__create_render_target(void);

#endif /* ROSE_INTERNAL_HEADER_GUARD */

