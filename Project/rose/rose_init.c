
/*
 *   Source [rose_init.c]
 */

#include "rose_internal.h"

void ROSE_Init(const wchar_t* title, uInt32 width, uInt32 height)
{
	rose__hinstance = GetModuleHandleW(NULL);
	WNDCLASSEXW window_class = {
		.cbSize        = sizeof(WNDCLASSEXW),
		.style         = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc   = rose__proc,
		.hInstance     = rose__hinstance,
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.lpszClassName = WINDOW_CLASS_NAME,
	};

	ATOM atom = RegisterClassExW(&window_class);
	DWORD error = GetLastError();
	assert(atom && "RegisterClassExW() failed!");

	RECT rect = { 0, 0, (long)width, (long)height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	width = (uInt32)(rect.right - rect.left);
	height = (uInt32)(rect.bottom - rect.top);

	rose__hwnd = CreateWindowExW(0,
		WINDOW_CLASS_NAME, title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(int)width,
		(int)height,
		NULL, NULL,
		rose__hinstance, NULL
	);

	assert(rose__hwnd, "CreateWindowExW() failed!");
	ShowWindow(rose__hwnd, SW_SHOWDEFAULT);

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
		&rose__device, NULL, &rose__context
	);

	CHECK_HRESULT(result, "D3D11CreateDevice()");

	#ifdef DEBUG
	ID3D11InfoQueue* info = NULL;
	if (SUCCEEDED(ID3D11Device_QueryInterface(rose__device, &IID_ID3D11InfoQueue, (void**)&info))) {
		ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
		ID3D11InfoQueue_Release(info); }
	#endif /* DEBUG */

	IDXGIDevice*   dxgi_device = NULL;
	IDXGIAdapter*  adapter     = NULL;
	IDXGIFactory2* factory     = NULL;
	
	result = ID3D11Device_QueryInterface(rose__device, &IID_IDXGIDevice, (void**)&dxgi_device);
	CHECK_HRESULT(result, "ID3D11Device_QueryInterface()");

	result = IDXGIDevice_GetAdapter(dxgi_device, &adapter);
	CHECK_HRESULT(result, "IDXGIDevice_GetAdapter()");

	result = IDXGIAdapter_GetParent(adapter, &IID_IDXGIFactory2, (void**)&factory);
	CHECK_HRESULT(result, "IDXGIAdapter_GetParent()");

	#ifdef DEBUG
	DXGI_ADAPTER_DESC description;
	IDXGIAdapter_GetDesc(adapter, &description);
	const wchar_t* name = description.Description;
	fprintf(stdout, "Using GPU Device: %ls\n", name);
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

	result = IDXGIFactory2_CreateSwapChainForHwnd(factory, (IUnknown*)rose__device, rose__hwnd, &swapchain_desc, NULL, NULL, &rose__swapchain);
	CHECK_HRESULT(result, "IDXGIFactory2_CreateSwapChainForHwnd()");

	IDXGIFactory2_MakeWindowAssociation(factory, rose__hwnd, DXGI_MWA_NO_ALT_ENTER);

	IDXGIFactory2_Release(factory);
	IDXGIAdapter_Release(adapter);
	IDXGIDevice_Release(dxgi_device);

	rose__create_render_target();
	ID3DBlob* vs_blob = rose__compile_shader(L"HLSL/shader.hlsl", "vs_main", "vs_5_0");
	ID3DBlob* ps_blob = rose__compile_shader(L"HLSL/shader.hlsl", "ps_main", "ps_5_0");

	result = ID3D11Device_CreateVertexShader(rose__device, ID3D10Blob_GetBufferPointer(vs_blob), ID3D10Blob_GetBufferSize(vs_blob), NULL, &rose__vertex_shader);
	CHECK_HRESULT(result, "ID3D11Device_CreateVertexShader()");

	result = ID3D11Device_CreatePixelShader(rose__device, ID3D10Blob_GetBufferPointer(ps_blob), ID3D10Blob_GetBufferSize(ps_blob), NULL, &rose__pixel_shader);
	CHECK_HRESULT(result, "ID3D11Device_CreatePixelShader()");

	D3D11_INPUT_ELEMENT_DESC input_desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(ROSE_Vertex2D, xpos),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(ROSE_Vertex2D, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	result = ID3D11Device_CreateInputLayout(
		rose__device, input_desc,
		ARRAYSIZE(input_desc),
		ID3D10Blob_GetBufferPointer(vs_blob),
		ID3D10Blob_GetBufferSize(vs_blob),
		&rose__input_layout
	);

	CHECK_HRESULT(result, "ID3D11Device_CreateInputLayout()");

	ID3D10Blob_Release(vs_blob);
	ID3D10Blob_Release(ps_blob);

	D3D11_BUFFER_DESC constant_buffer_desc = {
		.ByteWidth = sizeof(constants_t),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
	};

	result = ID3D11Device_CreateBuffer(rose__device, &constant_buffer_desc, NULL, &rose__constant_buffer);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");
}

void ROSE_Quit(void)
{
	ID3D11DeviceContext_ClearState(rose__context);

	ID3D11Buffer_Release(rose__constant_buffer);
	ID3D11InputLayout_Release(rose__input_layout);
	ID3D11PixelShader_Release(rose__pixel_shader);
	ID3D11VertexShader_Release(rose__vertex_shader);
	ID3D11RenderTargetView_Release(rose__render_target);
	IDXGISwapChain1_Release(rose__swapchain);
	ID3D11DeviceContext_Release(rose__context);
	ID3D11Device_Release(rose__device);

	rose__constant_buffer = NULL;
	rose__input_layout    = NULL;
	rose__pixel_shader    = NULL;
	rose__vertex_shader   = NULL;
	rose__render_target   = NULL;
	rose__swapchain = NULL;
	rose__context   = NULL;
	rose__device    = NULL;
	rose__viewport  = (D3D11_VIEWPORT) { 0 };

	bool close =
		rose__hwnd &&
		IsWindow(rose__hwnd);

	if (close)
		DestroyWindow(rose__hwnd);

	UnregisterClassW(WINDOW_CLASS_NAME, rose__hinstance);

	rose__quit      = FALSE;
	rose__hwnd      = NULL;
	rose__hinstance = NULL;

	rose__resize_pending = FALSE;
	rose__screen_width   = 0u;
	rose__screen_height  = 0u;

	MSG message;
	while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {}
}

