
/*
 *   Source [rose_window.c]
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

	assert(rose__hwnd && "CreateWindowExW() failed!");
	ShowWindow(rose__hwnd, SW_SHOWDEFAULT);

	UINT flags = 0u;
	#ifdef ROSE_DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif /* ROSE_DEBUG */

	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };

	HRESULT result = D3D11CreateDevice(
		NULL, D3D_DRIVER_TYPE_HARDWARE,
		NULL, flags,
		levels, ARRAYSIZE(levels),
		D3D11_SDK_VERSION,
		&rose__device, NULL, &rose__context
	);

	CHECK_HRESULT(result, "D3D11CreateDevice()");

	#ifdef ROSE_DEBUG
	ID3D11InfoQueue* info = NULL;
	if (SUCCEEDED(ID3D11Device_QueryInterface(rose__device, &IID_ID3D11InfoQueue, (void**)&info))) {
		ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
		ID3D11InfoQueue_Release(info); }
	#endif /* ROSE_DEBUG */

	IDXGIDevice*   dxgi_device = NULL;
	IDXGIAdapter*  adapter     = NULL;
	IDXGIFactory2* factory     = NULL;
	
	result = ID3D11Device_QueryInterface(rose__device, &IID_IDXGIDevice, (void**)&dxgi_device);
	CHECK_HRESULT(result, "ID3D11Device_QueryInterface()");

	result = IDXGIDevice_GetAdapter(dxgi_device, &adapter);
	CHECK_HRESULT(result, "IDXGIDevice_GetAdapter()");

	result = IDXGIAdapter_GetParent(adapter, &IID_IDXGIFactory2, (void**)&factory);
	CHECK_HRESULT(result, "IDXGIAdapter_GetParent()");

	#ifdef ROSE_DEBUG
	DXGI_ADAPTER_DESC description;
	IDXGIAdapter_GetDesc(adapter, &description);
	const wchar_t* name = description.Description;
	fprintf(stdout, "Using GPU Device: %ls\n", name);
	#endif /* ROSE_DEBUG */

	IDXGIFactory5* factory5 = NULL;
	if (SUCCEEDED(IDXGIFactory2_QueryInterface(factory, &IID_IDXGIFactory5, (void**)&factory5))) {
		BOOL allow = FALSE;
		IDXGIFactory5_CheckFeatureSupport(factory5, DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow, sizeof(allow));
		rose__tearing = (allow == TRUE);
		IDXGIFactory5_Release(factory5);
	}

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
		.Flags       = rose__tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0,
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

	ID3DBlob* vs3d_blob = rose__compile_shader(L"HLSL/shader3d.hlsl", "vs_main", "vs_5_0");
	ID3DBlob* ps3d_blob = rose__compile_shader(L"HLSL/shader3d.hlsl", "ps_main", "ps_5_0");

	result = ID3D11Device_CreateVertexShader(rose__device, ID3D10Blob_GetBufferPointer(vs_blob), ID3D10Blob_GetBufferSize(vs_blob), NULL, &rose__vertex_shader);
	CHECK_HRESULT(result, "ID3D11Device_CreateVertexShader()");

	result = ID3D11Device_CreateVertexShader(rose__device, ID3D10Blob_GetBufferPointer(vs3d_blob), ID3D10Blob_GetBufferSize(vs3d_blob), NULL, &rose__vertex_shader_3d);
	CHECK_HRESULT(result, "ID3D11Device_CreateVertexShader()");

	result = ID3D11Device_CreatePixelShader(rose__device, ID3D10Blob_GetBufferPointer(ps_blob), ID3D10Blob_GetBufferSize(ps_blob), NULL, &rose__pixel_shader);
	CHECK_HRESULT(result, "ID3D11Device_CreatePixelShader()");

	result = ID3D11Device_CreatePixelShader(rose__device, ID3D10Blob_GetBufferPointer(ps3d_blob), ID3D10Blob_GetBufferSize(ps3d_blob), NULL, &rose__pixel_shader_3d);
	CHECK_HRESULT(result, "ID3D11Device_CreatePixelShader()");

	D3D11_INPUT_ELEMENT_DESC input_desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(ROSE_Vertex2D, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(ROSE_Vertex2D, texcoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(ROSE_Vertex2D, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_INPUT_ELEMENT_DESC input_desc_3d[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(ROSE_Vertex3D, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(ROSE_Vertex3D, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	result = ID3D11Device_CreateInputLayout(
		rose__device, input_desc,
		ARRAYSIZE(input_desc),
		ID3D10Blob_GetBufferPointer(vs_blob),
		ID3D10Blob_GetBufferSize(vs_blob),
		&rose__input_layout
	);

	CHECK_HRESULT(result, "ID3D11Device_CreateInputLayout()");

	result = ID3D11Device_CreateInputLayout(
		rose__device, input_desc_3d,
		ARRAYSIZE(input_desc_3d),
		ID3D10Blob_GetBufferPointer(vs3d_blob),
		ID3D10Blob_GetBufferSize(vs3d_blob),
		&rose__input_layout_3d
	);

	CHECK_HRESULT(result, "ID3D11Device_CreateInputLayout()");

	ID3D10Blob_Release(vs_blob);
	ID3D10Blob_Release(ps_blob);

	ID3D10Blob_Release(vs3d_blob);
	ID3D10Blob_Release(ps3d_blob);

	D3D11_BUFFER_DESC constant_buffer_3d_desc = {
		.ByteWidth = sizeof(rose__constants3d_t),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
	};

	result = ID3D11Device_CreateBuffer(rose__device, &constant_buffer_3d_desc, NULL, &rose__constant_buffer_3d);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");

	D3D11_BUFFER_DESC constant_buffer_desc = {
		.ByteWidth = sizeof(rose__constants_t),
		.Usage     = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
	};

	result = ID3D11Device_CreateBuffer(rose__device, &constant_buffer_desc, NULL, &rose__constant_buffer);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");

	D3D11_SAMPLER_DESC sampler_desc = {
		.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT,
		.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP,
		.ComparisonFunc = D3D11_COMPARISON_NEVER,
		.MinLOD         = 0.0f,
		.MaxLOD         = D3D11_FLOAT32_MAX,
	};

	result = ID3D11Device_CreateSamplerState(rose__device, &sampler_desc, &rose__sampler);
	CHECK_HRESULT(result, "ID3D11Device_CreateSamplerState()");

	D3D11_BLEND_DESC blend_desc = {
		.RenderTarget[0] = {
			.BlendEnable           = TRUE,
			.SrcBlend              = D3D11_BLEND_SRC_ALPHA,
			.DestBlend             = D3D11_BLEND_INV_SRC_ALPHA,
			.BlendOp               = D3D11_BLEND_OP_ADD,
			.SrcBlendAlpha         = D3D11_BLEND_ONE,
			.DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA,
			.BlendOpAlpha          = D3D11_BLEND_OP_ADD,
			.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
		},
	};

	result = ID3D11Device_CreateBlendState(rose__device, &blend_desc, &rose__blend_state);
	CHECK_HRESULT(result, "ID3D11Device_CreateBlendState()");

	D3D11_DEPTH_STENCILOP_DESC stencil_keep = {
		.StencilFailOp      = D3D11_STENCIL_OP_KEEP,
		.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
		.StencilPassOp      = D3D11_STENCIL_OP_KEEP,
		.StencilFunc        = D3D11_COMPARISON_ALWAYS,
	};

	D3D11_DEPTH_STENCIL_DESC depth_on_desc = {
		.DepthEnable    = TRUE,
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
		.DepthFunc      = D3D11_COMPARISON_LESS,
		.StencilEnable  = FALSE,
		.FrontFace      = stencil_keep,
		.BackFace       = stencil_keep,
	};

	D3D11_DEPTH_STENCIL_DESC depth_off_desc = {
		.DepthEnable    = FALSE,
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO,
		.DepthFunc      = D3D11_COMPARISON_ALWAYS,
		.StencilEnable  = FALSE,
		.FrontFace      = stencil_keep,
		.BackFace       = stencil_keep,
	};

	result = ID3D11Device_CreateDepthStencilState(rose__device, &depth_on_desc, &rose__depth_on);
	CHECK_HRESULT(result, "ID3D11Device_CreateDepthStencilState()");

	result = ID3D11Device_CreateDepthStencilState(rose__device, &depth_off_desc, &rose__depth_off);
	CHECK_HRESULT(result, "ID3D11Device_CreateDepthStencilState()");

	ROSE_Image* blank = ROSE_ImageCreate(1, 1);
	ROSE_Image* ascii = ROSE_ImageLoad("textures/ascii.png");
	
	rose__blank_texture = ROSE_TextureCreate(blank);
	rose__ascii_texture = ROSE_TextureCreate(ascii);
	rose__batch_create();

	ROSE_ImageDestroy(blank);
	ROSE_ImageDestroy(ascii);

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	rose__qpc_frequency = frequency.QuadPart;

	Int64 now = rose__ticks();
	rose__time_start       = now;
	rose__frame_last       = now;
	rose__frame_next       = now;
	rose__fps_window_start = now;
	rose__vsync = TRUE;

	rose__timer = CreateWaitableTimerExW(
		NULL, NULL,
		CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
		TIMER_ALL_ACCESS
	);

	assert(rose__timer && "CreateWaitableTimerExW() failed!");

	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(rose__hwnd, &cursor);
	
	float mouse_xpos = (float)cursor.x;
	float mouse_ypos = (float)cursor.y;

	rose__mouse.curr_pos.x = mouse_xpos;
	rose__mouse.curr_pos.y = mouse_ypos;
	rose__mouse.prev_pos.x = mouse_xpos;
	rose__mouse.prev_pos.y = mouse_ypos;
}

void ROSE_Quit(void)
{
	ID3D11DeviceContext_ClearState(rose__context);
	rose__batch_destroy();

	ROSE_TextureDestroy(rose__blank_texture);
	ROSE_TextureDestroy(rose__ascii_texture);

	ID3D11BlendState_Release(rose__blend_state);
	ID3D11SamplerState_Release(rose__sampler);

	ID3D11Buffer_Release(rose__constant_buffer);
	ID3D11InputLayout_Release(rose__input_layout);
	ID3D11PixelShader_Release(rose__pixel_shader);
	ID3D11VertexShader_Release(rose__vertex_shader);
	ID3D11RenderTargetView_Release(rose__render_target);
	IDXGISwapChain1_Release(rose__swapchain);
	ID3D11DeviceContext_Release(rose__context);

	ID3D11DepthStencilState_Release(rose__depth_on);
	ID3D11DepthStencilState_Release(rose__depth_off);
	ID3D11DepthStencilView_Release(rose__depth_view);

	ID3D11Buffer_Release(rose__constant_buffer_3d);
	ID3D11InputLayout_Release(rose__input_layout_3d);
	ID3D11PixelShader_Release(rose__pixel_shader_3d);
	ID3D11VertexShader_Release(rose__vertex_shader_3d);

	#ifdef ROSE_DEBUG
	ID3D11Debug* debug = NULL;
	if (SUCCEEDED(ID3D11Device_QueryInterface(rose__device, &IID_ID3D11Debug, (void**)&debug))) {
		ID3D11Debug_ReportLiveDeviceObjects(debug, D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
		ID3D11Debug_Release(debug); }
	#endif /* ROSE_DEBUG */

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

	rose__blank_texture = NULL;
	rose__ascii_texture = NULL;

	rose__blend_state   = NULL;
	rose__sampler       = NULL;

	bool close =
		rose__hwnd &&
		IsWindow(rose__hwnd);

	if (close)
		DestroyWindow(rose__hwnd);

	UnregisterClassW(WINDOW_CLASS_NAME, rose__hinstance);

	if (rose__timer != NULL)
		CloseHandle(rose__timer);

	rose__timer       = NULL;
	rose__vsync       = FALSE;
	rose__tearing     = FALSE;
	rose__max_fps     = 0.0f;
	rose__frame_delta = 0.0f;
	rose__fps         = 0.0f;
	rose__fps_frames  = 0;

	rose__quit      = FALSE;
	rose__hwnd      = NULL;
	rose__hinstance = NULL;

	rose__resize_pending = FALSE;
	rose__screen_width   = 0;
	rose__screen_height  = 0;

	rose__depth_on   = NULL;
	rose__depth_off  = NULL;
	rose__depth_view = NULL;

	rose__constant_buffer_3d = NULL;
	rose__input_layout_3d    = NULL;
	rose__pixel_shader_3d    = NULL;
	rose__vertex_shader_3d   = NULL;

	rose__mouse    = (rose__mouse_t){ 0 };
	rose__keyboard = (rose__keyboard_t){ 0 };

	MSG message;
	while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {}
}

bool ROSE_WindowClosed(void)
{
	rose__mouse_clear();
	rose__keyboard_clear();

	MSG message;
	while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
		if (message.message == WM_QUIT) {
			rose__quit = TRUE;
		}

		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return rose__quit;
}

bool ROSE_WindowMinimized(void)
{
	bool minimized =
		(rose__screen_width  == 0) ||
		(rose__screen_height == 0);

	if (minimized)
		WaitMessage();

	return minimized;
}

void ROSE_WindowVSync(bool vsync)
{
	rose__vsync = vsync;
}

void ROSE_WindowMaxFPS(float max)
{
	rose__max_fps = (max > 0.0f) ? max : 0.0f;
	rose__frame_next = rose__ticks();
}

float ROSE_WindowFPS(void)
{
	return rose__fps;
}

float ROSE_FrameDelta(void)
{
	return rose__frame_delta;
}

double ROSE_Time(void)
{
	return (double)(rose__ticks() - rose__time_start) / (double)rose__qpc_frequency;
}

void ROSE_FrameClear(ROSE_Color color)
{
	if (rose__resize_pending) {
		rose__resize_pending = FALSE;

		ID3D11DeviceContext_OMSetRenderTargets(rose__context, 0, NULL, NULL);
		ID3D11RenderTargetView_Release(rose__render_target);
		ID3D11DepthStencilView_Release(rose__depth_view);
		ID3D11DeviceContext_Flush(rose__context);

		HRESULT result = IDXGISwapChain1_ResizeBuffers(
			rose__swapchain, 0,
			rose__screen_width,
			rose__screen_height,
			DXGI_FORMAT_UNKNOWN,
			rose__tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0
		);

		CHECK_HRESULT(result, "IDXGISwapChain1_ResizeBuffers()");
		rose__create_render_target();
	}

	ID3D11DeviceContext_OMSetRenderTargets(rose__context, 1, &rose__render_target, rose__depth_view);
	ID3D11DeviceContext_RSSetViewports(rose__context, 1, &rose__viewport);
	ID3D11DeviceContext_ClearRenderTargetView(rose__context, rose__render_target, (float*)(&color));
	ID3D11DeviceContext_ClearDepthStencilView(rose__context, rose__depth_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ROSE_FrameRender(void)
{
	rose__batch_flush();
	UINT flags = (!rose__vsync && rose__tearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	HRESULT result = IDXGISwapChain1_Present(rose__swapchain, (UINT)rose__vsync, flags);
	CHECK_HRESULT(result, "IDXGISwapChain1_Present()");

	if (rose__max_fps > 0.0f) {
		Int64 period = (Int64)((double)rose__qpc_frequency / (double)rose__max_fps);
		rose__frame_next += period;

		if (rose__frame_next < rose__ticks()) {
			rose__frame_next = rose__ticks();
		}
		else {
			Int64 spin = rose__qpc_frequency / 1000;
			Int64 remaining = rose__frame_next - rose__ticks();

			if (remaining > spin) {
				LARGE_INTEGER due = { 0 };
				due.QuadPart = -((remaining - spin) * 10000000 / rose__qpc_frequency);
				SetWaitableTimer(rose__timer, &due, 0, NULL, NULL, FALSE);
				WaitForSingleObject(rose__timer, INFINITE);
			}

			while (rose__ticks() < rose__frame_next)
				YieldProcessor();
		}
	}

	Int64 now = rose__ticks();
	double delta = (double)(now - rose__frame_last) / (double)rose__qpc_frequency;
	rose__frame_last = now;
	rose__frame_delta = (float)((delta > 0.25) ? 0.25 : delta);

	rose__fps_frames++;
	Int64 elapsed = now - rose__fps_window_start;
	if (elapsed >= rose__qpc_frequency / 2) {
		rose__fps = (float)((double)rose__fps_frames * (double)rose__qpc_frequency / (double)elapsed);
		rose__fps_frames = 0;
		rose__fps_window_start = now;
	}
}

