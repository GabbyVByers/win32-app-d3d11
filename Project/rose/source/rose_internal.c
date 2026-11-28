
/*
 *   Source [rose_internal.c]
 */

#define ROSE_DEFINE_GLOBALS
#include "rose_internal.h"

LRESULT CALLBACK ROSE__WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}

	if (msg == WM_SIZE) {
		rose__screen_width   = LOWORD(lparam);
		rose__screen_height  = HIWORD(lparam);
		rose__resize_pending = TRUE;
		return 0;
	}

	if (msg == WM_MOUSEMOVE) {
		rose__mouse.curr_pos.x = (float)GET_X_LPARAM(lparam);
		rose__mouse.curr_pos.y = (float)GET_Y_LPARAM(lparam);
		return 0;
	}

	if (msg == WM_LBUTTONDOWN) { ROSE__MouseButton(ROSE_MOUSE_LEFT,   true);  return 0; }
	if (msg == WM_LBUTTONUP)   { ROSE__MouseButton(ROSE_MOUSE_LEFT,   false); return 0; }
	if (msg == WM_MBUTTONDOWN) { ROSE__MouseButton(ROSE_MOUSE_MIDDLE, true);  return 0; }
	if (msg == WM_MBUTTONUP)   { ROSE__MouseButton(ROSE_MOUSE_MIDDLE, false); return 0; }
	if (msg == WM_RBUTTONDOWN) { ROSE__MouseButton(ROSE_MOUSE_RIGHT,  true);  return 0; }
	if (msg == WM_RBUTTONUP)   { ROSE__MouseButton(ROSE_MOUSE_RIGHT,  false); return 0; }

	bool mousex =
		(msg == WM_XBUTTONDOWN) ||
		(msg == WM_XBUTTONUP);

	if (mousex) {
		ROSE_MouseButton button =
			(GET_XBUTTON_WPARAM(wparam) == XBUTTON1) ?
			(ROSE_MOUSE_X1) :
			(ROSE_MOUSE_X2);

		bool pressed = (msg == WM_XBUTTONDOWN);
		ROSE__MouseButton(button, pressed);
		return TRUE;
	}

	if (msg == WM_INPUT) {
		ROSE__MouseRawInput(lparam);
		return DefWindowProcW(hwnd, msg, wparam, lparam);
	}

	bool hide_cursor =
		(msg == WM_SETCURSOR) &&
		(rose__mouse.hidden)  &&
		(LOWORD(lparam) == HTCLIENT);

	if (hide_cursor) {
		SetCursor(NULL);
		return TRUE;
	}

	if (msg == WM_MOUSEWHEEL) {
		rose__mouse.scroll +=
			(float)GET_WHEEL_DELTA_WPARAM(wparam) /
			(float)WHEEL_DELTA;
		return 0;
	}

	if (msg == WM_KILLFOCUS) {
		ROSE__MouseRelease();
		ROSE__KeyboardRelease();
		return 0;
	}

	bool scancode =
		(msg == WM_KEYDOWN)    ||
		(msg == WM_KEYUP)      ||
		(msg == WM_SYSKEYDOWN) ||
		(msg == WM_SYSKEYUP);

	if (scancode) {
		ROSE__KeyboardMessage(msg, wparam, lparam);

		if ((msg == WM_SYSKEYDOWN) ||
			(msg == WM_SYSKEYUP))
			return DefWindowProcW(hwnd, msg, wparam, lparam);

		return 0;
	}

	bool menu =
		(msg == WM_SYSCOMMAND) &&
		((wparam & 0xFFF0) == SC_KEYMENU);

	if (menu)
		return 0;

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}

ID3DBlob* ROSE__CompileShader(const wchar_t* path, const char* entry, const char* target)
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
	#ifdef ROSE_DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif /* ROSE_DEBUG */

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

void ROSE__CreateRenderTarget(void)
{
	ID3D11Texture2D* back_buffer = NULL;
	HRESULT result = IDXGISwapChain1_GetBuffer(rose__swapchain, 0, &IID_ID3D11Texture2D, (void**)&back_buffer);
	CHECK_HRESULT(result, "IDXGISwapChain1_GetBuffer()");

	result = ID3D11Device_CreateRenderTargetView(rose__device, (ID3D11Resource*)back_buffer, NULL, &rose__render_target);
	CHECK_HRESULT(result, "ID3D11Device_CreateRenderTargetView()");

	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D_GetDesc(back_buffer, &desc);
	
	D3D11_TEXTURE2D_DESC depth_desc = {
		.Width      = desc.Width,
		.Height     = desc.Height,
		.MipLevels  = 1,
		.ArraySize  = 1,
		.Format     = DXGI_FORMAT_D32_FLOAT,
		.SampleDesc = {.Count = 1, .Quality = 0 },
		.Usage      = D3D11_USAGE_DEFAULT,
		.BindFlags  = D3D11_BIND_DEPTH_STENCIL,
	};

	ID3D11Texture2D* depth_texture = NULL;
	result = ID3D11Device_CreateTexture2D(rose__device, &depth_desc, NULL, &depth_texture);
	CHECK_HRESULT(result, "ID3D11Device_CreateTexture2D()");

	result = ID3D11Device_CreateDepthStencilView(rose__device, (ID3D11Resource*)depth_texture, NULL, &rose__depth_view);
	CHECK_HRESULT(result, "ID3D11Device_CreateDepthStencilView()");

	ID3D11Texture2D_Release(depth_texture);
	ID3D11Texture2D_Release(back_buffer);

	rose__viewport = (D3D11_VIEWPORT) {
		.TopLeftX = 0.0f,
		.TopLeftY = 0.0f,
		.Width    = (float)desc.Width,
		.Height   = (float)desc.Height,
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};
}

void ROSE__BindPipelineShape(ID3D11Buffer* vertices, ROSE_Texture* texture, ROSE_Vec2 position, float scale, ROSE_Color color, ROSE_PRIMITIVE primitive, float diameter)
{
	if ((texture == NULL) || (texture->view == NULL))
		texture = rose__blank_texture;

	UINT stride = sizeof(ROSE_VertexShape);
	UINT offset = 0;
	ID3D11DeviceContext_IASetInputLayout(rose__context, rose__input_layout_shape);
	ID3D11DeviceContext_IASetVertexBuffers(rose__context, 0, 1, &vertices, &stride, &offset);
	ROSE__BindPrimitive(primitive, diameter);

	ID3D11DeviceContext_VSSetShader(rose__context, rose__vertex_shader_shape, NULL, 0);
	ID3D11DeviceContext_PSSetShader(rose__context, rose__pixel_shader_shape, NULL, 0);

	rose__constants_shape_t constants = {
		.screen_size = { rose__viewport.Width, rose__viewport.Height },
		.offset      = position,
		.tint        = color,
		.scale       = scale,
	};

	ID3D11DeviceContext_UpdateSubresource(rose__context, (ID3D11Resource*)rose__constant_buffer_shape, 0, NULL, &constants, 0, 0);
	ID3D11DeviceContext_VSSetConstantBuffers(rose__context, 0, 1, &rose__constant_buffer_shape);

	ID3D11DeviceContext_PSSetShaderResources(rose__context, 0, 1, &texture->view);
	ID3D11DeviceContext_PSSetSamplers(rose__context, 0, 1, &rose__sampler);
	ID3D11DeviceContext_RSSetState(rose__context, rose__raster_shape);
	ID3D11DeviceContext_OMSetBlendState(rose__context, rose__blend_state, NULL, 0xFFFFFFFF);
	ID3D11DeviceContext_OMSetDepthStencilState(rose__context, rose__depth_off, 0);
}

void ROSE__BindPrimitive(ROSE_PRIMITIVE primitive, float diameter)
{
	if (primitive == ROSE_TRIANGLES) {
		ID3D11DeviceContext_IASetPrimitiveTopology(rose__context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ID3D11DeviceContext_GSSetShader(rose__context, NULL, NULL, 0);
		return;
	}

	if (primitive == ROSE_LINES) {
		ID3D11DeviceContext_IASetPrimitiveTopology(rose__context, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		ID3D11DeviceContext_GSSetShader(rose__context, NULL, NULL, 0);
		return;
	}

	assert(primitive == ROSE_POINTS);

	rose__constants_points_t constants = {
		.screen_size = { rose__viewport.Width, rose__viewport.Height },
		.diameter    = diameter,
	};

	ID3D11DeviceContext_UpdateSubresource(rose__context, (ID3D11Resource*)rose__constant_buffer_points, 0, NULL, &constants, 0, 0);
	ID3D11DeviceContext_IASetPrimitiveTopology(rose__context, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	ID3D11DeviceContext_GSSetShader(rose__context, rose__geometry_shader_points, NULL, 0);
	ID3D11DeviceContext_GSSetConstantBuffers(rose__context, 0, 1, &rose__constant_buffer_points);
}

Int64 ROSE__Ticks(void)
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return ticks.QuadPart;
}

#define BATCH_SIZE 4096
#define VERTS_PER_QUAD 6
#define BATCH_VERTICES (BATCH_SIZE * VERTS_PER_QUAD)

static uInt32 batch_cursor = 0;
static uInt32 batch_count  = 0;
static ID3D11Buffer* batch_buffer  = NULL;
static ROSE_Texture* batch_texture = NULL;
static ROSE_VertexShape batch_vertices[BATCH_VERTICES];

ROSE_VertexShape* ROSE__BatchAllocate(ROSE_Texture* texture, uInt32 count)
{
	if (texture != batch_texture) {
		ROSE__BatchFlush();
		batch_texture = texture;
	}

	if ((batch_count + count) > BATCH_VERTICES)
		ROSE__BatchFlush();

	ROSE_VertexShape* vertices = batch_vertices + batch_count;
	batch_count += count;
	return vertices;
}

void ROSE__BatchCreate(void)
{
	D3D11_BUFFER_DESC desc = {
		.ByteWidth      = sizeof(ROSE_VertexShape) * BATCH_VERTICES,
		.Usage          = D3D11_USAGE_DYNAMIC,
		.BindFlags      = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

	HRESULT result = ID3D11Device_CreateBuffer(rose__device, &desc, NULL, &batch_buffer);
	CHECK_HRESULT(result, "ID3D11Device_CreateBuffer()");

	batch_cursor  = BATCH_VERTICES;
	batch_count   = 0;
	batch_texture = NULL;
}

void ROSE__BatchDestroy(void)
{
	if (batch_buffer != NULL)
		ID3D11Buffer_Release(batch_buffer);

	batch_cursor  = 0;
	batch_count   = 0;
	batch_buffer  = NULL;
	batch_texture = NULL;
}

void ROSE__BatchFlush(void)
{
	if (batch_count == 0)
		return;

	D3D11_MAP mode = D3D11_MAP_WRITE_NO_OVERWRITE;
	bool wrap = (batch_cursor + batch_count) > BATCH_VERTICES;

	if (wrap) {
		mode = D3D11_MAP_WRITE_DISCARD;
		batch_cursor = 0;
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT result = ID3D11DeviceContext_Map(rose__context, (ID3D11Resource*)batch_buffer, 0, mode, 0, &mapped);
	CHECK_HRESULT(result, "ID3D11DeviceContext_Map()");

	ROSE_VertexShape* dst = (ROSE_VertexShape*)mapped.pData + batch_cursor;
	memcpy(dst, batch_vertices, sizeof(ROSE_VertexShape) * batch_count);
	ID3D11DeviceContext_Unmap(rose__context, (ID3D11Resource*)batch_buffer, 0);

	ROSE__BindPipelineShape(batch_buffer, batch_texture, ROSE_VEC2_ZERO, 1.0f, ROSE_COLOR_WHITE, ROSE_TRIANGLES, 0.0f);
	ID3D11DeviceContext_Draw(rose__context, batch_count, batch_cursor);

	batch_cursor += batch_count;
	batch_count = 0;
}

