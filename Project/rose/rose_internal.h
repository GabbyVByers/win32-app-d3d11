
/*
 *   Header [rose_internal.h]
 */

#ifndef ROSE_INTERNAL_HEADER_GUARD
#define ROSE_INTERNAL_HEADER_GUARD

#include "rose.h"
#include "png.h"

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

#ifndef ROSE_ASSERT
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
  #define ROSE_ASSERT(x, ...)                    \
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
#endif /* ROSE_ASSERT */

#ifndef CHECK_HRESULT
  #define CHECK_HRESULT(hr, fn)      \
  do {                               \
    ROSE_ASSERT(!(FAILED(hr)),       \
    "%s failed! (HRESULT: 0x%08lX)", \
    fn, (unsigned long)(hr));        \
  } while (0)
#endif /* CHECK_HRESULT */

typedef struct constants_t {
	float screen_size[2];
	float _padding[2];
} constants_t;

typedef struct ROSE_Image {
    uInt32 width;
    uInt32 height;
    uInt32* pixels;
} ROSE_Image;

typedef struct ROSE_Mesh {
    ID3D11Buffer* vertices;
    uInt32 count;
} ROSE_Mesh;

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

ROSE_GLOBAL(bool,   rose__resize_pending)
ROSE_GLOBAL(uInt32, rose__screen_width)
ROSE_GLOBAL(uInt32, rose__screen_height)

LRESULT CALLBACK proc(HWND, UINT, WPARAM, LPARAM);
ID3DBlob* compile_shader(const wchar_t* path, const char* entry, const char* target);
void create_render_target(void);

#endif /* ROSE_INTERNAL_HEADER_GUARD */

