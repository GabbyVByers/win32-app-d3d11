
/*
 *   Source [rose_internal.c]
 */

#define ROSE_DEFINE_GLOBALS
#include "rose_internal.h"

LRESULT CALLBACK rose__proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
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
		rose__mouse.curr_xpos = (float)GET_X_LPARAM(lparam);
		rose__mouse.curr_ypos = (float)GET_Y_LPARAM(lparam);
		return 0;
	}

	if (msg == WM_LBUTTONDOWN) { rose__mouse_button(ROSE_MOUSE_LEFT,   true);  return 0; }
	if (msg == WM_LBUTTONUP)   { rose__mouse_button(ROSE_MOUSE_LEFT,   false); return 0; }
	if (msg == WM_MBUTTONDOWN) { rose__mouse_button(ROSE_MOUSE_MIDDLE, true);  return 0; }
	if (msg == WM_MBUTTONUP)   { rose__mouse_button(ROSE_MOUSE_MIDDLE, false); return 0; }
	if (msg == WM_RBUTTONDOWN) { rose__mouse_button(ROSE_MOUSE_RIGHT,  true);  return 0; }
	if (msg == WM_RBUTTONUP)   { rose__mouse_button(ROSE_MOUSE_RIGHT,  false); return 0; }

	bool mousex =
		(msg == WM_XBUTTONDOWN) ||
		(msg == WM_XBUTTONUP);

	if (mousex) {
		ROSE_MouseButton button =
			(GET_XBUTTON_WPARAM(wparam) == XBUTTON1) ?
			(ROSE_MOUSE_X1) :
			(ROSE_MOUSE_X2);

		bool pressed = (msg == WM_XBUTTONDOWN);
		rose__mouse_button(button, pressed);
		return TRUE;
	}

	if (msg == WM_MOUSEWHEEL) {
		rose__mouse.scroll +=
			(float)GET_WHEEL_DELTA_WPARAM(wparam) /
			(float)WHEEL_DELTA;
		return 0;
	}

	if (msg == WM_KILLFOCUS) {
		rose__mouse_release();
		rose__keyboard_release();
		return 0;
	}

	bool scancode =
		(msg == WM_KEYDOWN)    ||
		(msg == WM_KEYUP)      ||
		(msg == WM_SYSKEYDOWN) ||
		(msg == WM_SYSKEYUP);

	if (scancode) {
		rose__keyboard_message(msg, wparam, lparam);

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

ID3DBlob* rose__compile_shader(const wchar_t* path, const char* entry, const char* target)
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

void rose__create_render_target(void)
{
	ID3D11Texture2D* back_buffer = NULL;
	HRESULT result = IDXGISwapChain1_GetBuffer(rose__swapchain, 0, &IID_ID3D11Texture2D, (void**)&back_buffer);
	CHECK_HRESULT(result, "IDXGISwapChain1_GetBuffer()");

	result = ID3D11Device_CreateRenderTargetView(rose__device, (ID3D11Resource*)back_buffer, NULL, &rose__render_target);
	CHECK_HRESULT(result, "ID3D11Device_CreateRenderTargetView()");

	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D_GetDesc(back_buffer, &desc);
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

Int64 rose__ticks(void)
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return ticks.QuadPart;
}

void rose__bind_2d(ID3D11Buffer* vertices, ROSE_Texture* texture)
{
	if ((texture == NULL) || (texture->view == NULL))
		texture = rose__blank_texture;

	UINT stride = sizeof(ROSE_Vertex2D);
	UINT offset = 0;
	ID3D11DeviceContext_IASetInputLayout(rose__context, rose__input_layout);
	ID3D11DeviceContext_IASetVertexBuffers(rose__context, 0, 1, &vertices, &stride, &offset);
	ID3D11DeviceContext_IASetPrimitiveTopology(rose__context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11DeviceContext_VSSetShader(rose__context, rose__vertex_shader, NULL, 0);
	ID3D11DeviceContext_PSSetShader(rose__context, rose__pixel_shader, NULL, 0);

	rose__constants_t constants = { .screen_size = { rose__viewport.Width, rose__viewport.Height } };
	ID3D11DeviceContext_UpdateSubresource(rose__context, (ID3D11Resource*)rose__constant_buffer, 0, NULL, &constants, 0, 0);
	ID3D11DeviceContext_VSSetConstantBuffers(rose__context, 0, 1, &rose__constant_buffer);

	ID3D11DeviceContext_PSSetShaderResources(rose__context, 0, 1, &texture->view);
	ID3D11DeviceContext_PSSetSamplers(rose__context, 0, 1, &rose__sampler);
	ID3D11DeviceContext_OMSetBlendState(rose__context, rose__blend_state, NULL, 0xFFFFFFFF);
}

