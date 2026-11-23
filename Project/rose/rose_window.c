
/*
 *   Source [rose_window.c]
 */

#include "rose_internal.h"

bool ROSE_WindowClosed(void)
{
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

void ROSE_FrameClear(ROSE_Color color)
{
	if (rose__resize_pending) {
		rose__resize_pending = FALSE;

		ID3D11DeviceContext_OMSetRenderTargets(rose__context, 0, NULL, NULL);
		ID3D11RenderTargetView_Release(rose__render_target);
		ID3D11DeviceContext_Flush(rose__context);

		HRESULT result = IDXGISwapChain1_ResizeBuffers(
			rose__swapchain, 0,
			rose__screen_width,
			rose__screen_height,
			DXGI_FORMAT_UNKNOWN, 0
		);

		CHECK_HRESULT(result, "IDXGISwapChain1_ResizeBuffers()");
		rose__create_render_target();
	}

	ID3D11DeviceContext_OMSetRenderTargets(rose__context, 1, &rose__render_target, NULL);
	ID3D11DeviceContext_RSSetViewports(rose__context, 1, &rose__viewport);
	ID3D11DeviceContext_ClearRenderTargetView(rose__context, rose__render_target, (float*)(&color));
}

void ROSE_FrameRender(void)
{
	HRESULT result = IDXGISwapChain1_Present(rose__swapchain, 1, 0);
	CHECK_HRESULT(result, "IDXGISwapChain1_Present()");
}

