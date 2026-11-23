
/*
 *   Source [main.c]
 */

#include "rose/rose.h"
#include <math.h>

int main(void)
{
	ROSE_Init(L"My Window", 500, 500);

	ROSE_Image* image = ROSE_ImageCreate(8, 8);
	for (uInt32 y = 0; y < 8; y++) {
		for (uInt32 x = 0; x < 8; x++) {
			ROSE_Color color = ((x + y) & 1) ? ROSE_COLOR_WHITE : ROSE_COLOR_PURPLE;
			ROSE_ImageSetPixel(image, x, y, color);
		}
	}

	ROSE_Texture* texture = ROSE_TextureCreate(image);
	ROSE_ImageDestroy(image);

	ROSE_Vertex2D vertices[] = {
		{ .x = 100.0f, .y = 100.0f, .u = 0.0f, .v = 0.0f, .color = ROSE_COLOR_WHITE },
		{ .x = 400.0f, .y = 100.0f, .u = 1.0f, .v = 0.0f, .color = ROSE_COLOR_WHITE },
		{ .x = 100.0f, .y = 400.0f, .u = 0.0f, .v = 1.0f, .color = ROSE_COLOR_WHITE },
		{ .x = 400.0f, .y = 100.0f, .u = 1.0f, .v = 0.0f, .color = ROSE_COLOR_WHITE },
		{ .x = 400.0f, .y = 400.0f, .u = 1.0f, .v = 1.0f, .color = ROSE_COLOR_WHITE },
		{ .x = 100.0f, .y = 400.0f, .u = 0.0f, .v = 1.0f, .color = ROSE_COLOR_WHITE },
	};

	uInt32 count =
		sizeof(vertices) /
		sizeof(vertices[0]);

	ROSE_Mesh2D* mesh = ROSE_Mesh2DCreate(vertices, count);

	uInt32 frame = 0;

	while (true) {
		if (ROSE_WindowClosed())
			break;

		if (ROSE_WindowMinimized())
			continue;

		float t = (float)frame++ / 60.0f;
		float dx = sinf(t * 2.0f) * 80.0f;
		float dy = cosf(t * 3.0f) * 40.0f;

		float ax = 100.0f + dx, ay = 100.0f + dy;
		float bx = 400.0f + dx, by = 400.0f + dy;

		ROSE_Vertex2D moving[] = {
			{ .x = ax, .y = ay, .u = 0.0f, .v = 0.0f, .color = ROSE_COLOR_WHITE },
			{ .x = bx, .y = ay, .u = 1.0f, .v = 0.0f, .color = ROSE_COLOR_WHITE },
			{ .x = ax, .y = by, .u = 0.0f, .v = 1.0f, .color = ROSE_COLOR_WHITE },
			{ .x = bx, .y = ay, .u = 1.0f, .v = 0.0f, .color = ROSE_COLOR_WHITE },
			{ .x = bx, .y = by, .u = 1.0f, .v = 1.0f, .color = ROSE_COLOR_WHITE },
			{ .x = ax, .y = by, .u = 0.0f, .v = 1.0f, .color = ROSE_COLOR_WHITE },
		};

		ROSE_Mesh2DUpdate(mesh, moving, 6);

		ROSE_FrameClear(ROSE_COLOR_RGBA(30, 15, 70, 255));
		ROSE_Mesh2DDraw(mesh, texture);
		ROSE_FrameRender();
	}

	ROSE_TextureDestroy(texture);
	ROSE_Mesh2DDestroy(mesh);
	ROSE_Quit();
}

