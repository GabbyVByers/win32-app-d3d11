
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
#include <d3dcompiler.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define WINDOW_CLASS_NAME L"WindowClassNameDirect3D11"

#define CHECK_HRESULT(hr, func)                                                      \
do {                                                                                 \
	if (FAILED(result)) {                                                            \
		fprintf(stderr, "%s failed! (error: 0x%08lX)\n", func, (unsigned long)(hr)); \
		exit(EXIT_FAILURE);                                                          \
	}                                                                                \
} while (0)

typedef struct constants_t {
	float screen_size[2];
	float _padding[2];
} constants_t;

static bool g_resize_pending = false;
static UINT g_screen_width   = 0;
static UINT g_screen_height  = 0;

static HINSTANCE g_hinstance = NULL;
static HWND      g_hwnd      = NULL;

ID3D11Device*           g_device             = NULL;
ID3D11DeviceContext*    g_context            = NULL;
IDXGISwapChain1*        g_swapchain          = NULL;
ID3D11RenderTargetView* g_render_target_view = NULL;
static ID3D11Buffer*    g_constant_buffer    = NULL;

static LRESULT CALLBACK proc(HWND, UINT, WPARAM, LPARAM);
static ID3DBlob* compile_shader(const wchar_t*, const char*, const char*);
static void create_render_target(ID3D11Device*, IDXGISwapChain1*, ID3D11RenderTargetView**, D3D11_VIEWPORT*);

void app_init(const wchar_t* title, int width, int height)
{
	bool debug = false;
	#ifndef NDEBUG
	debug = true;
	#endif

	assert(!g_hinstance);
	g_hinstance = GetModuleHandleW(NULL);
	WNDCLASSEXW window_class = {
		.cbSize        = sizeof(WNDCLASSEXW),
		.style         = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc   = proc,
		.hInstance     = g_hinstance,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.lpszClassName = WINDOW_CLASS_NAME,
	};

	if (!RegisterClassExW(&window_class))
	{
		fprintf(stderr, "RegisterClassExW() failed! (error: %lu)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	RECT rect = { 0, 0, (long)width, (long)height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	width = (int)(rect.right - rect.left);
	height = (int)(rect.bottom - rect.top);

	assert(!g_hwnd);
	g_hwnd = CreateWindowExW(0,
		WINDOW_CLASS_NAME, title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width, height,
		NULL, NULL,
		g_hinstance, NULL
	);

	if (!g_hwnd) {
		fprintf(stderr, "CreateWindowExW() failed! (error: %lu)\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	ShowWindow(g_hwnd, SW_SHOWDEFAULT);

	UINT flags = 0u;
	if (debug) {
		flags |= D3D11_CREATE_DEVICE_DEBUG;
	}

	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };

	HRESULT result = D3D11CreateDevice(
		NULL, D3D_DRIVER_TYPE_HARDWARE,
		NULL, flags,
		levels, ARRAYSIZE(levels),
		D3D11_SDK_VERSION,
		&g_device, NULL, &g_context
	);

	CHECK_HRESULT(result, "D3D11CreateDevice()");

	if (debug) {
		ID3D11InfoQueue* info = NULL;
		if (SUCCEEDED(ID3D11Device_QueryInterface(g_device, &IID_ID3D11InfoQueue, (void**)&info))) {
			ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
			ID3D11InfoQueue_Release(info);
		}
	}
	
	IDXGIDevice*   dxgi_device = NULL;
	IDXGIAdapter*  adapter     = NULL;
	IDXGIFactory2* factory     = NULL;
	
	result = ID3D11Device_QueryInterface(g_device, &IID_IDXGIDevice, (void**)&dxgi_device);
	CHECK_HRESULT(result, "ID3D11Device_QueryInterface()");

	result = IDXGIDevice_GetAdapter(dxgi_device, &adapter);
	CHECK_HRESULT(result, "IDXGIDevice_GetAdapter()");

	result = IDXGIAdapter_GetParent(adapter, &IID_IDXGIFactory2, (void**)&factory);
	CHECK_HRESULT(result, "IDXGIAdapter_GetParent()");

	DXGI_ADAPTER_DESC description;
	IDXGIAdapter_GetDesc(adapter, &description);
	fprintf(stdout, "GPU Device: %ls\n", description.Description);

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

	D3D11_VIEWPORT viewport;
	create_render_target(g_device, g_swapchain, &g_render_target_view, &viewport);

	ID3DBlob* vs_blob = compile_shader(L"HLSL/shader.hlsl", "vs_main", "vs_5_0");
	ID3DBlob* ps_blob = compile_shader(L"HLSL/shader.hlsl", "ps_main", "ps_5_0");

	ID3D11VertexShader* vertex_shader = NULL;
	result = ID3D11Device_CreateVertexShader(g_device, ID3D10Blob_GetBufferPointer(vs_blob), ID3D10Blob_GetBufferSize(vs_blob), NULL, &vertex_shader);
	if (FAILED(result)) {
		fprintf(stderr, "ID3D11Device_CreateVertexShader() failed! (error: 0x%08lX)\n", (unsigned long)result);
		exit(EXIT_FAILURE);
	}

	ID3D11PixelShader* pixel_shader = NULL;
	result = ID3D11Device_CreatePixelShader(g_device, ID3D10Blob_GetBufferPointer(ps_blob), ID3D10Blob_GetBufferSize(ps_blob), NULL, &pixel_shader);
	if (FAILED(result)) {
		fprintf(stderr, "ID3D11Device_CreatePixelShader() failed! (error: 0x%08lX)\n", (unsigned long)result);
		exit(EXIT_FAILURE);
	}

	D3D11_INPUT_ELEMENT_DESC input_desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(vertex_t, xpos),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(vertex_t, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ID3D11InputLayout* input_layout = NULL;
	result = ID3D11Device_CreateInputLayout(
		g_device, input_desc,
		ARRAYSIZE(input_desc),
		ID3D10Blob_GetBufferPointer(vs_blob),
		ID3D10Blob_GetBufferSize(vs_blob),
		&input_layout
	);
	
	if (FAILED(result)) {
		fprintf(stderr, "ID3D11Device_CreateInputLayout() failed! (error: 0x%08lX)\n", (unsigned long)result);
		exit(EXIT_FAILURE);
	}

	ID3D10Blob_Release(vs_blob);
	ID3D10Blob_Release(ps_blob);

	// --- vertex buffer ----------------------

	vertex_t vertices[] = {
		{ .xpos = 600.0f, .ypos = 100.0f, .color = COLOR_RED   },
		{ .xpos = 900.0f, .ypos = 600.0f, .color = COLOR_GREEN },
		{ .xpos = 300.0f, .ypos = 600.0f, .color = COLOR_BLUE  },
	};

	D3D11_BUFFER_DESC vertex_buffer_desc = {
		.ByteWidth = sizeof(vertices),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
	};
	D3D11_SUBRESOURCE_DATA vertex_data = { .pSysMem = vertices };

	ID3D11Buffer* vertex_buffer = NULL;
	result = ID3D11Device_CreateBuffer(g_device, &vertex_buffer_desc, &vertex_data, &vertex_buffer);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");

	// --- constant buffer --------------------

	D3D11_BUFFER_DESC constant_buffer_desc = {
		.ByteWidth = sizeof(constants_t),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
	};

	result = ID3D11Device_CreateBuffer(device, &constant_buffer_desc, NULL, &g_constant_buffer);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");
}

void app_quit(void)
{
	ID3D11Buffer_Release(g_constant_buffer);
	ID3D11RenderTargetView_Release(g_render_target_view);
	IDXGISwapChain1_Release(g_swapchain);
	ID3D11DeviceContext_Release(context);
	ID3D11Device_Release(device);
}

bool win_closed(win_t* win)
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

	return running;
}

bool win_minimized(win_t* win)
{

}

void win_clear(win_t* win, color_t color)
{
	if (g_resize_pending) {
		g_resize_pending = false;

		ID3D11DeviceContext_OMSetRenderTargets(context, 0, NULL, NULL);
		ID3D11RenderTargetView_Release(render_target_view);
		ID3D11DeviceContext_Flush(context);

		result = IDXGISwapChain1_ResizeBuffers(
			swapchain, 0,
			g_client_width,
			g_client_height,
			DXGI_FORMAT_UNKNOWN, 0
		);

		CHECK_HRESULT(result, "IDXGISwapChain1_ResizeBuffers()");

		create_render_target(device, swapchain, &render_target_view, &viewport);
	}

	ID3D11DeviceContext_OMSetRenderTargets(context, 1, &render_target_view, NULL);
	ID3D11DeviceContext_RSSetViewports(context, 1, &viewport);

	color_t color = COLOR_RGBA(40, 15, 80, 255);
	ID3D11DeviceContext_ClearRenderTargetView(context, render_target_view, (float*)(&color));
}

void win_draw(win_t* win, mesh_t* mesh)
{
	UINT stride = sizeof(vertex_t);
	UINT offset = 0;
	ID3D11DeviceContext_IASetInputLayout(context, input_layout);
	ID3D11DeviceContext_IASetVertexBuffers(context, 0, 1, &vertex_buffer, &stride, &offset);
	ID3D11DeviceContext_IASetPrimitiveTopology(context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11DeviceContext_VSSetShader(context, vertex_shader, NULL, 0);
	ID3D11DeviceContext_PSSetShader(context, pixel_shader, NULL, 0);

	constants_t constants = { .screen_size = { viewport.Width, viewport.Height } };
	ID3D11DeviceContext_UpdateSubresource(context, (ID3D11Resource*)constant_buffer, 0, NULL, &constants, 0, 0);
	ID3D11DeviceContext_VSSetConstantBuffers(context, 0, 1, &constant_buffer);

	ID3D11DeviceContext_Draw(context, 3, 0);
}

void win_render(win_t* win)
{
	result = IDXGISwapChain1_Present(swapchain, 1, 0);
	CHECK_HRESULT(result, "IDXGISwapChain1_Present()");
}

static LRESULT CALLBACK proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
		case (WM_DESTROY): {
			PostQuitMessage(0);
			return 0;
		}
		case (WM_SIZE): {
			g_client_width  = LOWORD(lparam);
			g_client_height = HIWORD(lparam);
			g_resize_pending = true;
			return 0;
		}
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}

static ID3DBlob* compile_shader(const wchar_t* path, const char* entry, const char* target)
{
	bool debug = false;
	#ifndef NDEBUG
	debug = true;
	#endif

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
	if (debug) {
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	}

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

	if (FAILED(result)) {
		fprintf(stderr, "D3DCompileFromFile('%ls', %s) failed! (error: 0x%08lX)\n", path, entry, (unsigned long)result);
		exit(EXIT_FAILURE);
	}

	return code;
}

static void create_render_target(ID3D11Device* device, IDXGISwapChain1* swapchain, ID3D11RenderTargetView** out_rtv, D3D11_VIEWPORT* out_viewport)
{
	ID3D11Texture2D* back_buffer = NULL;
	HRESULT result = IDXGISwapChain1_GetBuffer(swapchain, 0, &IID_ID3D11Texture2D, (void**)&back_buffer);
	CHECK_HRESULT(result, "IDXGISwapChain1_GetBuffer()");

	result = ID3D11Device_CreateRenderTargetView(device, (ID3D11Resource*)back_buffer, NULL, out_rtv);
	CHECK_HRESULT(result, "ID3D11Device_CreateRenderTargetView()");

	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D_GetDesc(back_buffer, &desc);
	ID3D11Texture2D_Release(back_buffer);

	*out_viewport = (D3D11_VIEWPORT){
		.TopLeftX = 0.0f,
		.TopLeftY = 0.0f,
		.Width = (float)desc.Width,
		.Height = (float)desc.Height,
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};
}

