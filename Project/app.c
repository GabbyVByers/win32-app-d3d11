
/*
 *   Source [app.c]
 */

#include "app.h"

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

#ifndef ASSERT_PANIC
  #ifdef RELEASE
    #ifndef EXIT_TYPE
      #define EXIT_TYPE exit(EXIT_FAILURE);
    #endif /* EXIT_TYPE */
  #endif /* RELEASE */
  #ifdef DEBUG
    #ifndef EXIT_TYPE
      #define EXIT_TYPE __debugbreak(); abort();
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

typedef struct constants_t {
	float screen_size[2];
	float _padding[2];
} constants_t;

typedef struct mesh_t {
	ID3D11Buffer* vertex_buffer;
	size_t vertex_count;
} mesh_t;

static bool g_active_window = false;
static bool g_active_frame  = false;

static HINSTANCE g_hinstance = NULL;
static HWND      g_hwnd      = NULL;

static ID3D11Device*           g_device          = NULL;
static ID3D11DeviceContext*    g_context         = NULL;
static IDXGISwapChain1*        g_swapchain       = NULL;
static ID3D11Buffer*           g_constant_buffer = NULL;
static ID3D11InputLayout*      g_input_layout    = NULL;
static ID3D11RenderTargetView* g_render_target   = NULL;
static D3D11_VIEWPORT          g_viewport = { 0 };

static ID3D11VertexShader* g_vertex_shader = NULL;
static ID3D11PixelShader*  g_pixel_shader  = NULL;

static bool g_resize_pending = false;
static UINT g_screen_width   = 0u;
static UINT g_screen_height  = 0u;

static LRESULT CALLBACK proc(HWND, UINT, WPARAM, LPARAM);
static ID3DBlob* compile_shader(const wchar_t*, const char*, const char*);
static void create_render_target(void);

/*
 *   ------------ APP -------------
 */

void app_init(const wchar_t* title, int width, int height)
{
	/*
	 *   Window
	 */

	ASSERT_PANIC(!g_active_window, "app_init() failed! (already init)");
	g_active_window = true;

	g_hinstance = GetModuleHandleW(NULL);
	WNDCLASSEXW window_class = {
		.cbSize        = sizeof(WNDCLASSEXW),
		.style         = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc   = proc,
		.hInstance     = g_hinstance,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.lpszClassName = WINDOW_CLASS_NAME,
	};

	ATOM atom = RegisterClassExW(&window_class);
	DWORD error = GetLastError();
	ASSERT_PANIC(atom, "RegisterClassExW() failed! (error: %lu)", error);

	RECT rect = { 0, 0, (long)width, (long)height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	width = (int)(rect.right - rect.left);
	height = (int)(rect.bottom - rect.top);

	g_hwnd = CreateWindowExW(0,
		WINDOW_CLASS_NAME, title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width, height,
		NULL, NULL,
		g_hinstance, NULL
	);

	error = GetLastError();
	ASSERT_PANIC(g_hwnd, "CreateWindowExW() failed! (error: %lu)", error);

	ShowWindow(g_hwnd, SW_SHOWDEFAULT);

	/*
	 *   Device
	 */

	UINT flags = 0u;
	#ifdef DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif /* DEBUG */

	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };

	HRESULT result = D3D11CreateDevice(
		NULL, D3D_DRIVER_TYPE_HARDWARE,
		NULL, flags,
		levels, ARRAYSIZE(levels),
		D3D11_SDK_VERSION,
		&g_device, NULL, &g_context
	);

	CHECK_HRESULT(result, "D3D11CreateDevice()");

	#ifdef DEBUG
	ID3D11InfoQueue* info = NULL;
	if (SUCCEEDED(ID3D11Device_QueryInterface(g_device, &IID_ID3D11InfoQueue, (void**)&info))) {
		ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
		ID3D11InfoQueue_Release(info); }
	#endif /* DEBUG */
	
	/*
	 *   Swap Chain
	 */

	IDXGIDevice*   dxgi_device = NULL;
	IDXGIAdapter*  adapter     = NULL;
	IDXGIFactory2* factory     = NULL;
	
	result = ID3D11Device_QueryInterface(g_device, &IID_IDXGIDevice, (void**)&dxgi_device);
	CHECK_HRESULT(result, "ID3D11Device_QueryInterface()");

	result = IDXGIDevice_GetAdapter(dxgi_device, &adapter);
	CHECK_HRESULT(result, "IDXGIDevice_GetAdapter()");

	result = IDXGIAdapter_GetParent(adapter, &IID_IDXGIFactory2, (void**)&factory);
	CHECK_HRESULT(result, "IDXGIAdapter_GetParent()");

	#ifdef DEBUG
	DXGI_ADAPTER_DESC description;
	IDXGIAdapter_GetDesc(adapter, &description);
	const wchar_t* device_name = description.Description;
	fprintf(stdout, "Using GPU Device: (%ls)\n", device_name);
	#endif /* DEBUG */

	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {
		.Width       = 0,
		.Height      = 0,
		.Format      = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc  = { .Count = 1, .Quality = 0 },
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = 2,
		.Scaling     = DXGI_SCALING_NONE,
		.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED,
	};

	result = IDXGIFactory2_CreateSwapChainForHwnd(factory, (IUnknown*)g_device, g_hwnd, &swapchain_desc, NULL, NULL, &g_swapchain);
	CHECK_HRESULT(result, "IDXGIFactory2_CreateSwapChainForHwnd()");

	IDXGIFactory2_MakeWindowAssociation(factory, g_hwnd, DXGI_MWA_NO_ALT_ENTER);

	IDXGIFactory2_Release(factory);
	IDXGIAdapter_Release(adapter);
	IDXGIDevice_Release(dxgi_device);
	
	/*
	 *   Shaders
	 */

	create_render_target();
	ID3DBlob* vs_blob = compile_shader(L"HLSL/shader.hlsl", "vs_main", "vs_5_0");
	ID3DBlob* ps_blob = compile_shader(L"HLSL/shader.hlsl", "ps_main", "ps_5_0");

	result = ID3D11Device_CreateVertexShader(g_device, ID3D10Blob_GetBufferPointer(vs_blob), ID3D10Blob_GetBufferSize(vs_blob), NULL, &g_vertex_shader);
	CHECK_HRESULT(result, "ID3D11Device_CreateVertexShader()");

	result = ID3D11Device_CreatePixelShader(g_device, ID3D10Blob_GetBufferPointer(ps_blob), ID3D10Blob_GetBufferSize(ps_blob), NULL, &g_pixel_shader);
	CHECK_HRESULT(result, "ID3D11Device_CreatePixelShader()");

	D3D11_INPUT_ELEMENT_DESC input_desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(vertex_t, xpos),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(vertex_t, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	result = ID3D11Device_CreateInputLayout(
		g_device, input_desc,
		ARRAYSIZE(input_desc),
		ID3D10Blob_GetBufferPointer(vs_blob),
		ID3D10Blob_GetBufferSize(vs_blob),
		&g_input_layout
	);

	CHECK_HRESULT(result, "ID3D11Device_CreateInputLayout()");

	ID3D10Blob_Release(vs_blob);
	ID3D10Blob_Release(ps_blob);

	D3D11_BUFFER_DESC constant_buffer_desc = {
		.ByteWidth = sizeof(constants_t),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
	};

	result = ID3D11Device_CreateBuffer(g_device, &constant_buffer_desc, NULL, &g_constant_buffer);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");
}

void app_quit(void)
{
	ASSERT_PANIC(g_active_window, "app_quit() failed! (quit called before init)");
	g_active_window = false;

	ID3D11DeviceContext_ClearState(g_context);

	ID3D11Buffer_Release(g_constant_buffer);
	ID3D11InputLayout_Release(g_input_layout);
	ID3D11PixelShader_Release(g_pixel_shader);
	ID3D11VertexShader_Release(g_vertex_shader);
	ID3D11RenderTargetView_Release(g_render_target);
	IDXGISwapChain1_Release(g_swapchain);
	ID3D11DeviceContext_Release(g_context);
	ID3D11Device_Release(g_device);

	g_constant_buffer = NULL;
	g_input_layout    = NULL;
	g_pixel_shader    = NULL;
	g_vertex_shader   = NULL;
	g_render_target   = NULL;
	g_swapchain = NULL;
	g_context   = NULL;
	g_device    = NULL;
	g_viewport  = (D3D11_VIEWPORT) { 0 };

	if (g_hwnd && IsWindow(g_hwnd))
		DestroyWindow(g_hwnd);

	UnregisterClassW(WINDOW_CLASS_NAME, g_hinstance);

	g_hwnd      = NULL;
	g_hinstance = NULL;

	g_active_frame   = false;
	g_resize_pending = false;
	g_screen_width   = 0u;
	g_screen_height  = 0u;

	MSG message;
	while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {}
}

bool app_closed(void)
{
	bool running = true;
	MSG message;
	
	while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
		if (message.message == WM_QUIT) {
			running = false;
		}

		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return (!running);
}

bool app_minimized(void)
{
	bool minimized =
		(g_screen_width  == 0) ||
		(g_screen_height == 0);

	if (minimized)
		WaitMessage();

	return minimized;
}

void app_clear(color_t color)
{
	ASSERT_PANIC(!g_active_frame, "app_clear() failed! (frame already active)");
	g_active_frame = true;

	if (g_resize_pending) {
		g_resize_pending = false;

		ID3D11DeviceContext_OMSetRenderTargets(g_context, 0, NULL, NULL);
		ID3D11RenderTargetView_Release(g_render_target);
		ID3D11DeviceContext_Flush(g_context);

		HRESULT result = IDXGISwapChain1_ResizeBuffers(
			g_swapchain, 0,
			g_screen_width,
			g_screen_height,
			DXGI_FORMAT_UNKNOWN, 0
		);

		CHECK_HRESULT(result, "IDXGISwapChain1_ResizeBuffers()");
		create_render_target();
	}

	ID3D11DeviceContext_OMSetRenderTargets(g_context, 1, &g_render_target, NULL);
	ID3D11DeviceContext_RSSetViewports(g_context, 1, &g_viewport);
	ID3D11DeviceContext_ClearRenderTargetView(g_context, g_render_target, (float*)(&color));
}

void app_draw(mesh_t* mesh)
{
	ASSERT_PANIC(mesh, "app_draw() failed! (mesh is null)");
	ASSERT_PANIC(g_active_frame, "app_draw() failed! (no active frame)");

	UINT stride = sizeof(vertex_t);
	UINT offset = 0;
	ID3D11DeviceContext_IASetInputLayout(g_context, g_input_layout);
	ID3D11DeviceContext_IASetVertexBuffers(g_context, 0, 1, &mesh->vertex_buffer, &stride, &offset);
	ID3D11DeviceContext_IASetPrimitiveTopology(g_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11DeviceContext_VSSetShader(g_context, g_vertex_shader, NULL, 0);
	ID3D11DeviceContext_PSSetShader(g_context, g_pixel_shader, NULL, 0);

	constants_t constants = { .screen_size = { g_viewport.Width, g_viewport.Height } };
	ID3D11DeviceContext_UpdateSubresource(g_context, (ID3D11Resource*)g_constant_buffer, 0, NULL, &constants, 0, 0);
	ID3D11DeviceContext_VSSetConstantBuffers(g_context, 0, 1, &g_constant_buffer);

	ID3D11DeviceContext_Draw(g_context, mesh->vertex_count, 0);
}

void app_render(void)
{
	ASSERT_PANIC(g_active_frame, "app_render() failed! (no active frame)");
	g_active_frame = false;

	HRESULT result = IDXGISwapChain1_Present(g_swapchain, 1, 0);
	CHECK_HRESULT(result, "IDXGISwapChain1_Present()");
}

/*
 *   ------------ MESH ------------
 */

mesh_t* mesh_create(const vertex_t* vertices, size_t count)
{
	ASSERT_PANIC(vertices, "mesh_create() failed! (vertices is null)");
	ASSERT_PANIC(count,    "mesh_create() failed! (zero vertex count)");

	mesh_t* mesh = calloc(1, sizeof(mesh_t));
	ASSERT_PANIC(mesh, "mesh_create() failed! (out of memory)");

	D3D11_BUFFER_DESC vertex_buffer_desc = {
		.ByteWidth = sizeof(vertex_t) * count,
		.Usage     = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
	};

	D3D11_SUBRESOURCE_DATA vertex_data = { .pSysMem = vertices };
	ID3D11Buffer* vertex_buffer = NULL;
	HRESULT result = ID3D11Device_CreateBuffer(g_device, &vertex_buffer_desc, &vertex_data, &vertex_buffer);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");

	mesh->vertex_buffer = vertex_buffer;
	mesh->vertex_count  = count;
	
	return mesh;
}

void mesh_destroy(mesh_t* mesh)
{
	if (!mesh)
		return;

	if (mesh->vertex_buffer)
		ID3D11Buffer_Release(mesh->vertex_buffer);

	free(mesh);
}

/*
 *   ---------- INTERNAL ----------
 */

static LRESULT CALLBACK proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
		case (WM_DESTROY): {
			PostQuitMessage(0);
			return 0;
		}
		case (WM_SIZE): {
			g_screen_width  = LOWORD(lparam);
			g_screen_height = HIWORD(lparam);
			g_resize_pending = true;
			return 0;
		}
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}

static ID3DBlob* compile_shader(const wchar_t* path, const char* entry, const char* target)
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
	#ifdef DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif /* DEBUG */

	ID3DBlob* code   = NULL;
	ID3DBlob* errors = NULL;

	HRESULT result = D3DCompileFromFile(
		path, NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entry, target,
		flags, 0,
		&code, &errors
	);

	if (errors) {
		fprintf(stderr, "%s\n", (const char*)ID3D10Blob_GetBufferPointer(errors));
		ID3D10Blob_Release(errors);
	}

	CHECK_HRESULT(result, "D3DCompileFromFile()");

	return code;
}

static void create_render_target(void)
{
	ID3D11Texture2D* back_buffer = NULL;
	HRESULT result = IDXGISwapChain1_GetBuffer(g_swapchain, 0, &IID_ID3D11Texture2D, (void**)&back_buffer);
	CHECK_HRESULT(result, "IDXGISwapChain1_GetBuffer()");

	result = ID3D11Device_CreateRenderTargetView(g_device, (ID3D11Resource*)back_buffer, NULL, &g_render_target);
	CHECK_HRESULT(result, "ID3D11Device_CreateRenderTargetView()");

	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D_GetDesc(back_buffer, &desc);
	ID3D11Texture2D_Release(back_buffer);

	g_viewport = (D3D11_VIEWPORT) {
		.TopLeftX = 0.0f,
		.TopLeftY = 0.0f,
		.Width = (float)desc.Width,
		.Height = (float)desc.Height,
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};
}

