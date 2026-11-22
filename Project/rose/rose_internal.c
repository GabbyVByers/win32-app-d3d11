
/*
 *   Source [rose_internal.c]
 */

#define ROSE_EXTERN
#include "rose_internal.h"

LRESULT CALLBACK rose__proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
		case (WM_DESTROY): {
			PostQuitMessage(0);
			return 0;
		}
		case (WM_SIZE): {
			rose__screen_width   = LOWORD(lparam);
			rose__screen_height  = HIWORD(lparam);
			rose__resize_pending = TRUE;
			return 0;
		}
	}

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

