
/*
 *   Source [rose_window.c]
 */

#include "rose_internal.h"

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

	ID3D11DeviceContext_OMSetRenderTargets(rose__context, 1, &rose__render_target, rose__depth_view);
	ID3D11DeviceContext_RSSetViewports(rose__context, 1, &rose__viewport);
	ID3D11DeviceContext_ClearRenderTargetView(rose__context, rose__render_target, (float*)(&color));
	ID3D11DeviceContext_ClearDepthStencilView(rose__context, rose__depth_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ROSE_FrameRender(void)
{
	rose__batch_flush();
	HRESULT result = IDXGISwapChain1_Present(rose__swapchain, (UINT)rose__vsync, 0);
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

